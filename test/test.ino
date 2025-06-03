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

    Color() {
      red = 0;
      green = 0;
      blue = 0;
    }

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

    // NOTE: will overflow
    Color plus(Color color) {
      Color c = Color(0);
      c.red = red + color.red;
      c.green = green + color.green;
      c.blue = blue + color.blue;
      return c;
    }

    uint64_t hex() {
      return (red << 16) | (green << 8) | (blue << 0);
    }
};

class CircularPixels {
  private:
    uint64_t _colors[NUMPIXELS];
    float _maxBrightness = 1.0;
    float _spread = 1;
    float _scale = 0.4;

    // TODO: improve performance?
    float gaussianDistribution(float x, float mean, float variance) {
      return 1.0 / sqrt(2.0*M_PI*variance) * pow(M_E, -1.0*(x-mean)*(x-mean)/(2.0*variance));
    }
    
  public:

    void setSpread(float spread) {
      _spread = spread;
      _scale = 1 / gaussianDistribution(0, 0, _spread); // scale the distribution so the peak is 1.0
    }
    
    void setMaxBrightness(float b) {
      _maxBrightness = b;
    }
    
    void clear() {
      for (size_t i = 0; i < NUMPIXELS; i++) {
        _colors[i] = 0;
      }
    }

    // angle: float [0-1) of % of circle to center color
    void setColor(float angle, Color color) {
      for (size_t i = 0; i < NUMPIXELS; i++) {
        float x = (float) i / NUMPIXELS;
        Color current = Color(_colors[i]);
        float normValue =
          // 3 times handles wraps
          (
            gaussianDistribution(x, angle, _spread) +
            gaussianDistribution(x-1.0, angle, _spread) +
            gaussianDistribution(x+1.0, angle, _spread)
          )* _scale;

        _colors[i] = current.plus(color.scale(normValue)).hex();
      }
    }

    void display() {
      Color pixel;
      pixels.clear();
      for (size_t i = 0; i < NUMPIXELS; i++) {
        pixel = Color(_colors[i]);
        pixel.scale(_maxBrightness);
        pixels.setPixelColor(i, pixel.red, pixel.green, pixel.blue);
      }
      pixels.show();
    }
};

CircularPixels cir;

void setup() {
  pixels.begin();
  pixels.setBrightness(DEFAULT_BRIGHTNESS);
  pixels.clear();
  pixels.show();
  Serial.begin(115200);

  cir.setSpread(pow(0.0225, 2)); // determined imperically
  cir.setMaxBrightness(255);
}

Color red = Color(0xff0000);
Color blue = Color(0x00ff00);
Color green = Color(0x0000ff);

void loop() {
  for (uint16_t i = 0; i < 50000; i += 100) {
    cir.clear();
    cir.setColor((float)(i/100)/50000.0, red);
    cir.setColor((float)i/50000.0, green);
    cir.setColor(0.5, blue);
    cir.display();
    delay(100);
  }
}
