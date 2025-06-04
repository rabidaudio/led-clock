#include <TimeLib.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "display.h"

#define NUMPIXELS 24
#define PIXEL_PIN 6
#define DEFAULT_BRIGHTNESS 1.0 // 0.2
//#define MIN_BRIGHTNESS 0.06

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

CircularPixels<NUMPIXELS> cir;

float brightness = DEFAULT_BRIGHTNESS;

Color SECONDS = Color(0xff0000); // red
Color MINUTES = Color(0x00ff00); // green
Color HOURS = Color(0x0000ff); // blue

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

void syncronizeClocks(time_t pctime) {
  // wait until start of a new miliseconds to sync clocks
  delay(1000 - (millis()%1000));
  Teensy3Clock.set(pctime);
  setTime(pctime);
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
      syncronizeClocks(pctime);
      s->print("Set time: ");
      printTime(s);
    }
  } else if(s->peek() == 'B') { // set brightness
    s->read();
    brightness = s->parseFloat();
    s->print("Set brightness: ");
    s->println(brightness);
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
  Color pixel;
  pixels.clear();
  cir.clear();

  float ss = ((float) s + ((float) (millis() % 1000) / 1000)) / 60.0;
  float mm = ((float) m + ss) / 60.0;
  float hh = ((float) (h % 12) + mm) / 12.0;

  cir.setColor(ss, SECONDS);
  cir.setColor(mm, MINUTES);
  cir.setColor(hh, HOURS);

  // draw
  for (size_t i = 0; i < NUMPIXELS; i++) {
    pixel = cir.get(i).scale(brightness);
    pixels.setPixelColor(i, pixel.red, pixel.green, pixel.blue);
  }
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

  syncronizeClocks(getTeensy3Time());

  pixels.begin();
  pixels.setBrightness(255);
  pixels.clear();
  pixels.show();

  cir.setSpread(pow(0.0225, 2)); // determined imperically

  Serial.begin(115200);
  delay(100);
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
    errorState();
  } else {
    Serial.println("RTC has set the system time");
  }
}

void loop() {
  if (Serial.available()) {
    processMessage(&Serial);
  }
  uint8_t h = hour();
  uint8_t m = minute();
  uint8_t s = second();
  displayTime(h, m, s);

//  if (m == 0 && s == 0) {
//    if (chimeEnabled) {
//      chime();
//    }
//  }

  uint32_t timeToDelay = 100 - (millis() % 100);
  delay(timeToDelay);
}
