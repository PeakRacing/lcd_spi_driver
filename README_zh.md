[**English**](./README.md)  | **中文**



# LCD SPI DRIVER

[![Pull Requests Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](http://makeapullrequest.com)[![first-timers-only Friendly](https://img.shields.io/badge/first--timers--only-friendly-blue.svg)](http://www.firsttimersonly.com/)![github license](https://img.shields.io/github/license/PeakRacing/lcd_spi_driver)



github: [PeakRacing/lcd_spi_driver](https://github.com/PeakRacing/lcd_spi_driver) (推荐)

gitee: [PeakRacing/lcd_spi_driver](https://gitee.com/PeakRacing/lcd_spi_driver)(由于同步问题可能导致更新不及时)

## 介绍
​	适用于spi驱动的lcd通用驱动，考虑分辨率与内存原因，仅支持RGB565

**支持驱动:**

- [x] ST7789V2

- [x] ST7735S

- [x] ST7796S

- [x] ILI9341

- [x] ILI9486

- [x] GC9A01

- [x] NV3030B

## 使用说明
​	在 Env 命令行里输入 `menuconfig` 进入配置界面配置工程。

​	勾选使能软件包: `T-Thread online packages → peripheral libraries and drivers → [*] The lcd spi driver`

![lcd_spi_driver](./docs/lcd_spi_driver.png)

​	根据自己的LCD来配置

- LCD型号(Select LCD driver)

- SPI Bus Name(LCD SPI Bus Nam)

- SPI Device Name(LCD SPI Device Name)

- SPI 频率(SPI Frequency)

- SPI 片选引脚(CS Pin)

- LCD 数据选择引脚(DC Pin)

- LCD 复位引脚(RES Pin)

  **这几项为必配置项**

![lcd_spi_config](./docs/lcd_spi_config.png)

之后进入 `LCD Configuration` 配置

- LCD方向(LCD Orientation 可选 0° 90° 180° 270°)
- 分辨率(LCD Pixel Width & LCD Pixel Heigh 以0°时为准, 无需考虑方向会自动修改)
- 偏移(LCD Pixel X Offset & LCD Pixel Y Offset)
- BUFF模式(LCD Use Frame Buff 内部会有一个屏幕大小的BUFF)
- 大小端转换( LCD Swap Data Endian)
- BGR格式(LCD Use BGR Color Format)
- 反显(LCD Enable Display Inversion)

![lcd_config](./docs/lcd_config.png)



## 常见问题

​	**问: 颜色异常怎么办？**

​	答: 检查是否需要大小端转换，BGR格式以及反显



​	**问: 显示无法全屏怎么办?**

​	答: 检查是否需要设置偏移 (可查找屏幕原厂提供的代码里 0x2A 和 0x2B 命令发送参数中是否有进行加减偏移操作)



## 交流群

​	**非技术支持，仅作为兴趣交流**

![Communication](./docs/Communication.png)





