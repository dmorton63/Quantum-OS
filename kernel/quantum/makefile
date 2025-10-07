# Toolchain
CC      := i686-elf-gcc
LD      := i686-elf-ld
AS      := nasm

# Paths
BUILD_DIR := ../../build/kernel/quantum

# Flags
CFLAGS  := -ffreestanding -O2 -Wall -Wextra -I../.. -m32
ASFLAGS := -f elf
LDFLAGS := -m elf_i386

# Sources
SRC_C   := quantum_kernel.c
OBJ_C   := $(addprefix $(BUILD_DIR)/,$(SRC_C:.c=.o))
TARGET  := $(BUILD_DIR)/quantum.o

# Build rules
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ_C)
	@echo "[quantum] Linking objects into $(TARGET)..."
	@mkdir -p $(BUILD_DIR)
	$(LD) $(LDFLAGS) -r -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@echo "[quantum] Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "[quantum] Cleaning..."
	rm -f $(BUILD_DIR)/*.o $(TARGET)