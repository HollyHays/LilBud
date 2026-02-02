#include <FastLED.h>

//LED STRIP CONFIGURATION
//sends data to LED
#define DATA_PIN 6 //NOT OFFICIAL PIN NUM JUST THREW A NUMBER IN FOR NOW
//how many leds are on the strip
#define NUM_LEDS 300 //NOT OFFICIAL LED NUM JUST THREW A NUMBER IN FOR NOW
//tells FastLED what protocol the leds use
#define LED_TYPE WS2811
#define COLOR_ORDER GRB //Can change this if the order ends up being a little different
#define BRIGHTNESS 80       //0-255

//LED array
CRGB leds[NUM_LEDS];

void setup() {
  //starts serial comms for debugging
  Serial.begin(115200);
  Serial.println("Starting LED test");

  //tells FastLED all the info
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  //sets all leds to turn off
  FastLED.clear();
  //sends data from pico to strip
  FastLED.show();
}

//this test will hold each color for two seconds and repeat
void loop() {
  //BLUE
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(2000);

  //RED
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(2000);

  //GREEN
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(2000);

}
