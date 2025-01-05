#
# Copyright (C) 2018 bzt (bztsrc@github)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#

SRCS = /Users/jcsawyer/Development/JcOS/src/main.cpp /Users/jcsawyer/Development/JcOS/src/console.cpp /Users/jcsawyer/Development/JcOS/src/driver.cpp /Users/jcsawyer/Development/JcOS/src/std/printf.c /Users/jcsawyer/Development/JcOS/src/_arch/aarch64/cpu/boot.cpp /Users/jcsawyer/Development/JcOS/src/bsp/device_driver/bcm/bcm2xxx_gpio.cpp /Users/jcsawyer/Development/JcOS/src/bsp/device_driver/bcm/bcm2xxx_pl011_uart.cpp /Users/jcsawyer/Development/JcOS/src/bsp/raspberrypi/raspi_driver.cpp /Users/jcsawyer/Development/JcOS/src/console/null_console.cpp
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -I /Users/jcsawyer/Development/JcOS/src -I /Users/jcsawyer/Development/JcOS/src/std -I /Users/jcsawyer/Development/JcOS/src/_arch -I /Users/jcsawyer/Development/JcOS/src/bsp -I /Users/jcsawyer/Development/JcOS/src/bsp/raspberrypi -I /Users/jcsawyer/Development/JcOS/src/console
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles $(INCLUDES)

all: clean kernel8.img

start.o: src/_arch/aarch64/cpu/boot.s
	aarch64-elf-gcc $(CFLAGS) -c src/_arch/aarch64/cpu/boot.s -o start.o

%.o: %.cpp
	aarch64-elf-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-elf-ld -nostdlib start.o $(OBJS) -T src/bsp/raspberrypi/kernel.ld -o kernel8.elf
	aarch64-elf-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio
