#include "led_control.h"

bool led_enabled = true;
int  led_last_pct = 60;

static inline void _apply_pwm_percent(int pct) {
  pct = constrain(pct, 0, 100);
  const int maxDuty = (1 << LEDC_RES_BITS) - 1;
  int duty = map(pct, 0, 100, 0, maxDuty);
#if LED_ACTIVE_LOW
  duty = maxDuty - duty;
#endif
  ledcWrite(LEDC_CH, duty);
}

void led_set_percent(int pct) {
  led_last_pct = constrain(pct, 0, 100);
  if (led_enabled) _apply_pwm_percent(led_last_pct);
}

void led_force_off() {
  const int maxDuty = (1 << LEDC_RES_BITS) - 1;
  int duty = LED_ACTIVE_LOW ? maxDuty : 0;
  ledcWrite(LEDC_CH, duty);
}

void led_init() {
  pinMode(LED_PIN, OUTPUT);
  ledcSetup(LEDC_CH, LEDC_FREQ, LEDC_RES_BITS);
  ledcAttachPin(LED_PIN, LEDC_CH);

  // Autotest: 250 ms ON al 100% y vuelve al valor guardado
  bool was_enabled = led_enabled;
  int  was_pct     = led_last_pct;

  led_enabled = true;
  _apply_pwm_percent(100);
  delay(250);

  led_enabled = was_enabled;
  if (led_enabled) _apply_pwm_percent(was_pct);
  else led_force_off();
}
