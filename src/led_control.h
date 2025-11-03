#pragma once
#include <Arduino.h>

// === Config del LED (ajusta si tu placa difiere) ===
#ifndef LED_PIN
  #define LED_PIN 17          // En varios CYD el LED azul está en GPIO17
#endif
#ifndef LED_ACTIVE_LOW
  #define LED_ACTIVE_LOW 1    // 1 = LED se enciende en LOW
#endif
#ifndef LEDC_CH
  #define LEDC_CH 0
#endif
#ifndef LEDC_FREQ
  #define LEDC_FREQ 5000
#endif
#ifndef LEDC_RES_BITS
  #define LEDC_RES_BITS 8
#endif

// Estado expuesto para ui_events
extern bool led_enabled;   // ON/OFF general
extern int  led_last_pct;  // último porcentaje pedido (0..100)

// Inicializa PWM y hace un autodiagnóstico breve
void led_init();

// Ajusta brillo 0..100 usando PWM (respeta ACTIVE_LOW)
void led_set_percent(int pct);

// Fuerza OFF instantáneo (sin alterar led_last_pct)
void led_force_off();
