#include <TimeLib.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define NUMPIXELS 24
#define PIXEL_PIN 6
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define DEFAULT_BRIGHTNESS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

time_t processSyncMessage() {
  time_t pctime = 0L;
  const time_t DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if(pctime < DEFAULT_TIME) {
       pctime = 0L;
     }
  }
  return pctime;
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}


void displayTime(uint8_t h, uint8_t m, uint8_t s) {
  pixels.clear();
 
  // this method blends colors between two pixels. I don't like it as much
//  float ss = (float) s / 60.0;
//  float mm = ((float) m + ss) / 60.0 * 24.0;
//  m = (uint8_t) floor(mm);
//  mm -= floor(mm);
//  uint8_t mv = (uint8_t) (255.0 * mm);
//  pixels.setPixelColor(m, 0, 255 - mv, 0);
//  pixels.setPixelColor(m == 23 ? 0 : m+1, 0, mv, 0);
//  s = (uint8_t) (ss * 24.0);
//  ss = ss * 24.0;
//  s = (uint8_t) floor(ss);
//  ss -= floor(ss);
//  uint8_t sv = (uint8_t) (255.0 * ss);
//  pixels.setPixelColor(s, 255 - sv, 0, 0);
//  pixels.setPixelColor(s == 23 ? 0 : s+1, sv, 0, 0);

  float ss = (float) s / 60.0;
  float mm = ((float) m + ss) / 60.0;
  float hh = (((float) (h % 12) + mm) / 12.0) * NUMPIXELS;
  h = (uint8_t) floor(hh);
  mm = mm * NUMPIXELS;
  m = (uint8_t) floor(mm);
  ss = ss * NUMPIXELS;
  s = (uint8_t) floor(ss);

  if (h == m && m == s) {
    pixels.setPixelColor(h, 255, 255, 255);
  } else {
    pixels.setPixelColor(h, 0, 0, 255);
    pixels.setPixelColor(m, pixels.getPixelColor(m) | pixels.Color(0, 255, 0));
    pixels.setPixelColor(s, pixels.getPixelColor(s) | pixels.Color(255, 0, 0));
  }
  pixels.show();
}

void errorState() {
  pinMode(13, OUTPUT);
  while (true) {
    digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
    delay(100); 
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
//  if (Serial.available()) {
//    time_t t = processSyncMessage();
//    if (t != 0) {
//      Teensy3Clock.set(t); // set the RTC
//      setTime(t);
//    }
//  }
  displayTime(hour(), minute(), second());
  delay(2500);
}
