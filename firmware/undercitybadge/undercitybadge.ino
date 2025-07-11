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
#include <SdFat_Adafruit_Fork.h>
#include <Adafruit_EPD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ImageReader_EPD.h>

Adafruit_SSD1680 display(296, 128, _DC_EPD, _RESET_EPD, _CS_EPD, _CS_SRAM, _BUSY_EPD, &SPI);

SdFat SD;
Adafruit_ImageReader_EPD reader(SD);

volatile bool recvBuffReady = false;
char recvBuff[45] = "";
int recvIdx = 0;
void recvCallback(uint8_t *data, size_t len) {
  memcpy(recvBuff + recvIdx, data, len);
  recvIdx += len;
  if (recvIdx == sizeof(recvBuff)) {
    recvBuffReady = true;
    recvIdx = 0;
  }
}

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);

#define SD_CONFIG SdSpiConfig(_CS_SD, SHARED_SPI, SD_SCK_MHZ(1000000), &SPI1)

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel stripl = Adafruit_NeoPixel(3, NEOPIXEL_L, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripr = Adafruit_NeoPixel(3, NEOPIXEL_R, NEO_GRB + NEO_KHZ800);

void setup1() {
  SPISlave.setTX(_MISO_PGM);
  SPISlave.setRX(_MOSI_PGM);
  SPISlave.setSCK(_CLK_PGM);
  SPISlave.setCS(_CS_PGM);

  // Ensure we start with something to send...
  // sentCallback();
  // Hook our callbacks into the slave
  SPISlave.onDataRecv(recvCallback);
  // SPISlave1.onDataSent(sentCallback);
  SPISlave.begin(spisettings);
  delay(3000);
  Serial.println("S-INFO: SPISlave started");
}

void reDraw() {
  // NOTE: set rotation of display here, idk what the correct setting it is on the hardware
  display.setRotation(2);

  display.clearBuffer();
  display.fillScreen(EPD_WHITE);

  display.setTextWrap(true);
  display.setCursor(10, 10);
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  display.print("Hack Club Undercity");
  display.display();

  delay(2000);

  Serial.print("Loading image");

  ImageReturnCode stat = reader.drawBMP((char *)"/badge.bmp", display, 0, 0);

  if (stat == IMAGE_SUCCESS) {
    Serial.println("Image loaded successfully!");
    display.display();
  } else {
    Serial.print("Image loading failed: ");
    reader.printStatus(stat);
    display.clearBuffer();
    display.fillScreen(EPD_WHITE);
    display.setCursor(10, 10);
    display.setTextSize(1);
    display.setTextColor(EPD_BLACK);
    display.print("Error loading image!");
    display.display();
  }

  delay(2000);

  Serial.println("Loading name.");
  display.setTextWrap(true);
  display.setCursor(10, 40);
  display.setTextSize(2);
  display.setTextColor(EPD_BLACK);
  File32 name = SD.open("name.txt");
  String s = "";
  if (name) {
    Serial.println("name.txt:");

    while (name.available()) {
      s += name.read();
    }
    Serial.println(s);
    display.print(s);

    name.close();
  } else {
    Serial.println("Error opening name.txt");
    display.print("NONAME");
  }

  display.display();

  delay(2000);
}

#include "badge.h"
void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(115200);
  pinMode(_MISO_PGM, OUTPUT);
  digitalWrite(_MISO_PGM, 1);

  stripr.begin();
  stripr.setBrightness(50);
  stripr.show();
  stripl.begin();
  stripl.setBrightness(50);
  stripl.show();

  while (!Serial) {
    delay(1); // wait for serial port to connect. Needed for native USB port only (aka the rp2350 i think maybe, idk it was in the example rp2350 code)
  }

  Serial.println("\nInitializing SD card...");

  // Set up SPI pins for the SD card
  SPI1.setMISO(_MISO_SD);
  SPI1.setMOSI(_MOSI_SD);
  SPI1.setSCK(_SCK_SD);
  SPI1.setCS(_CS_SD);

  if (!SD.begin(SD_CONFIG)) {
    Serial.println("SD initialization failed!");
  } else {
    Serial.println("SD initialization done.");
  }

  // FAQ THIS IMA BRUTE FORCE THIS I DONT CARE IF SDCARDS DIE
  File32 badge_bmp = SD.open("badge.bmp", FILE_WRITE);

  // if the file opened okay, write to it:
  if (badge_bmp) {
    Serial.print("Writing to badge.bmp...");
    badge_bmp.print(badge_bmp, badge_bmp_len);
    // close the file:
    badge_bmp.close();
    Serial.println("Done saving pic.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening badge.bmp");
  }

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
      Serial.println("Error opening test.txt");
    }

    reDraw();
  }

  colorWipe(stripl.Color(255, 0, 0), 50); // Red
  colorWipe(stripl.Color(0, 255, 0), 50); // Green
  colorWipe(stripl.Color(0, 0, 255), 50); // Blue

  // IMG
  /*
  if (recvBuffReady && !mode) {
    Serial.println("Loading image.");
    File32 name = SD.open("badge.bmp", FILE_WRITE);

    // if the file opened okay, write to it:
    if (name) {
      Serial.print("Writing to badge.bmp...");
      name.print(recvBuff);
      // close the file:
      name.close();
      Serial.println("Done saving pic.");
    } else {
      // if the file didn't open, print an error:
      Serial.println("Error opening test.txt");
    }

    reDraw();
  }
  */
}
