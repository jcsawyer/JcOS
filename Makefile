BOARD	?=	bsp_rpi3

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
				$(wildcard src/kernel/exceptions/asynchronous/*.cpp)
KERNEL_INC	:=	-isystem ./src/kernel

AARCH64_SRCS :=	$(wildcard src/kernel/arch/aarch64/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/cpu/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/exception/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/mailbox/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/memory/*.cpp)
AARCH64_ASM	:=	$(wildcard src/kernel/arch/aarch64/*.s) \
				$(wildcard src/kernel/arch/aarch64/cpu/*.s)

BSP_RPI_SRCS:=	$(wildcard src/kernel/bsp/device_driver/bcm/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/lcd/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/console/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/mailbox/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/memory/*.cpp)

BIN_DIR		:= bin
OBJ_DIR		:= $(BIN_DIR)/objects
ASM_OBJ_DIR	:= $(BIN_DIR)/asm_objects
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
INCLUDES	:= $(KERNEL_INC) $(LIBC_INC)
DEFINES		:= -DARCH=$(ARCH) -DBOARD=$(BOARD)
CFLAGS		:= -Wall -O0 -mgeneral-regs-only -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -mno-outline-atomics
LDFLAGS		:= -nostdlib -g

all: check-args clean format kernel8.img run
$(ASM_OBJ_DIR)/%.o: %.s
	@echo "  AS\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@aarch64-elf-gcc $(DEFINES) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "  CC\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@aarch64-elf-gcc $(DEFINES) $(CFLAGS) $(INCLUDES) -c $< -o $@ -MMD -MP

kernel8.img: $(ASM_OBJS) $(C_OBJS)
	@mkdir -p $(@D)
	@aarch64-elf-ld $(LDFLAGS) $(ASM_OBJS) $(C_OBJS) -T ./src/kernel/bsp/raspberrypi/kernel.ld -o ./bin/kernel8.elf
	@aarch64-elf-objcopy -O binary ./bin/kernel8.elf ./bin/kernel8.img

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
.PHONY : all format analyze clean run check-args
