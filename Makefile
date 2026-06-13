BOARD	?=	bsp_rpi3
CROSS_COMPILE ?= aarch64-elf
CC	:= $(CROSS_COMPILE)-gcc
LD	:= $(CROSS_COMPILE)-ld
OBJCOPY	:= $(CROSS_COMPILE)-objcopy

ifeq ($(BOARD), bsp_rpi3)
	ARCH		= aarch64
	DEFINES		= -DBOARD=bsp_rpi3
	QEMU_DEVICE	= raspi3b
else ifeq ($(BOARD), bsp_rpi4)
	ARCH		= aarch64
	DEFINES		= -DBOARD=bsp_rpi4
	QEMU_DEVICE	= raspi4b
endif

# src/libc
LIBC_SRC	:=	$(wildcard src/libc/*.cpp) \
				$(wildcard src/libc/stdio/*.cpp)
LIBC_INC	:=	-isystem ./src/libc

CHAINLOADER_SRC	:=	src/chainloader/boot.cpp \
				src/chainloader/boot_core.cpp \
				src/chainloader/boot_display.cpp \
				src/chainloader/gpio.cpp \
				src/chainloader/lcd.cpp \
				src/chainloader/main.cpp \
				src/chainloader/timer.cpp \
				src/chainloader/uart.cpp \
				src/libc/memory.cpp \
				src/libc/minimal_runtime.cpp \
				src/libc/stdio/printf.cpp \
				src/kernel/arch/aarch64/cpu.cpp
CHAINLOADER_ASM	:=	src/chainloader/boot.s
CHAINLOADER_ASM_CPP := src/chainloader/relocator.S

# src/kernel
KERNEL_SRC	:=	$(wildcard src/kernel/*.cpp) \
				$(wildcard src/kernel/arch/*.cpp) \
				$(wildcard src/kernel/time/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/*.cpp) \
				$(wildcard src/kernel/bsp/bsp.cpp) \
				$(wildcard src/kernel/console/*.cpp) \
				$(wildcard src/kernel/console/null_console/*.cpp) \
				$(wildcard src/kernel/driver/*.cpp) \
				$(wildcard src/kernel/exceptions/*.cpp) \
				$(wildcard src/kernel/exceptions/asynchronous/*.cpp) \
				$(wildcard src/kernel/userland/*.cpp)
KERNEL_INC	:=	-isystem ./src/kernel

AARCH64_SRCS :=	$(wildcard src/kernel/arch/aarch64/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/cpu/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/exception/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/mailbox/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/memory/*.cpp)
AARCH64_ASM	:=	$(wildcard src/kernel/arch/aarch64/*.s) \
				$(wildcard src/kernel/arch/aarch64/cpu/*.s)

BSP_RPI_SRCS:=	$(wildcard src/kernel/bsp/device_driver/bcm/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/bcm/bcm2xxx_interrupt_controller/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/lcd/*.cpp) \
				$(wildcard src/kernel/bsp/exception/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/console/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/exception/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/mailbox/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/memory/*.cpp)

BIN_DIR		:= bin
OBJ_DIR		:= $(BIN_DIR)/objects
ASM_OBJ_DIR	:= $(BIN_DIR)/asm_objects
CHAINLOADER_BIN_DIR := $(BIN_DIR)/chainloader
CHAINLOADER_OBJ_DIR := $(CHAINLOADER_BIN_DIR)/objects
CHAINLOADER_ASM_OBJ_DIR := $(CHAINLOADER_BIN_DIR)/asm_objects
SRCS		= $(KERNEL_SRC) $(LIBC_SRC)
ASM_SRCS	=

ifeq ($(ARCH), aarch64)
	SRCS		+= $(AARCH64_SRCS)
	ASM_SRCS	+= $(AARCH64_ASM)
endif

ifeq ($(BOARD), bsp_rpi3)
	SRCS		+= $(BSP_RPI_SRCS)
else ifeq ($(BOARD), bsp_rpi4)
	SRCS		+= $(BSP_RPI_SRCS)
endif


C_OBJS		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
ASM_OBJS	:= $(ASM_SRCS:%.s=$(ASM_OBJ_DIR)/%.o)
CHAINLOADER_C_OBJS := $(CHAINLOADER_SRC:%.cpp=$(CHAINLOADER_OBJ_DIR)/%.o)
CHAINLOADER_ASM_OBJS := $(CHAINLOADER_ASM:%.s=$(CHAINLOADER_ASM_OBJ_DIR)/%.o) \
				$(CHAINLOADER_ASM_CPP:%.S=$(CHAINLOADER_ASM_OBJ_DIR)/%.o)
INCLUDES	:= $(KERNEL_INC) $(LIBC_INC)
CHAINLOADER_INC	:=	-isystem ./src/chainloader $(KERNEL_INC) $(LIBC_INC)
DEFINES		:= -DARCH=$(ARCH) -DBOARD=$(BOARD)
CFLAGS		:= -Wall -O0 -mgeneral-regs-only -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -mno-outline-atomics
LDFLAGS		:= -nostdlib -g

all: check-args clean format kernel8.img run
$(ASM_OBJ_DIR)/%.o: %.s
	@echo "  AS\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@$(CC) $(DEFINES) -c $< -o $@

$(CHAINLOADER_ASM_OBJ_DIR)/%.o: %.s
	@echo "  AS\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@$(CC) $(DEFINES) -c $< -o $@

$(CHAINLOADER_ASM_OBJ_DIR)/%.o: %.S
	@echo "  AS\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@$(CC) $(DEFINES) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "  CC\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@$(CC) $(DEFINES) $(CFLAGS) $(INCLUDES) -c $< -o $@ -MMD -MP

$(CHAINLOADER_OBJ_DIR)/%.o: %.cpp
	@echo "  CC\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@$(CC) $(DEFINES) $(CFLAGS) $(CHAINLOADER_INC) -c $< -o $@ -MMD -MP

kernel8.img: $(ASM_OBJS) $(C_OBJS)
	@mkdir -p $(@D)
	@$(LD) $(LDFLAGS) $(ASM_OBJS) $(C_OBJS) -T ./src/kernel/bsp/raspberrypi/kernel.ld -o ./bin/kernel8.elf
	@dotnet run --project ./src/tools/Jc.OS.RaspBootPreCompute -- ./bin/kernel8.elf $(BOARD)
	@$(OBJCOPY) -O binary ./bin/kernel8.elf ./bin/kernel8.img

chainloader8.img: $(CHAINLOADER_ASM_OBJS) $(CHAINLOADER_C_OBJS)
	@mkdir -p $(CHAINLOADER_BIN_DIR)
	@$(LD) $(LDFLAGS) $(CHAINLOADER_ASM_OBJS) $(CHAINLOADER_C_OBJS) -T ./src/chainloader/chainloader.ld -o ./$(CHAINLOADER_BIN_DIR)/chainloader8.elf
	@$(OBJCOPY) -O binary ./$(CHAINLOADER_BIN_DIR)/chainloader8.elf ./$(CHAINLOADER_BIN_DIR)/chainloader8.img

format:
	@find ./ -name '*.cpp' | xargs clang-format -i
	@find ./ -name '*.hpp' | xargs clang-format -i
	@find ./ -name '*.h' | xargs clang-format -i

analyze:
	@cppcheck --enable=all --inconclusive --std=c++11 --language=c++ --platform=unix64 --suppress=missingIncludeSystem --suppress=unusedFunction ./src

clean:
	@rm -r ./bin

run:
	@qemu-system-aarch64 -M $(QEMU_DEVICE) -kernel bin/kernel8.elf -serial stdio -display none

deploy:
	@echo "Deploying to $(BOARD)..."
	@dotnet run --project ./src/tools/JcOS.RaspBootCom -- /dev/tty.usbserial-0001 /Volumes/Data/Development/JcOS/bin/kernel8.img

run-chainloader: chainloader8.img
	@qemu-system-aarch64 -M $(QEMU_DEVICE) -kernel ./$(CHAINLOADER_BIN_DIR)/chainloader8.elf -serial stdio -display none

deploy-chainloader: chainloader8.img
	@echo "Deploying chainloader to $(BOARD)..."
	@dotnet run --project ./src/tools/JcOS.RaspBootCom -- /dev/tty.usbserial-0001 /Volumes/Data/Development/JcOS/bin/chainloader/chainloader8.img


check-args:
ifeq ($(BOARD), bsp_rpi3)
	@echo "Building for Raspberry Pi 3\n"
else ifeq ($(BOARD), bsp_rpi4)
	@echo "Building for Raspberry Pi 4\n"
else
	@echo "Invalid BOARD: $(BOARD)"
	@exit 1
endif

ifeq ($(ARCH), )
	@echo "ARCH is not set"
	@exit 1
endif
.PHONY : all format analyze clean run check-args chainloader8.img run-chainloader deploy-chainloader
