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
//
constexpr const int _MISO_SD = 20;  // Changed to gpio12 as per schematic
constexpr const int _CS_SD = 21;
constexpr const int _SCK_SD = 22;   // Changed to gpio14 as per schematic
constexpr const int _MOSI_SD = 23;  // Changed to gpio15 as per schematic


// Programma! SPI1
constexpr const int _MISO_PGM = 8;
constexpr const int _CS_PGM = 9;
constexpr const int _CLK_PGM = 10;
constexpr const int _MOSI_PGM = 11;

#include <SPI.h>
#include <SdFat_Adafruit_Fork.h>

SPISettings spisettings(1000000, MSBFIRST, SPI_MODE0);
SdFat SD;

#define SD_CONFIG SdSpiConfig(_CS_SD, SHARED_SPI, SD_SCK_MHZ(1000000), &SPI1)

void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(115200);

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

  pinMode(_MISO_PGM, INPUT_PULLDOWN);

  SPI.setTX(_MISO_PGM);
  SPI.setRX(_MOSI_PGM);
  SPI.setSCK(_CLK_PGM);
  SPI.setCS(_CS_PGM);

  SPI.begin();
}

bool flashed = false;

void loop() {
  auto i = digitalRead(_MISO_PGM);
  if (i == HIGH) {
    // Start transaction
    char* name = "Cyao";
    SPI.transfer(static_cast<void*>(name), strlen(name) + 1);
  } else {
    // No badge
    flashed = true;
  }

  delay(300);
}
