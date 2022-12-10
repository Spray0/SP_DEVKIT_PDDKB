# PDDKB Readme

<img src="https://img.shields.io/badge/SoftwareVer-1.0.2-green" /> <img src="https://img.shields.io/badge/Hardware-1.1-blue" /> <img src="https://img.shields.io/badge/Spray0-21%2F5-red" />

<img src=".\img\brd.jpg" alt="brd" height = "400" />

## 简介

**PDDKB** 一个USB Typec Power Delivery (PD) Sink端开发工具板。使用安森美半导体(ON Semiconductor)公司的**FUSB302B**作为USBC控制器，与主控**STM32F103T8U6**之间通过iic连接。用于PD供电应用的Sink角色应用开发。

提供了CP2102 UART to USB bridge芯片，用于MCU程序下载（ISP）、Log输出调试。

提供了按钮B0，用于进入MCU串口ISP。

提供了按钮RST，用于复位。

提供了按钮SW，用于按键输入触发，默认用于切换PD电压档（处于PD_READY下）。

提供了RGB LED灯，用于指示程序状态。

提供了一路USBA连接器输出，用于输出PD电压。可接USB负载仪便于观察输出电压。

## 开发参考文档

*FUSB302BCN-D.pdf*

> FUSB302B手册

*USB_PD_R3_0 V2.0 20190829.pdf*

> PD3.0规范文档

## 开发环境

平台：Ubuntu 18.04.2 LTS （AMD64）

编译器：arm-none-eabi-gcc version 5.4.1

下载工具：stm32flash 0.5

PCB：KICAD

## 编译方法

编译(16线程编译)

```
make -j16
```

> ...
>
> build/stm32f1xx_hal_usart.o build/stm32f1xx_hal_i2c.o build/bsi2c.o build/FUSB302B.o build/pd_ufp.o build/system_stm32f1xx.o build/startup_stm32f103xb.o -mcpu=cortex-m3 -mthumb   -specs=nano.specs -TSTM32F103T8Ux_FLASH.ld  -lc -lm -lnosys  -Wl,-Map=build/PDDKB.map,--cref -Wl,--gc-sections -o build/PDDKB.elf
> arm-none-eabi-size build/PDDKB.elf
>    text    data     bss     dec     hex filename
>   15880     124    4300   20304    4f50 build/PDDKB.elf
> arm-none-eabi-objcopy -O ihex build/PDDKB.elf build/PDDKB.hex
> arm-none-eabi-objcopy -O binary -S build/PDDKB.elf build/PDDKB.bin

清除工程文件

```
make clean
```

## 备注

为了正常使用VBUS检测，需要焊接R14,将R15NC处理。
