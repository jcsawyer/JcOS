BOARD	?=	bsp_rpi3

# src/libc
LIBC_SRC	:=	$(wildcard src/libc/*.cpp) \
				$(wildcard src/libc/stdio/*.cpp)
LIBC_INC	:=	-isystem ./src/libc

# src/kernel
KERNEL_SRC	:=	$(wildcard src/kernel/*.cpp) \
				$(wildcard src/kernel/arch/*.cpp) \
				$(wildcard src/kernel/time/*.cpp) \
				$(wildcard src/kernel/bsp/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/*.cpp) \
				$(wildcard src/kernel/bsp/device_driver/bcm/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/console/*.cpp) \
				$(wildcard src/kernel/bsp/raspberrypi/memory/*.cpp) \
				$(wildcard src/kernel/console/*.cpp) \
				$(wildcard src/kernel/console/null_console/*.cpp) \
				$(wildcard src/kernel/driver/*.cpp)
KERNEL_INC	:=	-isystem ./src/kernel

AARCH64_SRCS :=	$(wildcard src/kernel/arch/aarch64/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/cpu/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/exception/*.cpp) \
				$(wildcard src/kernel/arch/aarch64/memory/*.cpp)

BIN_DIR		:= bin
OBJ_DIR		:= $(BIN_DIR)/objects
SRCS		:= $(KERNEL_SRC) $(LIBC_SRC) $(AARCH64_SRCS)
C_OBJS		:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
INCLUDES	:= $(KERNEL_INC) $(LIBC_INC)
DEFINES		:= -DBOARD=$(BOARD)
CFLAGS		:= -Wall -O0 -mgeneral-regs-only -g -ffreestanding -nostdinc -nostdlib -nostartfiles -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit

all: clean kernel8.img run
$(OBJ_DIR)/start.o: src/kernel/arch
	@echo "  AS\t$@"
	@mkdir -p $(@D)
	@aarch64-elf-gcc $(DEFINES) $(CFLAGS) -c ./src/kernel/arch/aarch64/cpu/boot.s -o $(OBJ_DIR)/start.o

$(OBJ_DIR)/exception.o: src/kernel/arch
	@echo "  AS\t$@"
	@mkdir -p $(@D)
	@aarch64-elf-gcc $(DEFINES) $(CFLAGS) -c ./src/kernel/arch/aarch64/exception.s -o $(OBJ_DIR)/exception.o

$(OBJ_DIR)/%.o: %.cpp
	@echo "  CC\t$<\t\t->\t$@"
	@mkdir -p $(@D)
	@aarch64-elf-gcc $(DEFINES) $(CFLAGS) $(INCLUDES) -c $< -o $@ -MMD -MP

kernel8.img: $(OBJ_DIR)/start.o $(OBJ_DIR)/exception.o $(C_OBJS)
	@mkdir -p $(@D)
	@aarch64-elf-ld -nostdlib -g $(OBJ_DIR)/start.o $(OBJ_DIR)/exception.o $(C_OBJS) -T ./src/kernel/bsp/raspberrypi/kernel.ld -o ./bin/kernel8.elf
	@aarch64-elf-objcopy -O binary ./bin/kernel8.elf ./bin/kernel8.img

format:
	@find ./ -name '*.cpp' | xargs clang-format -i
	@find ./ -name '*.hpp' | xargs clang-format -i
	@find ./ -name '*.h' | xargs clang-format -i

analyze:
	@cppcheck --enable=all --inconclusive --std=c++11 --language=c++ --platform=unix64 --suppress=missingIncludeSystem --suppress=unusedFunction ./src

clean:
	@rm ./bin/kernel8.elf ./bin/kernel8.img *.o >/dev/null 2>/dev/null || true
	@find ./ -name '*.o' -delete

run:
	@qemu-system-aarch64 -M raspi3b -kernel bin/kernel8.elf -serial stdio -display none

.PHONY : all format analyze clean run