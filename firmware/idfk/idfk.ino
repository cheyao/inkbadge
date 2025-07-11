#include <Adafruit_NeoPixel.h>

#define PIN     27  // Pin where the data line is connected
#define NUM_LEDS 3   // Number of LEDs in the strip

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Example: Set all LEDs to red
  setColor(strip.Color(255, 0, 0)); // Red
  delay(1000);
  
  // Set all LEDs to green
  setColor(strip.Color(0, 255, 0)); // Green
  delay(1000);
  
  // Set all LEDs to blue
  setColor(strip.Color(0, 0, 255)); // Blue
  delay(1000);
}

void setColor(uint32_t color) {
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

