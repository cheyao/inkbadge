// GxEPD2_HelloWorld.ino by Jean-Marc Zingg
//
// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: https://www.good-display.com/companyfile/32/
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// Supporting Arduino Forum Topics (closed, read only):
// Good Display ePaper for Arduino: https://forum.arduino.cc/t/good-display-epaper-for-arduino/419657
// Waveshare e-paper displays with SPI: https://forum.arduino.cc/t/waveshare-e-paper-displays-with-spi/467865
//
// Add new topics in https://forum.arduino.cc/c/using-arduino/displays/23 for new questions and issues

// see GxEPD2_wiring_examples.h for wiring suggestions and examples
// if you use a different wiring, you need to adapt the constructor parameters!

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>
#include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection_new_style.h"

// or select the display constructor line in one of the following files (old style):
// #include "GxEPD2_display_selection.h"
// #include "GxEPD2_display_selection_added.h"

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
// e.g. for Wemos D1 mini:
//GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEH0154D67
// GxEPD2_BW<GxEPD2_290_GDEY029T94, MAX_HEIGHT(GxEPD2_290_GDEY029T94)> display(GxEPD2_290_GDEY029T94(/*CS=*/ 13, /*DC=*/ 11, /*RST=*/ 10, /*BUSY=*/ 9)); // GDEY029T94  128x296, SSD1680, (FPC-A005 20.06.15)

// for handling alternative SPI pins (ESP32, RP2040) see example GxEPD2_Example.ino
constexpr const int _MISO_EPD = 12;  // AKA SPI RX (same as SD MISO)
constexpr const int _MOSI_EPD = 15;  // AKA SPI TX (same as SD MOSI)
constexpr const int _CS_EPD = 13;
constexpr const int _SCK_EPD = 14;   // AKA SPI SCK (same as SD SCK)
                                     //
constexpr const int NEOPIXEL_R = 27;
constexpr const int NEOPIXEL_L = 16;

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel stripl = Adafruit_NeoPixel(3, NEOPIXEL_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripr = Adafruit_NeoPixel(3, NEOPIXEL_R, NEO_GRB + NEO_KHZ800);

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

SPIClassRP2040 SPIn(spi1, _MISO_EPD, _CS_EPD, _SCK_EPD, _MOSI_EPD);

void setup() {
  SPI1.setMISO(_MISO_EPD);
  SPI1.setMOSI(_MOSI_EPD);
  SPI1.setSCK(_SCK_EPD);
  SPI1.setCS(_CS_EPD);

  stripr.begin();
  stripr.show();

  stripl.begin();
  stripl.show();

  display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  // display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.epd2.selectSPI(SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  helloWorld();
  display.hibernate();
}

const char HelloWorld[] = "Hello World!";

void helloWorld() {
  display.setRotation(2);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HelloWorld);
  }
  while (display.nextPage());
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<stripl.numPixels(); i++) {
    stripl.setPixelColor(i, c);
    stripl.show();

    stripr.setPixelColor(i, c);
    stripr.show();

    delay(20);
  }
}

void loop() {
  colorWipe(stripl.Color(255, 0, 0), 50); // Red
  colorWipe(stripl.Color(0, 255, 0), 50); // Green
  colorWipe(stripl.Color(0, 0, 255), 50); // Blue
}
