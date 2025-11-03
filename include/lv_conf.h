#ifndef LV_CONF_H
#define LV_CONF_H

// Profundidad de color usada por ILI9341
#define LV_COLOR_DEPTH 16

// SquareLine exige SWAP=1; si vino definido por línea de comandos, lo limpiamos:
#ifdef LV_COLOR_16_SWAP
  #undef LV_COLOR_16_SWAP
#endif
#define LV_COLOR_16_SWAP 1

// Opcional: desactiva log para ahorrar flash
#define LV_USE_LOG 0

#endif // LV_CONF_H
