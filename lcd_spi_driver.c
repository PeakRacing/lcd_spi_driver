/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2023-06-10     Vandoul       First version
 */

#include "rtthread.h"
#include "lcd_spi_driver.h"

#define DBG_TAG "lcd_driver"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

// #ifdef PKG_LCD_SPI

// Sleep in
#define LCD_CMD_SLPIN           0x10
// Sleep Out
#define LCD_CMD_SLPOUT          0x11
// Partial Display Mode On
#define LCD_CMD_PTLON           0x12
// Normal Display Mode On
#define LCD_CMD_NORON           0x13
// Display Inversion Off
#define LCD_CMD_INVOFF          0x20
// Display Inversion On
#define LCD_CMD_INVON           0x21
// Display Off
#define LCD_CMD_DISPOFF         0x28
// Display On
#define LCD_CMD_DISPON          0x29
// Column Address Set
#define LCD_CMD_CASET           0x2A
// Row Address Set
#define LCD_CMD_RASET           0x2B
// Memory Write
#define LCD_CMD_RAMWR           0x2C
/*
    D7,D6,D5,D4,D3 
    MY,MX,MV,ML,BGR
*/
// Memory Data Access Control
#define LCD_CMD_MADCTL          0x36
// Interface Pixel Format
#define LCD_CMD_COLMOD          0x3A

typedef struct{
    uint16_t width;
    uint16_t height;
    int8_t offset_x;
    int8_t offset_y;
#ifdef PKG_LCD_SPI_FRAME_BUFF
    lcd_color_t frame_buff[PKG_LCD_SPI_PIXEL_HEIGHT][PKG_LCD_SPI_PIXEL_WIDTH];
#endif
} lcd_custom_t;

static lcd_custom_t lcd_custom_data;

static struct rt_spi_device* lcd_spi_device;

static void lcd_reset(void){
    rt_pin_write(PKG_LCD_SPI_RES_PIN, PIN_LOW);
    rt_thread_mdelay(100);
    rt_pin_write(PKG_LCD_SPI_RES_PIN, PIN_HIGH);
    rt_thread_mdelay(120);
}

static inline void lcd_write_cmd(uint8_t cmd){
    rt_pin_write(PKG_LCD_SPI_DC_PIN, PIN_LOW);
    rt_spi_send(lcd_spi_device, &cmd, 1);
    rt_pin_write(PKG_LCD_SPI_DC_PIN, PIN_HIGH);
}

static inline void lcd_write_data(uint8_t* data, uint16_t length){
    rt_spi_send(lcd_spi_device, data, length);
}

static inline void lcd_write_cmd_data(uint8_t cmd, uint8_t* data, uint16_t length){
    lcd_write_cmd(cmd);
    if(length){
        lcd_write_data(data, length);
    }
}

void lcd_set_windows(int16_t x0, int16_t y0, int16_t x1, int16_t y1){
    lcd_write_cmd_data(LCD_CMD_CASET, (uint8_t[]){(x0+lcd_custom_data.offset_x)>>8, x0+lcd_custom_data.offset_x, (x1+lcd_custom_data.offset_x)>>8, x1+lcd_custom_data.offset_x}, 4);
    lcd_write_cmd_data(LCD_CMD_RASET, (uint8_t[]){(y0+lcd_custom_data.offset_y)>>8, y0+lcd_custom_data.offset_y, (y1+lcd_custom_data.offset_y)>>8, y1+lcd_custom_data.offset_y}, 4);
    lcd_write_cmd_data(LCD_CMD_RAMWR, NULL, 0);
}

void lcd_set_direction(lcd_spi_direction_t direction){
    uint8_t parameter = 0;
#ifdef PKG_LCD_SPI_COLOR_BGR
    parameter |= (1 << 3);
#endif
    switch (direction){
    case LCD_SPI_DIRECTION_0:
        parameter |= (0 << 5) | (0 << 6) | (0 << 7);
        break;
    case LCD_SPI_DIRECTION_90:
        parameter |= (1 << 5) | (1 << 6) | (0 << 7);
        break;
    case LCD_SPI_DIRECTION_180:
        parameter |= (0 << 5) | (1 << 6) | (1 << 7);
        break;
    case LCD_SPI_DIRECTION_270:
        parameter |= (1 << 5) | (0 << 6) | (1 << 7);
        break;
    default:
        break;
    }
    lcd_write_cmd_data(LCD_CMD_MADCTL, (uint8_t[]){parameter}, 1);
}

