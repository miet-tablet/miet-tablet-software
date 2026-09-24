#

BUILD_DIR ?= build

.PHONY: build
build:
	cmake -GNinja -B $(BUILD_DIR) .
	cmake --build $(BUILD_DIR)

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

.PHONY: upload-dfu
upload-dfu:
	dfu-util -a 0 --dfuse-address 0x08000000 -D $(BUILD_DIR)/main.bin

.PHONY: upload-openocd
upload-openocd:
	openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program 0x08000000 $(BUILD_DIR)/main.bin verify reset exit"
