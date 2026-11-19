# Ps5Downloadly - Native Homebrew & ShadowMountPlus FFPKG Makefile
# Compatible with ps5-payload-sdk and ShadowMountPlus (kstuff)

TITLE_ID := PPSA99188
TARGET_ELF := Ps5Downloadly.elf
OUTPUT_DIR := dist
APP_DIR := $(OUTPUT_DIR)/$(TITLE_ID)
FFPKG_OUTPUT := $(OUTPUT_DIR)/$(TITLE_ID).ffpkg

CC ?= clang
CFLAGS ?= -O2 -Wall
CFLAGS += -D__PS5__ -Isrc
LDFLAGS += -lSceVideoOut -lScePad -lSceUserService -lpthread

SRCS := src/main.c \
        src/core/tinyurl.c \
        src/core/downloader.c \
        src/ui/keyboard.c \
        src/ui/ui.c \
        src/platform/ps5_hal.c

all: $(OUTPUT_DIR)/$(TARGET_ELF) app_dir

$(OUTPUT_DIR)/$(TARGET_ELF): $(SRCS)
	@mkdir -p $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
	@echo "[+] PS5 ELF successfully built: $@"

# Prepares the ShadowMountPlus app directory
app_dir: $(OUTPUT_DIR)/$(TARGET_ELF)
	@echo "[*] Preparing ShadowMountPlus app directory ($(TITLE_ID))..."
	@mkdir -p $(APP_DIR)/sce_sys
	@cp $(OUTPUT_DIR)/$(TARGET_ELF) $(APP_DIR)/eboot.bin
	@cp package/param.json $(APP_DIR)/sce_sys/param.json
	@if [ -f package/icon0.png ]; then cp package/icon0.png $(APP_DIR)/sce_sys/icon0.png; fi
	@echo "[+] Ready app directory created at: $(APP_DIR)"

# Generates UFS2 .ffpkg container using makefs
ffpkg: app_dir
	@echo "[*] Packaging $(TITLE_ID).ffpkg using makefs (UFS2)..."
	@if command -v makefs >/dev/null 2>&1; then \
		makefs -t ffs -B little -o version=2,bsize=32768,fsize=4096,minfree=0 $(FFPKG_OUTPUT) $(APP_DIR); \
		echo "[+] Generated $(FFPKG_OUTPUT) successfully!"; \
	else \
		echo "[-] Notice: makefs not found on host. Use CI to produce the .ffpkg container."; \
	fi

clean:
	rm -rf $(OUTPUT_DIR)

.PHONY: all clean app_dir ffpkg
