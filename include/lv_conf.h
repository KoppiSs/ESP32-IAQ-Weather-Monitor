#if 1 /* Set to 1 to enable LVGL configuration */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/
/* The ILI9488 TFT screen uses 16-bit RGB565 color */
#define LV_COLOR_DEPTH 16

/*=========================
   MEMORY SETTINGS
 *=========================*/
/* Size of the memory available for LVGL's internal `lv_malloc()` in bytes (>= 2kB) */
/* 64KB is a very safe and standard size for an ESP32 UI */
#define LV_MEM_SIZE (48U * 1024U)

/*==================
   FONT USAGE
 *===================*/
/* Montserrat fonts with ASCII range and some symbols using bpp = 4
 * https://fonts.google.com/specimen/Montserrat */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 0
#define LV_FONT_MONTSERRAT_16 1  /* <-- ENABLED FOR FORECAST */
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1  /* <-- ENABLED FOR SIDE DATA */
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 1  /* <-- ENABLED FOR CENTER ARCS */
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Always set a default font to prevent crashes if a widget lacks styling */
#define LV_FONT_DEFAULT &lv_font_montserrat_16

/*=========================
   TICK CONFIGURATION
 *=========================*/
/* Tell LVGL to use the ESP32's built-in Arduino clock instead of manual ticking */
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())

/*=======================
   FEATURE CONFIGURATION
 *=======================*/
/* Enable the core widgets we are using in the UI */
#define LV_USE_ARC        1
#define LV_USE_LABEL      1
#define LV_USE_TABVIEW    1
#define LV_INDEF_READ_PERIOD 10

#endif /* LV_CONF_H */
#endif /* End of content enable */