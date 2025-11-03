// src/main.cpp — ESP32 + ILI9341 + XPT2046 + LVGL 8.3.x
// UI en landscape 320x240, colores OK (BGR) y selector de rotación del touch.

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include <Arduino_GFX_Library.h>     // umbrella: incluye databus + displays
// (Opcional solo para IntelliSense): 
//   #include <databus/Arduino_ESP32SPI.h>
//   #include <display/Arduino_ILI9341.h>
#include <XPT2046_Touchscreen.h>

#include "ui/ui.h"
#include "led_control.h"

// =================== SWITCHES ===================
#define UI_IS_LANDSCAPE   1   // 1 => 320x240, 0 => 240x320
#define CONVERT_TO_BGR    1   // 1 si el panel invierte colores (CYD típico)
#define TOUCH_ROT         0   // 0: sin rotar, 1: 90° CW, 2: 180°, 3: 90° CCW
// =================================================

// ---------- Pines CYD 2.8" ----------
#define TFT_SCK   14
#define TFT_MOSI  13
#define TFT_MISO  12
#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST   -1
#define TFT_BL    21

// XPT2046 en HSPI
#define TOUCH_CS   33
#define TOUCH_IRQ  36
#define TOUCH_SCK  25
#define TOUCH_MOSI 32
#define TOUCH_MISO 39

// Calibración RAW aproximada (ajusta si ves corrimientos)
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3800
#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3800

// Resolución y rotación
#if UI_IS_LANDSCAPE
  static constexpr int SCREEN_W = 320;
  static constexpr int SCREEN_H = 240;
  static constexpr uint8_t TFT_ROTATION = 1;   // landscape
#else
  static constexpr int SCREEN_W = 240;
  static constexpr int SCREEN_H = 320;
  static constexpr uint8_t TFT_ROTATION = 0;   // portrait
#endif

// Arduino_GFX
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_GFX     *gfx = new Arduino_ILI9341(bus, TFT_RST, /*r=*/0, /*ips=*/false);

// Touch
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// LVGL buffers — 1 solo buffer para ahorrar RAM
#define LVGL_BUF_LINES 12
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[SCREEN_W * LVGL_BUF_LINES];

// ---------- FLUSH ----------
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  const uint16_t w = (uint16_t)(area->x2 - area->x1 + 1);
  const uint16_t h = (uint16_t)(area->y2 - area->y1 + 1);
  const uint32_t n = (uint32_t)w * h;

  uint16_t *pix = (uint16_t *)color_p;

#if LV_COLOR_16_SWAP
  for (uint32_t i = 0; i < n; ++i) {
    uint16_t c = pix[i];
    c = (uint16_t)((c >> 8) | (c << 8)); // a little-endian
  #if CONVERT_TO_BGR
    const uint16_t r = (c >> 11) & 0x1F;
    const uint16_t g = (c >> 5)  & 0x3F;
    const uint16_t b =  c        & 0x1F;
    c = (uint16_t)((b << 11) | (g << 5) | r);
  #endif
    pix[i] = c;
  }
#else
  #if CONVERT_TO_BGR
  for (uint32_t i = 0; i < n; ++i) {
    uint16_t c = pix[i];
    const uint16_t r = (c >> 11) & 0x1F;
    const uint16_t g = (c >> 5)  & 0x3F;
    const uint16_t b =  c        & 0x1F;
    pix[i] = (uint16_t)((b << 11) | (g << 5) | r);
  }
  #endif
#endif

  gfx->draw16bitRGBBitmap(area->x1, area->y1, pix, w, h);
  lv_disp_flush_ready(disp);
}

// ---------- TOUCH ----------
static void my_touch_read(lv_indev_drv_t *indev, lv_indev_data_t *data)
{
  if (ts.tirqTouched() || ts.touched()) {
    TS_Point p = ts.getPoint();

    // 1) Escala RAW al tamaño de la pantalla en orientación base
    int16_t xr = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_W - 1);
    int16_t yr = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_H - 1);

    // 2) Aplica rotación discreta para alinear con la UI
    int16_t x, y;
    switch (TOUCH_ROT) {
      case 0: x = xr;                      y = yr;                      break;
      case 1: x = (SCREEN_W - 1) - yr;     y = xr;                      break; // 90° CW
      case 2: x = (SCREEN_W - 1) - xr;     y = (SCREEN_H - 1) - yr;     break; // 180°
      default:x = yr;                      y = (SCREEN_H - 1) - xr;     break; // 90° CCW
    }

    x = constrain(x, 0, SCREEN_W - 1);
    y = constrain(y, 0, SCREEN_H - 1);

    data->state   = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ---------- SETUP ----------
void setup()
{
  Serial.begin(115200);
  delay(40);

  pinMode(TFT_CS, OUTPUT);   digitalWrite(TFT_CS, HIGH);
  pinMode(TOUCH_CS, OUTPUT); digitalWrite(TOUCH_CS, HIGH);

  gfx->begin();
  gfx->setRotation(TFT_ROTATION);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);        // enciende backlight

  // Touch en HSPI
  SPI.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin();

  // LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, SCREEN_W * LVGL_BUF_LINES);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = SCREEN_W;
  disp_drv.ver_res  = SCREEN_H;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type    = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read;
  lv_indev_drv_register(&indev_drv);

  // Cargar UI de SquareLine y el módulo del LED
  ui_init();
  led_init();
}

// ---------- LOOP ----------
void loop()
{
  lv_timer_handler();
  lv_tick_inc(5);
  delay(3);
}