void lcd_display_off(void) {
    // 处理背光

    lcd_write_cmd(LCD_CMD_DISPOFF);
}

void lcd_display_on(void) {
    // 处理背光

    lcd_write_cmd(LCD_CMD_DISPON);
}

void lcd_sleep(void) {
    // 处理背光

    lcd_write_cmd(LCD_CMD_SLPIN);
}

void lcd_wakeup(void) {
    // 处理背光

    lcd_write_cmd(LCD_CMD_SLPOUT);
}

void lcd_clear(lcd_color_t color){
    lcd_set_windows(0, 0, lcd_custom_data.width - 1, lcd_custom_data.height - 1);
#ifdef PKG_LCD_SPI_FRAME_BUFF
    for (int i = 0; i < lcd_custom_data.height; i++){
        for (int j = 0; j < lcd_custom_data.width; j++){
            lcd_custom_data.frame_buff[i][j] = color;
        }
        lcd_write_data((uint8_t*)lcd_custom_data.frame_buff[i], lcd_custom_data.width * sizeof(lcd_color_t));
    }
#else
    lcd_color_t* buff = (lcd_color_t*)rt_malloc(lcd_custom_data.width * sizeof(lcd_color_t));
    if (buff == NULL){
        return;
    }
    for (int i = 0; i < lcd_custom_data.height; i++){
        for (int j = 0; j < lcd_custom_data.width; j++){
            buff[j] = color;
        }
        lcd_write_data((uint8_t*)buff, lcd_custom_data.width * sizeof(lcd_color_t));
    }
    rt_free(buff);
#endif
}

void lcd_fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, lcd_color_t *color_buff){
#ifdef PKG_LCD_SPI_FRAME_BUFF
    lcd_color_t *pcolor = color_buff;
#endif
    lcd_set_windows(x0, y0, x1, y1);
    for(int i=0; i < (y1 - y0); i++){
#ifdef PKG_LCD_SPI_FRAME_BUFF
        for(int j=0; j < (x1 - x0); j++){
            lcd_custom_data.frame_buff[i + y0][j + x0] = *pcolor++;
        }
#endif
        lcd_write_data((uint8_t*)(color_buff + (i*(x1 - x0 + 1))), (x1 - x0 + 1) * sizeof(lcd_color_t));
    }
}

void lcd_sync_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h){
#ifdef PKG_LCD_SPI_FRAME_BUFF
    lcd_set_windows(x, y, x + w - 1, y + h - 1);
    for (int i = 0; i < h; i++){
        lcd_write_data((uint8_t *)&lcd_custom_data.frame_buff[y+i][x], w * sizeof(lcd_color_t));
    }
#endif
}

