#include <FastLED.h>

//LED STRIP CONFIG
//sends data to LED
#define DATA_PIN 6 //NOT OFFICIAL PIN NUM JUST THREW A NUMBER IN FOR NOW
//how many leds are on the strip
#define NUM_LEDS 300 //NOT OFFICIAL LED NUM JUST THREW A NUMBER IN FOR NOW
//tells FastLED what protocol the leds use
#define LED_TYPE WS2811
#define COLOR_ORDER GRB //Can change this if the order ends up being a little different
#define BRIGHTNESS 80       //0-255

//MOISTURE SENSOR CONFIG
#define MOIST_PIN 1
const int WET_ADC = 1200;    // test, then adjust this (just threw in a number)!!
const int DRY_ADC = 3200;    // test, then adjust this (just threw in a number)!!
int potMoisture = 0;

//LED array
CRGB leds[NUM_LEDS];


int alphafilter(int nMoisture){
  int alpha = 4;
  int fMoisture =
    (nMoisture*alpha + (10-alpha)*potMoisture)/10;
  return fMoisture;
}

int readSensor(){
  int pin = 1;
  int cMoisture = analogRead(pin);
  potMoisture = alphafilter(cMoisture);
  return cMoisture;
}

void setup() {
  //starts serial comms for debugging
  Serial.begin(115200);
  //tells FastLED all the info
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  //sets all leds to turn off
  FastLED.clear();
  //sends data from pico to strip
  FastLED.show();

  //initialize filter to first read
  potMoisture = analogRead(MOIST_PIN);
}
// 0 = wet (blue), 1 = dry (red)
float drynessSmooth = 0.0f; 
const flaot colorAlpha = 0.05f; //lower = slower; ADJUST LATER

void loop() {
  // updates potMoisture
  int no_filt = readSensor(); 
  int filt = potMoisture;

  float dryness = (float)(filt - WET_ADC) / (float)(DRY_ADC - WET_ADC);

  //smooth dryness with filter
  drynessSmooth = (1.0f - colorAlpha) * drynessSmooth + colorAlpha * dryness;

  //turns dryness to color, using FastLED
  CRGB color;
  color.r = 255*drynessSmooth;
  color.g = 0;
  color.b = 255*(1.0f - drynessSmooth);

  //send data to leds
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();

  delay(100);
}
