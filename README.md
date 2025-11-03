# ESP32 + ILI9341 + XPT2046 + LVGL 8.3 (SquareLine UI)

Proyecto de ejemplo **funcional** para la placa tipo **CYD 2.8"** (ESP32 + TFT ILI9341 + touch XPT2046) usando:
- **PlatformIO/VSCode + Arduino framework**
- **Arduino_GFX** como driver gráfico
- **LVGL 8.3.x** como motor de UI
- UI generada con **SquareLine Studio**

> **Estado verificado en este repo**
> - Resolución **320x240** (landscape).
> - Colores correctos con **BGR** (panel ILI9341 típico en CYD).
> - Touch XPT2046 por **HSPI** con selector de rotación (`TOUCH_ROT`).
> - Control de LED trasero por PWM en **GPIO17** (activo en **LOW**).
> - **`ui_events.c` fue renombrado a `ui_events.cpp`** (ver sección SquareLine).

---

## Hardware y pines (CYD 2.8")

**TFT ILI9341 (SPI VSPI)**  
- SCK = **GPIO14**  
- MOSI = **GPIO13**  
- MISO = **GPIO12** *(no requerido por ILI9341, pero definido)*  
- CS = **GPIO15**  
- DC = **GPIO2**  
- RST = **-1** *(cableado interno; no usar pin)*  
- BL (backlight) = **GPIO21** *(enciende con HIGH)*

**Touch XPT2046 (SPI HSPI, bus separado)**  
- CS = **GPIO33**  
- IRQ = **GPIO36**  
- SCK = **GPIO25**  
- MOSI = **GPIO32**  
- MISO = **GPIO39**

**LED trasero de la placa**  
- LED = **GPIO17**, **activo LOW**, PWM por **LEDC canal 0**

---

## Configuración clave de LVGL (`include/lv_conf.h`)

Asegúrate de estos valores (extracto):
```c
#define LV_COLOR_DEPTH      16
#define LV_COLOR_16_SWAP    1   // <-- necesario con SquareLine 8.3 en este setup
/* ... */
```

SquareLine normalmente genera UIs en 16-bit 565 y espera `LV_COLOR_16_SWAP=1`.  
En el `flush` del `main.cpp` ya se contempla el byte-swap y la conversión a BGR si fuera necesario.

---

## Compilar y cargar (PlatformIO)

1. Abre la carpeta en VSCode.
2. Selecciona el entorno `esp32dev` (platform espressif32 6.5.0).
3. **Build** y **Upload** desde la barra de PlatformIO.
4. Abre el **Monitor Serie** a `115200` para ver los logs de arranque.

---

## Flujo con **SquareLine Studio** (LVGL 8.3)

1. Crea tu proyecto en **SquareLine** con **LVGL 8.3** y resolución **320x240** si usarás landscape.
2. Diseña tu UI (labels, botones, arc, etc.).
3. En **Export**:
   - Target: **LVGL 8.3** (C).
   - Marca **Add UI events** (para tener `ui_events.*`).
   - Exporta a `src/ui/` de este repo.
4. **Renombra** `src/ui/ui_events.c` → **`src/ui/ui_events.cpp`**.
   - **Nota sobre enlace C/C++:** SquareLine suele envolver `ui_events.h` con `extern "C"` para C++; al incluir ese header en `ui_events.cpp` antes de las definiciones, el *linkage* coincide con `ui.c` y enlaza sin problemas.
5. Revisa que **no** queden archivos meta del exportador que no uses en Arduino (puedes borrar `project.info`, `filelist.txt`, `CMakeLists.txt`).

> Si agregas nuevos eventos en SquareLine, **re-exporta** y conserva el archivo como `.cpp` (no `.c`).

---

## Ajustes en `src/main.cpp`

- `UI_IS_LANDSCAPE`: pone `1` para **320x240** (recomendado en CYD 2.8").  
- `CONVERT_TO_BGR`: **1** si ves colores invertidos (en estos paneles suele ser necesario).  
- `TOUCH_ROT`: 0/1/2/3 para alinear el toque con la UI.  
  - Si al pulsar en el botón reacciona en otra zona, prueba `1` o `3`.

**Calibración touch RAW**  
Ajusta `TOUCH_X_MIN/MAX` y `TOUCH_Y_MIN/MAX` si ves corrimientos. Los valores iniciales típicos:
```c
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3800
#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3800
```
Puedes imprimir `TS_Point p = ts.getPoint();` por serie para afinar.

---

## Control del LED trasero

En `led_control.*`:
- `LED_PIN = 17`, `LED_ACTIVE_LOW = 1`
- `LEDC_CH = 0`, `LEDC_RES_BITS = 8`, `LEDC_FREQ = 5000`

En la UI (SquareLine):
- **Arc** (0..100) → evento `ui_event_ArcLed` actualiza PWM (`led_set_percent(v)`).
- **Botón** → `ui_event_BtnLed` conmuta **ON/OFF** (si OFF fuerza duty para apagar).

> Si cambias de pin o lógica, ajusta `led_control.h` y recompila.

---

## Errores típicos y soluciones

- **Pantalla blanca total**: revisa `TFT_BL=21` en HIGH, conexiones, `gfx->begin()`, `setRotation` y que `LV_COLOR_16_SWAP=1`. Si aún invierte colores, deja `CONVERT_TO_BGR=1` (por defecto en este repo).
- **Colores raros**: usa `CONVERT_TO_BGR=1` y verifica `LV_COLOR_16_SWAP=1`.
- **Touch desalineado/rotado**: cambia `TOUCH_ROT` (0/1/2/3) hasta que coincida con la UI.
- **Overflow de DRAM**: reduce líneas de buffer (`LVGL_BUF_LINES` en `main.cpp`, por defecto `12`).
- **Linker no encuentra callbacks de eventos**: asegúrate de que `ui_events.c` fue renombrado a **`.cpp`** y que el **header** `ui_events.h` se incluye al inicio de ese archivo (SquareLine ya trae los `extern "C"` de forma condicional).

---

## Créditos

- [LVGL](https://lvgl.io/) 8.3.x
- [Arduino_GFX](https://github.com/moononournation/Arduino_GFX)
- [SquareLine Studio](https://squareline.io/)
- [XPT2046_Touchscreen](https://github.com/PaulStoffregen/XPT2046_Touchscreen)

