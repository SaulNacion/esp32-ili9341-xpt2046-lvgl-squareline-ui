// ui_events.cpp  (C++)
#include <Arduino.h>
#include "lvgl.h"
#include "ui.h"
#include "led_control.h"

void ui_event_ArcLed(lv_event_t * e) {
  if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
  if (!led_enabled) return;

  lv_obj_t *arc = lv_event_get_target(e);
  int v = lv_arc_get_value(arc);     // 0..100
  led_set_percent(v);
}

void ui_event_BtnLed(lv_event_t * e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;

  led_enabled = !led_enabled;
  if (led_enabled) {
    led_set_percent(led_last_pct);
  } else {
    led_force_off();
  }

  // (Opcional) actualizar label del botón:
  // lv_obj_t *btn = lv_event_get_target(e);
  // lv_obj_t *lbl = lv_obj_get_child(btn, 0);
  // lv_label_set_text(lbl, led_enabled ? "ON" : "OFF");
}
