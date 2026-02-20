
#ifndef __LCD_SPI_DRIVER_H__
#define __LCD_SPI_DRIVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <rtthread.h>
#include <rtdevice.h>

// RGB565
#define lcd_color_t uint16_t

#if defined PKG_LCD_SPI_DIRECTION_0
    #define PKG_LCD_SPI_DIRECTION      LCD_SPI_DIRECTION_0
#elif defined PKG_LCD_SPI_DIRECTION_90
    #define PKG_LCD_SPI_DIRECTION      LCD_SPI_DIRECTION_90
#elif defined PKG_LCD_SPI_DIRECTION_180
    #define PKG_LCD_SPI_DIRECTION      LCD_SPI_DIRECTION_180
#elif defined PKG_LCD_SPI_DIRECTION_270
    #define PKG_LCD_SPI_DIRECTION      LCD_SPI_DIRECTION_270
#else
    #define PKG_LCD_SPI_DIRECTION      LCD_SPI_DIRECTION_0
#endif

typedef enum {
    LCD_SPI_DIRECTION_0,
    LCD_SPI_DIRECTION_90,
    LCD_SPI_DIRECTION_180,
    LCD_SPI_DIRECTION_270
}lcd_spi_direction_t;


void lcd_set_windows(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void lcd_set_direction(lcd_spi_direction_t direction);

void lcd_clear(lcd_color_t color);
void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, lcd_color_t *color_buff);


#ifdef __cplusplus
}
#endif

#endif
