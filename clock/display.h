#pragma once

#include <stddef.h>
#include <stdint.h>

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

template <size_t NUMPIXELS>
class CircularPixels {
  private:
    uint64_t _pixels[NUMPIXELS];
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
    
    void clear() {
      for (size_t i = 0; i < NUMPIXELS; i++) {
        _pixels[i] = 0;
      }
    }

    // angle: float [0-1) of % of circle to center color
    void setColor(float angle, Color color) {
      for (size_t i = 0; i < NUMPIXELS; i++) {
        float x = (float) i / NUMPIXELS;
        Color current = Color(_pixels[i]);
        float normValue =
          // 3 times handles wraps
          (
            gaussianDistribution(x, angle, _spread) +
            gaussianDistribution(x-1.0, angle, _spread) +
            gaussianDistribution(x+1.0, angle, _spread)
          ) * _scale;

        _pixels[i] = current.plus(color.scale(normValue)).hex();
      }
    }

    Color get(size_t idx) {
        return Color(_pixels[idx]);
    }
};
