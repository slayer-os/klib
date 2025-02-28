
CFLAGS := -g -fno-inline-small-functions \
	-Wall \
	-Wextra \
	-std=gnu11 \
	-ffreestanding \
	-fno-stack-protector \
	-fno-stack-check \
	-fno-lto \
	-m64 \
	-march=x86-64 \
	-mno-80387 \
	-mno-mmx \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-O3
LDFLAGS := -nostdlib -z max-page-size=0x1000

INCLUDES := -Isrc/include

OBJ_DIR := build/obj

SRC_DIR := src/libc
LIBC_LIB := build/libc.a


SOURCE_FILES := $(shell find $(SRC_DIR) -name '*.c' -or -name '*.s')
OBJECT_FILES := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(patsubst $(SRC_DIR)/%.s, $(OBJ_DIR)/%.o, $(SOURCE_FILES)))

all: $(LIBC_LIB)

$(LIBC_LIB): $(OBJECT_FILES)
	$(AR) rcs $@ $(OBJECT_FILES)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(LIBC_LIB)

.PHONY: all clean
