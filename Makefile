STD_SRCS = ./src/std/printf.cpp ./src/std/minimal_runtime.cpp ./src/std/duration.cpp ./src/std/memory.cpp
STD_INC = -I ./src/std

ARCH_SRCS = ./src/_arch/time.cpp
ARCH_INC = -I ./src/_arch

AARCH64_SRCS = ./src/_arch/aarch64/cpu/boot.cpp ./src/_arch/aarch64/cpu.cpp ./src/_arch/aarch64/time.cpp
AARCH64_INC = -I ./src/_arch/aarch64

BSP_SRCS = ./src/bsp/device_driver/bcm/bcm2xxx_gpio.cpp ./src/bsp/device_driver/bcm/bcm2xxx_pl011_uart.cpp ./src/bsp/device_driver/bcm/bcm2xxx_rng.cpp
BSP_INC = -I ./src/bsp -I ./src/bsp/device_driver/bcm

RASPI_SRCS = ./src/bsp/raspberrypi/raspberrypi.cpp ./src/bsp/raspberrypi/cpu.cpp
RASPI_INC = -I ./src/bsp/raspberrypi

CONSOLE_SRCS = ./src/console/console.cpp ./src/console/null_console/null_console.cpp ./src/bsp/raspberrypi/console/qemu_console.cpp
CONSOLE_INC = -I ./src/console -I ./src/console/null_console -I ./src/bsp/raspberrypi/console

DRIVER_SRCS = ./src/driver/driver.cpp
DRIVER_INC = -I ./src/driver

SRCS = ./src/main.cpp ./src/time.cpp $(STD_SRCS) $(ARCH_SRCS) $(AARCH64_SRCS) $(BSP_SRCS) $(CONSOLE_SRCS) $(DRIVER_SRCS) $(RASPI_SRCS)
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -I ./src $(STD_INC) $(ARCH_INC) $(AARCH64_INC) $(BSP_INC) $(CONSOLE_INC) $(DRIVER_INC) $(RASPI_INC)
DEFINES = -DBOARD=bsp_rpi3bplus
CFLAGS = $(DEFINES) -Wall -O0 -mgeneral-regs-only -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit $(INCLUDES)

all: clean kernel8.img
start.o: src/_arch/aarch64/cpu/boot.s
	aarch64-elf-gcc $(CFLAGS) -c src/_arch/aarch64/cpu/boot.s -o start.o

%.o: %.cpp
	aarch64-elf-gcc $(CFLAGS) -c $< -o $@

kernel8.img: start.o $(OBJS)
	aarch64-elf-ld -nostdlib -g start.o $(OBJS) -T src/bsp/raspberrypi/kernel.ld -o kernel8.elf
	aarch64-elf-objcopy -O binary kernel8.elf kernel8.img

clean:
	rm kernel8.elf *.o >/dev/null 2>/dev/null || true
	find ./ -name '*.o' -delete

run:
	qemu-system-aarch64 -M raspi3b -kernel kernel8.elf -serial stdio -display none
