/*
  Hack Club Undercity badge firmware
  Credit: 

  The circuit according to the badge schematic:
   SD card attached to SPI bus on rp2350:
   ************ SPI0 ************
   ** MISO (AKA RX) - gpio12
   ** MOSI (AKA TX) - gpio15
   ** CS            - gpio5
   ** SCK           - gpio14

  Eink Display attached to SPI bus on rp2350:
   ************ SPI0 ************
   ** MISO (AKA RX) - gpio12
   ** MOSI (AKA TX) - gpio15
   ** CS            - gpio5
   ** SCK           - gpio14

*/

/*
    *** DEPENDENCIES ***

  - arduino SPI
  - adafruit SD 
  - earlphilehower arduino-pico
  - adafruit EPD
  - adafruit ImageReader
*/

// This are GP pins for SPI on the Raspberry Pi Pico board, and connect
// to different *board* level pinouts.  Check the PCB while wiring.
// Only certain pins can be used by the SPI hardware, so if if you change
// these be sure they are legal or the program will crash.
constexpr const int _MISO_SD = 12;  // Changed to gpio12 as per schematic
constexpr const int _MOSI_SD = 15;  // Changed to gpio15 as per schematic
constexpr const int _CS_SD = 5;
constexpr const int _SCK_SD = 14;   // Changed to gpio14 as per schematic

// same as above but for display
// Eink display is sharing the SPI bus with the SD card
constexpr const int _MISO_EPD = 12;  // AKA SPI RX (same as SD MISO)
constexpr const int _MOSI_EPD = 15;  // AKA SPI TX (same as SD MOSI)
constexpr const int _CS_EPD = 13;
constexpr const int _SCK_EPD = 14;   // AKA SPI SCK (same as SD SCK)

// more disp pins
constexpr const int _BUSY_EPD = 9;
constexpr const int _DC_EPD = 11;
constexpr const int _RESET_EPD = 10;
constexpr const int _CS_SRAM = -1; // not using external sram ic

// Programma!
constexpr const int _CLK_PGM = 22;
constexpr const int _MOSI_PGM = 20;
constexpr const int _MISO_PGM = 23;
constexpr const int _CS_PGM = 21;

// Neopixels
constexpr const int NEOPIXEL_R = 27;
constexpr const int NEOPIXEL_L = 16;

// Buttons
constexpr const int BUTTON_1 = 4;
constexpr const int BUTTON_2 = 2;
constexpr const int BUTTON_3 = 1;
constexpr const int BUTTON_4 = 0;

#include <SPI.h>
#include <SPISlave.h>
#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ImageReader_EPD.h>
#include <Adafruit_NeoPixel.h>

Adafruit_SSD1680 display(296, 128, _DC_EPD, _RESET_EPD, _CS_EPD, _CS_SRAM, _BUSY_EPD, &SPI);

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

Adafruit_NeoPixel stripl = Adafruit_NeoPixel(3, NEOPIXEL_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripr = Adafruit_NeoPixel(3, NEOPIXEL_R, NEO_GRB + NEO_KHZ800);

void setup1() {}

void drawBMPFromMemory(const uint8_t *bmpData) {
  if (bmpData[0] != 'B' || bmpData[1] != 'M') {
    Serial.println("Not a valid BMP file");
    return;
  }

  uint32_t pixelOffset = bmpData[10] | (bmpData[11] << 8) | (bmpData[12] << 16) | (bmpData[13] << 24);
  int32_t width = bmpData[18] | (bmpData[19] << 8) | (bmpData[20] << 16) | (bmpData[21] << 24);
  int32_t height = bmpData[22] | (bmpData[23] << 8) | (bmpData[24] << 16) | (bmpData[25] << 24);
  uint16_t bpp = bmpData[28] | (bmpData[29] << 8);

  if (bpp != 24) {
    Serial.print("Unsupported BMP bit depth: ");
    Serial.println(bpp);
    return;
  }

  Serial.print("Drawing BMP from memory: ");
  Serial.print(width);
  Serial.print("x");
  Serial.println(height);

  // Each row is aligned to 4 bytes
  uint32_t rowSize = ((width * 3 + 3) / 4) * 4;

  // Draw bottom-up (BMP stores pixels from bottom row to top)
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint32_t pixelIndex = pixelOffset + (height - 1 - y) * rowSize + x * 3;
      uint8_t b = bmpData[pixelIndex];
      uint8_t g = bmpData[pixelIndex + 1];
      uint8_t r = bmpData[pixelIndex + 2];

      // Simple brightness thresholding to black or white
      uint8_t brightness = (r + g + b) / 3;
      uint16_t color = (brightness > 128) ? EPD_WHITE : EPD_BLACK;

      display.drawPixel(x, y, color);
    }
  }
}

#include "badge.h"

void reDraw() {
  // NOTE: set rotation of display here, idk what the correct setting it is on the hardware
  display.setRotation(2);

  display.clearBuffer();
  display.fillScreen(EPD_WHITE);

  Serial.println("Image loaded successfully!");
  drawBMPFromMemory(badge_bmp);
  display.display();

  delay(2000);

  Serial.println("Loading name.");
  display.setTextWrap(true);
  display.setCursor(10, 10);
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.print("NONAME");
  display.display();

  delay(2000);
}
void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(115200);

  stripr.begin();
  stripr.setBrightness(50);
  stripr.show();
  stripl.begin();
  stripl.setBrightness(50);
  stripl.show();

  // Set up SPI pins for the SD card
  SPI1.setMISO(_MISO_SD);
  SPI1.setMOSI(_MOSI_SD);
  SPI1.setSCK(_SCK_SD);
  SPI1.setCS(_CS_SD);

  Serial.println("Opening display");
  display.begin();

  reDraw();
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<stripl.numPixels(); i++) {
    stripl.setPixelColor(i, c);
    stripl.show();
    stripr.setPixelColor(i, c);
    stripr.show();
    delay(wait);
  }
}

void loop() {
  // Read programmer for flashes
  /*
  if (recvBuffReady) {
    Serial.println("Loading name.");
    File32 name = SD.open("name.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (name) {
      Serial.print("Writing to name.txt...");
      name.print(recvBuff);
      // close the file:
      name.close();
      Serial.println("Done saving name.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("Error opening name.txt");
    }

    reDraw();
  }
  */

  colorWipe(stripl.Color(255, 0, 0), 50); // Red
  colorWipe(stripl.Color(0, 255, 0), 50); // Green
  colorWipe(stripl.Color(0, 0, 255), 50); // Blue

}
