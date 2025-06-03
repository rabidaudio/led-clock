#include <TimeLib.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NUMPIXELS 24
#define PIXEL_PIN 6
#define DEFAULT_BRIGHTNESS 50
#define MIN_BRIGHTNESS 16
// at 10pm, start scaling brightness down
// at 11pm, brightness should be minimum
// at 7am, start scaling brightness back up
// at 8am, brightness shold be full again
#define NIGHT_BRIGHTNESS_START 22 // 11pm
#define NIGHT_BRIGHTNESS_END 8 // 8am

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint8_t maxBrightness = DEFAULT_BRIGHTNESS;

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

void printTime(Stream *s) {
  s->print(hour());
  s->print(":");
  s->print(minute());
  s->print(":");
  s->print(second());
  s->print(" ");
  s->print(year());
  s->print("-");
  s->print(month());
  s->print("-");
  s->print(day());
  s->println(); 
}

void updateNightBrightness(uint8_t h) {
  // chimeOnHour = false;
  uint8_t brightness;
  if (h == NIGHT_BRIGHTNESS_START) {
    // start scaling down
    brightness = (uint8_t) (((float) (60 - minute()) / 60.0) * (maxBrightness - MIN_BRIGHTNESS) + MIN_BRIGHTNESS);
  } else if (h == NIGHT_BRIGHTNESS_END) {
    // start scaling up
    brightness = (uint8_t) (((float) minute() / 60.0) * (maxBrightness - MIN_BRIGHTNESS) + MIN_BRIGHTNESS);
  } else if (h > NIGHT_BRIGHTNESS_END && h < NIGHT_BRIGHTNESS_START) {
    // daytime brightness
    brightness = DEFAULT_BRIGHTNESS;
    // chimeOnHour = true; // enable chiming if during the day
  } else {
    // nighttime brightness
    brightness = MIN_BRIGHTNESS;
  }
  pixels.setBrightness(brightness);
}

void processMessage(Stream *s) {
  if(s->peek() == 'G') { // get time
    s->read();
    printTime(s);
  } else if(s->peek() == 'T') { // set time
    s->read();
    const time_t DEFAULT_TIME = 1357041600; // Jan 1 2013 
    time_t pctime = s->parseInt();
    if(pctime >= DEFAULT_TIME) {
      Teensy3Clock.set(pctime);
      setTime(pctime);
      s->print("Set time: ");
      printTime(s);
    }
  } else if(s->peek() == 'B') { // set brightness
    s->read();
    maxBrightness = s->parseInt();
    updateNightBrightness(hour());
    s->print("Set brightness: ");
    s->println(maxBrightness);
  } else {
    while (s->available()) {
      s->read();
    }
    s->println("?");
    printTime(s);
  }
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}



void displayTime(uint8_t h, uint8_t m, uint8_t s) {
  pixels.clear();
 
  // this method blends colors between two pixels. I don't like it as much
  float ss = (float) s / 60.0;
  float mm = ((float) m + ss) / 60.0 * 24.0;
  m = (uint8_t) floor(mm);
  mm -= floor(mm);
  uint8_t mv = (uint8_t) (255.0 * mm);
  pixels.setPixelColor(m, 0, 255 - mv, 0);
  pixels.setPixelColor(m == 23 ? 0 : m+1, 0, mv, 0);
  s = (uint8_t) (ss * 24.0);
  ss = ss * 24.0;
  s = (uint8_t) floor(ss);
  ss -= floor(ss);
  uint8_t sv = (uint8_t) (255.0 * ss);
  pixels.setPixelColor(s, 255 - sv, 0, 0);
  pixels.setPixelColor(s == 23 ? 0 : s+1, sv, 0, 0);

//  float ss = (float) s / 60.0;
//  float mm = ((float) m + ss) / 60.0;
//  float hh = (((float) (h % 12) + mm) / 12.0) * NUMPIXELS;
//  h = (uint8_t) floor(hh);
//  mm = mm * NUMPIXELS;
//  m = (uint8_t) floor(mm);
//  ss = ss * NUMPIXELS;
//  s = (uint8_t) floor(ss);
//
//  if (h == m && m == s) {
//    pixels.setPixelColor(h, 255, 255, 255);
//  } else {
//    pixels.setPixelColor(h, 0, 0, 255);
//    pixels.setPixelColor(m, pixels.getPixelColor(m) | pixels.Color(0, 255, 0));
//    pixels.setPixelColor(s, pixels.getPixelColor(s) | pixels.Color(255, 0, 0));
//  }
  pixels.show();
}

void errorState() {
  pixels.clear();
  while (true) {
    pixels.setPixelColor(0, 255, 0, 0);
    pixels.show();
    delay(1000);
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.show();
    delay(1000);
  }
}

void setup() {
  setSyncProvider(getTeensy3Time);

  pixels.begin();
  pixels.setBrightness(DEFAULT_BRIGHTNESS);
  pixels.clear();
  pixels.show();

  Serial.begin(115200);
//  while (!Serial); // Don't wait for Serial to open
  delay(100);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
    errorState();
  } else {
    Serial.println("RTC has set the system time");
  }
}

// TODO: timer to make pleasant pwm buzzer sound
// TODO: bluetooth serial module, set time, brightness, alarms
// TODO: fade between?

void loop() {
//  uint8_t s = i % 60;
//  uint8_t m = (i / 60) % 60;
//  uint8_t h = (i / 60 / 60) % 24;
//  displayTime(h, m, s);
//  i++;
//  delay(1);

  uint32_t start = millis();
  if (Serial.available()) {
    processMessage(&Serial);
  }
  uint8_t h = hour();
  uint8_t m = minute();
  uint8_t s = second();
  displayTime(h, m, s);
  updateNightBrightness(h);
//  if (m == 0 && s == 0) {
//    if (chimeEnabled && chimeOnHour) {
//      chime();
//    }
//  }
  uint32_t end = millis();
  if (end > start) // catch wrap-around
    delay(2500 - (end - start));
}
