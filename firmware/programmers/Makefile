all:
	arduino-cli compile --fqbn rp2040:rp2040:generic_rp2350 --output-dir ./build

install-deps:
	arduino-cli config add board_manager.additional_urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
	arduino-cli core update-index
	arduino-cli core install rp2040:rp2040

	arduino-cli lib install "SdFat - Adafruit Fork"
	arduino-cli lib install "Adafruit EPD"
	arduino-cli lib install "Adafruit ImageReader Library"
	arduino-cli lib install "SerialTransfer.h"

