//st7789v.h
#pragma once

#include "stm32f4xx_hal.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include "guitypes.h"

//public flags (config)
#define ST7789V_FLG_DMA  0x08 // DMA enabled
#define ST7789V_FLG_MISO 0x10 // MISO enabled
#define ST7789V_FLG_SAFE 0x20 // SAFE mode (no DMA and safe delay)

#define ST7789V_DEF_COLMOD 0x05 // interface pixel format (5-6-5, hi-color)
#define ST7789V_DEF_MADCTL 0xC0 // memory data access control (mirror XY)

#define ST7789V_COLS 240 //
#define ST7789V_ROWS 320 //

typedef struct _st7789v_config_t {
    SPI_HandleTypeDef *phspi; // spi handle pointer
    uint8_t pinCS;            // CS pin
    uint8_t pinRS;            // RS pin
    uint8_t pinRST;           // RST pin
    uint8_t flg;              // flags (DMA, MISO)
    uint8_t colmod;           // interface pixel format
    uint8_t madctl;           // memory data access control

    uint8_t gamma;
    uint8_t brightness;
    uint8_t is_inverted;
    uint8_t control;
} st7789v_config_t;

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

extern void st7789v_init(void);
extern void st7789v_done(void);
extern void st7789v_clear_C(uint32_t clr);

extern void st7789v_draw_png_ex(uint16_t point_x, uint16_t point_y, FILE *pf, uint32_t clr0, uint8_t rop);

extern void st7789v_inversion_on(void);
extern void st7789v_inversion_off(void);
extern void st7789v_inversion_tgl(void);
extern uint8_t st7789v_inversion_get(void);

extern void st7789v_gamma_next(void);
extern void st7789v_gamma_prev(void);
extern void st7789v_gamma_set(uint8_t gamma);
extern uint8_t st7789v_gamma_get(void);

extern void st7789v_brightness_set(uint8_t brightness);
extern uint8_t st7789v_brightness_get(void);

extern void st7789v_brightness_enable(void);
extern void st7789v_brightness_disable(void);

extern uint32_t st7789v_get_pixel_C(uint16_t point_x, uint16_t point_y);
extern void st7789v_set_pixel_directColor_C(uint16_t point_x, uint16_t point_y, uint16_t noClr);
extern uint16_t st7789v_get_pixel_directColor_C(uint16_t point_x, uint16_t point_y);

extern st7789v_config_t st7789v_config;

extern uint16_t st7789v_reset_delay;

extern void st7789v_enable_safe_mode(void);

extern void st7789v_spi_tx_complete(void);

#ifdef __cplusplus
}
#endif //__cplusplus
