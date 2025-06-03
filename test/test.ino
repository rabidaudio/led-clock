#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NUMPIXELS 24
#define PIXEL_PIN 6
#define DEFAULT_BRIGHTNESS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

class Color {
  public:
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Color(uint64_t hex) {
      red   = (hex >> 16) & 0xFF;
      green = (hex >>  8) & 0xFF;
      blue  = (hex >>  0) & 0xFF;
    }

    Color scale(float brightness) {
      Color c = Color(0);
      c.red = red * brightness;
      c.green = green * brightness;
      c.blue = blue * brightness;
      return c;
    }
};

// angle 0-1, fullBrightness 0-1
void displayTime2(float angle, Color color, float fullBrightness, size_t spred) {
  size_t currentPixel = (size_t) floor(angle * NUMPIXELS);
//  size_t nextPixel = (currentPixel + 1) % NUMPIXELS;
  size_t prevPixel = currentPixel == 0 ? (NUMPIXELS - 1) : (currentPixel - 1);
  Color reduced = color.scale(fullBrightness);

  // scale again by the angle
  float v = ((angle * NUMPIXELS) - (float) currentPixel) / spred;
  Color prevColor = reduced.scale(0.5 - v);
  Color currentColor = reduced.scale(0.25 + v);
  Color nextColor = reduced.scale(v);

  pixels.setPixelColor(prevPixel, 0, 0, 0);
  for (size_t p = 0; p < (size_t) spred; p++) {
    pixels.setPixelColor(prevPixel, prevColor.red, prevColor.green, prevColor.blue);  
  }
  
  pixels.setPixelColor(currentPixel, currentColor.red, currentColor.green, currentColor.blue);
  pixels.setPixelColor(nextPixel, nextColor.red, nextColor.green, nextColor.blue);
}

void setup() {
  pixels.begin();
  pixels.setBrightness(DEFAULT_BRIGHTNESS);
  pixels.clear();
  pixels.show();
  Serial.begin(115200);
}

Color foo = Color(0xff0000);

void loop() {
  for (size_t i = 0; i < 255; i++) {
    displayTime2((float) i / 255, foo, 240.0/255);
    pixels.show();
    delay(100);
  }
//  for (size_t j = 0; j < NUMPIXELS; j++) {
//    for (size_t i = 0; i < 255; i++) {
//      pixels.setPixelColor(j, 255 - i, 0, 0);
//      pixels.setPixelColor((j + 1) % NUMPIXELS, i, 0, 0);
//      pixels.show();
//      delay(10);
//    }
//    pixels.setPixelColor(j, 0, 0, 0);
//  }
}
