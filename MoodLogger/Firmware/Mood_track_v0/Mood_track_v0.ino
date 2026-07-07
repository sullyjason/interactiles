#include <FastLED.h>

#include "driver/gpio.h"


#define LED_PIN    43
#define NUM_LEDS   15

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("FastLED RGB cycle test");

  gpio_set_drive_capability((gpio_num_t)LED_PIN, GPIO_DRIVE_CAP_3);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(50);
}

void loop() {
  Serial.println("RED");
  leds[0] = CRGB::Red;
  FastLED.show();
  delay(1000);

  Serial.println("GREEN");
  leds[0] = CRGB::Green;
  FastLED.show();
  delay(1000);

  Serial.println("BLUE");
  leds[0] = CRGB::Blue;
  FastLED.show();
  delay(1000);
}