#ifdef PKG_LCD_SPI_USING_LCD_ST7789V2
static void lcd_st7789v2_init(void){
    //Porch Setting
    lcd_write_cmd_data(0xB2, (uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    //Gate Control
    lcd_write_cmd_data(0xB7, (uint8_t[]){0x46}, 1);
    //VCOM Setting
    lcd_write_cmd_data(0xBB, (uint8_t[]){0x1B}, 1);
    //LCM Control
    lcd_write_cmd_data(0xC0, (uint8_t[]){0x2C}, 1);
    //VDV and VRH Command Enable
    lcd_write_cmd_data(0xC2, (uint8_t[]){0x01}, 1);
    //VRH set
    lcd_write_cmd_data(0xC3, (uint8_t[]){0x0F}, 1);
    //VDV set
    lcd_write_cmd_data(0xC4, (uint8_t[]){0x20}, 1);
    //Frame Rate Control
    lcd_write_cmd_data(0xC6, (uint8_t[]){0x0F}, 1);
    //Power Control 1
    lcd_write_cmd_data(0xD0, (uint8_t[]){0xA4, 0xA1}, 2);

    lcd_write_cmd_data(0xD6, (uint8_t[]){0xA1}, 1);
    /* set Positive Voltage Gamma */
    lcd_write_cmd_data(0xE0, (uint8_t[]){0xF0,0x00,0x06,0x04,0x05,0x05,0x31,0x44,0x48,0x36,0x12,0x12,0x2B,0x34}, 14);
    /* set Negative Voltage Gamma */
    lcd_write_cmd_data(0xE1, (uint8_t[]){0XF0,0X0B,0X0F,0X0F,0X0D,0X26,0X31,0X43,0X47,0X38,0X14,0X14,0X2C,0X32}, 14);
}
#elif defined PKG_LCD_SPI_USING_LCD_ST7735S
static void lcd_st7735s_init(void){
    lcd_write_cmd_data(0xB1, (uint8_t[]){0x05, 0x3C, 0x3C}, 3);
    lcd_write_cmd_data(0xB2, (uint8_t[]){0x05, 0x3C, 0x3C}, 3);
    lcd_write_cmd_data(0xB3, (uint8_t[]){0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}, 6);
    //------------------------------------End ST7735S Frame Rate---------------------------------//
    lcd_write_cmd_data(0xB4, (uint8_t[]){0x03}, 1);
	//------------------------------------ST7735S Power Sequence---------------------------------//
    lcd_write_cmd_data(0xC0, (uint8_t[]){0x28, 0x08, 0x04}, 3);
    lcd_write_cmd_data(0xC1, (uint8_t[]){0xC0}, 1);
    lcd_write_cmd_data(0xC2, (uint8_t[]){0x0D, 0x00}, 2);
    lcd_write_cmd_data(0xC3, (uint8_t[]){0x8D, 0x2A}, 2);
    lcd_write_cmd_data(0xC4, (uint8_t[]){0x8D, 0xEE}, 2);
	//---------------------------------End ST7735S Power Sequence-------------------------------------//
    lcd_write_cmd_data(0xC5, (uint8_t[]){0x1A}, 1);
	//------------------------------------ST7735S Gamma Sequence---------------------------------//
    lcd_write_cmd_data(0xE0, (uint8_t[]){0x04,0x22,0x07,0x0A,0x2E,0x30,0x25,0x2A,0x28,0x26,0x2E,0x3A,0x00,0x01,0x03,0x13}, 16);
    lcd_write_cmd_data(0xE1, (uint8_t[]){0x04,0x16,0x06,0x0D,0x2D,0x26,0x23,0x27,0x27,0x25,0x2D,0x3B,0x00,0x01,0x04,0x13}, 16);
}
#elif defined PKG_LCD_SPI_USING_LCD_ST7796S
static void lcd_st7796s_init(void){
    lcd_write_cmd_data(0xf0, (uint8_t[]){0xc3}, 1);
    lcd_write_cmd_data(0xf0, (uint8_t[]){0x96}, 1);
    lcd_write_cmd_data(0xb4, (uint8_t[]){0x01}, 1);
    lcd_write_cmd_data(0xb7, (uint8_t[]){0xc6}, 1);
    lcd_write_cmd_data(0xe8, (uint8_t[]){0x40,0x8a,0x00,0x00,0x29,0x19,0xa5,0x33,}, 8);
    lcd_write_cmd_data(0xc1, (uint8_t[]){0x06}, 1);
    lcd_write_cmd_data(0xc2, (uint8_t[]){0xa7}, 1);
    lcd_write_cmd_data(0xc5, (uint8_t[]){0x18}, 1);
    lcd_write_cmd_data(0xe0, (uint8_t[]){0xf0,0x09,0x0b,0x06,0x04,0x15,0x2f,0x54,0x42,0x3c,0x17,0x14,0x18,0x1b}, 14);
    lcd_write_cmd_data(0xe1, (uint8_t[]){0xf0,0x09,0x0b,0x06,0x04,0x03,0x2d,0x43,0x42,0x3b,0x16,0x14,0x17,0x1b}, 14);
    lcd_write_cmd_data(0xf0, (uint8_t[]){0x3c}, 1);
    lcd_write_cmd_data(0xf0, (uint8_t[]){0x69}, 1);
}
#elif defined PKG_LCD_SPI_USING_LCD_NV3030B
static void lcd_nv3030b_init(void){
    lcd_write_cmd_data(0xfd, (uint8_t[]){0x06,0x08}, 2);
    lcd_write_cmd_data(0x61, (uint8_t[]){0x07,0x04}, 2);
    lcd_write_cmd_data(0x62, (uint8_t[]){0x00,0x44,0x45}, 3);
    lcd_write_cmd_data(0x63, (uint8_t[]){0x41,0x07,0x12,0x12}, 4);
    lcd_write_cmd_data(0x64, (uint8_t[]){0x37}, 1);
    lcd_write_cmd_data(0x65, (uint8_t[]){0x09,0x10,0x21}, 3);
    lcd_write_cmd_data(0x66, (uint8_t[]){0x09,0x10,0x21}, 3);
    lcd_write_cmd_data(0x67, (uint8_t[]){0x20,0x40}, 2);
    lcd_write_cmd_data(0x68, (uint8_t[]){0x90,0x4c,0x7C,0x66}, 4);
    lcd_write_cmd_data(0xb1, (uint8_t[]){0x0F,0x02,0x01}, 3);
    lcd_write_cmd_data(0xB4, (uint8_t[]){0x01}, 1);
    lcd_write_cmd_data(0xB5, (uint8_t[]){0x02,0x02,0x0a,0x14}, 4);
    lcd_write_cmd_data(0xB6, (uint8_t[]){0x04,0x01,0x9f,0x00,0x02}, 5);
    lcd_write_cmd_data(0xdf, (uint8_t[]){0x11}, 1);
    lcd_write_cmd_data(0xE2, (uint8_t[]){0x13,0x00,0x00,0x30,0x33,0x3f}, 6);
    lcd_write_cmd_data(0xE5, (uint8_t[]){0x3f,0x33,0x30,0x00,0x00,0x13}, 6);
    lcd_write_cmd_data(0xE1, (uint8_t[]){0x00,0x57}, 2);
    lcd_write_cmd_data(0xE4, (uint8_t[]){0x58,0x00}, 2);
    lcd_write_cmd_data(0xE0, (uint8_t[]){0x01,0x03,0x0d,0x0e,0x0e,0x0c,0x15,0x19}, 8);
    lcd_write_cmd_data(0xE3, (uint8_t[]){0x1a,0x16,0x0C,0x0f,0x0e,0x0d,0x02,0x01}, 8);
    lcd_write_cmd_data(0xE6, (uint8_t[]){0x00,0xff}, 2);
    lcd_write_cmd_data(0xE7, (uint8_t[]){0x01,0x04,0x03,0x03,0x00,0x12}, 6);
    lcd_write_cmd_data(0xE8, (uint8_t[]){0x00,0x70,0x00}, 3);
    lcd_write_cmd_data(0xEc, (uint8_t[]){0x52}, 1);
    lcd_write_cmd_data(0xF1, (uint8_t[]){0x01,0x01,0x02}, 3);
    lcd_write_cmd_data(0xF6, (uint8_t[]){0x09,0x10,0x00,0x00}, 4);
    lcd_write_cmd_data(0xfd, (uint8_t[]){0xfa,0xfc}, 2);
}
#endif

static void lcd_pin_init(void){
    rt_pin_mode(PKG_LCD_SPI_DC_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(PKG_LCD_SPI_RES_PIN, PIN_MODE_OUTPUT);
    // rt_pin_mode(PKG_LCD_SPI_BLK_PIN, PIN_MODE_OUTPUT);
}

static void lcd_set_info(void){
    switch (PKG_LCD_SPI_DIRECTION){
    case LCD_SPI_DIRECTION_0:
    case LCD_SPI_DIRECTION_180:
        lcd_custom_data.width = PKG_LCD_SPI_PIXEL_WIDTH;
        lcd_custom_data.height = PKG_LCD_SPI_PIXEL_HEIGHT;
        lcd_custom_data.offset_x = PKG_LCD_SPI_X_OFFSET;
        lcd_custom_data.offset_y = PKG_LCD_SPI_Y_OFFSET;
        break;
    case LCD_SPI_DIRECTION_90:
    case LCD_SPI_DIRECTION_270:
        lcd_custom_data.width = PKG_LCD_SPI_PIXEL_HEIGHT;
        lcd_custom_data.height = PKG_LCD_SPI_PIXEL_WIDTH;
        lcd_custom_data.offset_x = PKG_LCD_SPI_Y_OFFSET;
        lcd_custom_data.offset_y = PKG_LCD_SPI_X_OFFSET;
        break;
    default:
        break;
    }
}

rt_err_t spi_device_attach(const char *bus_name, const char *device_name, rt_base_t cs_pin){
    RT_ASSERT(bus_name != RT_NULL);
    RT_ASSERT(device_name != RT_NULL);
    rt_err_t result = RT_EOK;
    /* attach the device to spi bus*/
    struct rt_spi_device* spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
    RT_ASSERT(spi_device != RT_NULL);

    result = rt_spi_bus_attach_device_cspin(spi_device, device_name, bus_name, cs_pin, RT_NULL);
    if (RT_EOK != result){
        LOG_E("%s attach to %s faild, %d\n", device_name, bus_name, result);
    }else{
        LOG_I("%s attach to %s done", device_name, bus_name);
    }

    return result;
}

rt_err_t lcd_dev_ops_init(rt_device_t dev){
    rt_err_t res = RT_EOK;

    spi_device_attach(PKG_LCD_SPI_BUS_NAME, PKG_LCD_SPI_DEVICE_NAME, PKG_LCD_SPI_CS_PIN);
    lcd_spi_device = (struct rt_spi_device *)rt_device_find(PKG_LCD_SPI_DEVICE_NAME);
    if (lcd_spi_device != RT_NULL){
        struct rt_spi_configuration spi_config;
        spi_config.data_width = 8;
        spi_config.max_hz = PKG_LCD_SPI_FREQ;
        spi_config.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
        rt_spi_configure(lcd_spi_device, &spi_config);
    }else{
        res = -RT_ERROR;
    }

    lcd_pin_init();

    lcd_reset();
    rt_thread_mdelay(120);
    lcd_write_cmd(LCD_CMD_SLPOUT);
    rt_thread_mdelay(200);
    
    lcd_set_info();
#ifdef PKG_LCD_SPI_USING_LCD_ST7789V2
    lcd_st7789v2_init();
#elif defined PKG_LCD_SPI_USING_LCD_ST7735S
    lcd_st7735s_init();
#elif defined PKG_LCD_SPI_USING_LCD_ST7796S
    lcd_st7796s_init();
#elif defined PKG_LCD_SPI_USING_LCD_NV3030B
    lcd_nv3030b_init();
#endif
    // RGB565
    lcd_write_cmd_data(LCD_CMD_COLMOD, (uint8_t[]){0x55}, 1);

    lcd_set_direction(PKG_LCD_SPI_DIRECTION);

    // 反显
#ifdef PKG_LCD_SPI_DISPLAY_INVERSION
    lcd_write_cmd(LCD_CMD_INVON);
#else
    lcd_write_cmd(LCD_CMD_INVOFF);
#endif
    lcd_clear(0xFFFF);              // 颜色默认黑色

    lcd_write_cmd(LCD_CMD_DISPON);  // Display On

    // 背光

    return res;
}

rt_err_t lcd_dev_ops_open(rt_device_t dev, rt_uint16_t oflag){
    return RT_ERROR;
}

rt_err_t lcd_dev_ops_close(rt_device_t dev){
    return RT_ERROR;
}

rt_ssize_t lcd_dev_ops_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size){
#ifdef PKG_LCD_SPI_FRAME_BUFF
    rt_memcpy(buffer, &((lcd_color_t*)lcd_custom_data.frame_buff)[pos], size * sizeof(lcd_color_t));
    return RT_EOK;
#else
    return -RT_ERROR;
#endif
}

rt_ssize_t lcd_dev_ops_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size){
#ifdef PKG_LCD_SPI_FRAME_BUFF
    rt_memcpy(&((lcd_color_t *)lcd_custom_data.frame_buff)[pos], buffer, size * sizeof(lcd_color_t));
    return RT_EOK;
#else
    return -RT_ERROR;
#endif
}

rt_err_t lcd_dev_ops_control(rt_device_t dev, int cmd, void *args){
    switch(cmd){
        case RTGRAPHIC_CTRL_RECT_UPDATE:{
            struct rt_device_rect_info *rect_info = (struct rt_device_rect_info *)args;
            lcd_sync_frame(rect_info->x, rect_info->y, rect_info->width, rect_info->height);
        }
        case RTGRAPHIC_CTRL_POWERON:{
            break;
        }
        case RTGRAPHIC_CTRL_POWEROFF:{
            break;
        }
        case RTGRAPHIC_CTRL_GET_INFO:{
            struct rt_device_graphic_info *info = args;
            if(info == RT_NULL){
                return -RT_ERROR;
            }
            info->pixel_format   = RTGRAPHIC_PIXEL_FORMAT_RGB565;
            info->bits_per_pixel = 16;
            info->pitch          = 0;
            info->width          = lcd_custom_data.width;
            info->height         = lcd_custom_data.height;
#ifdef PKG_LCD_SPI_FRAME_BUFF
            info->framebuffer    = (void *)lcd_custom_data.frame_buff;
            info->smem_len       = sizeof(lcd_custom_data.frame_buff);
#else
            info->framebuffer    = NULL;
            info->smem_len       = 0;
#endif
        }
        case RTGRAPHIC_CTRL_SET_MODE:{
            break;
        }
        case RTGRAPHIC_CTRL_GET_EXT:{
            break;
        }
        case RTGRAPHIC_CTRL_SET_BRIGHTNESS:{
            break;
        }
        case RTGRAPHIC_CTRL_GET_BRIGHTNESS:{
            break;
        }
        case RTGRAPHIC_CTRL_GET_MODE:{
            break;
        }
        case RTGRAPHIC_CTRL_GET_STATUS:{
            break;
        }
        case RTGRAPHIC_CTRL_PAN_DISPLAY:{
            break;
        }
        case RTGRAPHIC_CTRL_WAIT_VSYNC:{
            break;
        }
    }
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
const struct rt_device_ops lcd_dev_ops = {
    .init    = lcd_dev_ops_init   ,
    .open    = lcd_dev_ops_open   ,
    .close   = lcd_dev_ops_close  ,
    .read    = lcd_dev_ops_read   ,
    .write   = lcd_dev_ops_write  ,
    .control = lcd_dev_ops_control,
};
#endif

static int lcd_dev_init(void) {
    rt_device_t lcd_device = rt_device_create(RT_Device_Class_Graphic, 0);
    if (!lcd_device){
        return -RT_ERROR;
    }
    #ifdef RT_USING_DEVICE_OPS
    lcd_device->ops = &lcd_dev_ops;
    #else
    lcd_device->init    = lcd_dev_ops_init   ;
    lcd_device->open    = lcd_dev_ops_open   ;
    lcd_device->close   = lcd_dev_ops_close  ;
    lcd_device->read    = lcd_dev_ops_read   ;
    lcd_device->write   = lcd_dev_ops_write  ;
    lcd_device->control = lcd_dev_ops_control;
    #endif
    lcd_device->user_data = &lcd_custom_data;
    rt_device_register(lcd_device, "lcd", RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}
INIT_DEVICE_EXPORT(lcd_dev_init);

static int lcd_spi_test(int argc, char *argv[]) {
    lcd_color_t color_test[] = {0X0000, 0xff00, 0x00ff, 0XFFFF, 0x0000};
    rt_device_t lcd = rt_device_find("lcd");
    if(lcd){
        rt_device_open(lcd, RT_DEVICE_FLAG_RDWR);
        for (uint8_t i = 0; i < 5; i++){
            LOG_I("lcd clear color: 0x%04x", color_test[i]);
            lcd_clear(color_test[i]);
            rt_thread_mdelay(1000);
        }
        rt_device_close(lcd);
    }else{
        LOG_E("lcd not found.");
    }

    return RT_EOK;
}
MSH_CMD_EXPORT(lcd_spi_test, test lcd clear color);

// #endif
