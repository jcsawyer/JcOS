STD_SRCS = ./src/kernel/std/printf.cpp ./src/kernel/std/minimal_runtime.cpp ./src/kernel/std/duration.cpp ./src/kernel/std/memory.cpp
STD_INC = -isystem ./src/kernel/std

ARCH_SRCS = ./src/kernel/_arch/time.cpp ./src/kernel/_arch/aarch64/exception/asynchronous.cpp ./src/kernel/_arch/aarch64/exception.cpp ./src/kernel/_arch/aarch64/memory/mmu.cpp
ARCH_INC = -I ./src/kernel/_arch

AARCH64_SRCS = ./src/kernel/_arch/aarch64/cpu/boot.cpp ./src/kernel/_arch/aarch64/cpu.cpp ./src/kernel/_arch/aarch64/time.cpp
AARCH64_INC = -I ./src/kernel/_arch/aarch64

BSP_SRCS = ./src/kernel/bsp/device_driver/bcm/bcm2xxx_gpio.cpp ./src/kernel/bsp/device_driver/bcm/bcm2xxx_pl011_uart.cpp ./src/kernel/bsp/device_driver/bcm/bcm2xxx_rng.cpp
BSP_INC = -I ./src/kernel/bsp -I ./src/kernel/bsp/device_driver/bcm

RASPI_SRCS = ./src/kernel/bsp/raspberrypi/raspberrypi.cpp ./src/kernel/bsp/raspberrypi/cpu.cpp ./src/kernel/bsp/raspberrypi/memory/mmu.cpp
RASPI_INC = -I ./src/kernel/bsp/raspberrypi

CONSOLE_SRCS = ./src/kernel/console/console.cpp ./src/kernel/console/null_console/null_console.cpp ./src/kernel/bsp/raspberrypi/console/qemu_console.cpp
CONSOLE_INC = -I ./src/kernel/console -I ./src/kernel/console/null_console -I ./src/kernel/bsp/raspberrypi/console

DRIVER_SRCS = ./src/kernel/driver/driver.cpp
DRIVER_INC = -I ./src/kernel/driver

SRCS = ./src/kernel/main.cpp ./src/kernel/time.cpp $(STD_SRCS) $(ARCH_SRCS) $(AARCH64_SRCS) $(BSP_SRCS) $(CONSOLE_SRCS) $(DRIVER_SRCS) $(RASPI_SRCS)
OBJS = $(SRCS:.cpp=.o)
INCLUDES = -isystem ./src/kernel $(STD_INC) $(ARCH_INC) $(AARCH64_INC) $(BSP_INC) $(CONSOLE_INC) $(DRIVER_INC) $(RASPI_INC)
DEFINES = -DBOARD=bsp_rpi3bplus
CFLAGS = $(DEFINES) -Wall -O0 -mgeneral-regs-only -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit $(INCLUDES)

all: clean kernel8.img
start.o: ./src/kernel/_arch/aarch64/cpu/boot.s
	@echo "  AS     $@"
	@aarch64-elf-gcc $(CFLAGS) -c ./src/kernel/_arch/aarch64/cpu/boot.s -o start.o

exception.o: ./src/kernel/_arch/aarch64/exception.s
	@echo "  AS     $@"
	@aarch64-elf-gcc $(CFLAGS) -c ./src/kernel/_arch/aarch64/exception.s -o exception.o

%.o: %.cpp
	@echo "  CC     $@"
	@aarch64-elf-gcc $(CFLAGS) -c -o $@ $<

kernel8.img: start.o exception.o $(OBJS)
	@aarch64-elf-ld -nostdlib -g start.o exception.o $(OBJS) -T ./src/kernel/bsp/raspberrypi/kernel.ld -o ./bin/kernel8.elf
	aarch64-elf-objcopy -O binary ./bin/kernel8.elf ./bin/kernel8.img

clean:
	@rm ./bin/kernel8.elf ./bin/kernel8.img *.o >/dev/null 2>/dev/null || true
	@find ./ -name '*.o' -delete

run:
	@qemu-system-aarch64 -M raspi3b -kernel bin/kernel8.elf -serial stdio -display none
