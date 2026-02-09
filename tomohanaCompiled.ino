#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//connect to WiFi
#include <WiFi.h>
//make HTTP request
#include <HTTPClient.h>

#define BTN_UP 8
#define BTN_DOWN 10
#define BTN_SELECT 12

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

//address of flask server: change depending on wifi; runn app.py to get it
const char* SERVER_IP = "172.20.10.9" ; //add IP here
//set up wifi
const char* ssid = "iPhone (3)";
const char* password = "Albus1010$";
//test for sending between pot IDs
const char* DEVICE_ID = "lilbud_01";
const char* TARGET_ID = "lilbud_02";

#define MOIST_PIN 1
const int WET_ADC = 1200;    // test, then adjust this (just threw in a number)!!
const int DRY_ADC = 3200;    // test, then adjust this (just threw in a number)!!
int potMoisture = 0;

constexpr int MENU_COUNT = 3;

enum State{
  refressh,
  send,
  tomohana,
  display,
  end
  };

// Common I2C addresses: 0x27 or 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool getButton(int pin){
  if (digitalRead(pin) == HIGH){
    while (digitalRead(pin) == HIGH){
      delay(250);
    }
    return 1;
  }
  return 0;
}

//function for sending messages
void sendMessage(const char* from_id,const char* to_id, const char* msg) {
  //pico's web messenger
  HTTPClient http;
  //builds the url
  String url = "http://" + String(SERVER_IP) + ":5000/send";

  http.begin(url);
  //sending JSON data since Flask server expects JSON
  http.addHeader("Content-Type", "application/json");
  
  //message format that server understands
  //String payload = "{\"from_id\":\"" + String(device_id) + "\",\"msg\":\"" + String(msg) + "\"}";
  String payload =
    String("{\"from_id\":\"") + from_id +
    "\",\"to_id\":\"" + to_id +
    "\",\"msg\":\"" + msg + "\"}";
  
  //checking what we are getting
  Serial.print("POST URL: "); Serial.println(url);
  Serial.print("POST JSON: "); Serial.println(payload);
  
  //sends the message to the server
  int code = http.POST(payload);
  
  //checking
  Serial.print("POST status: "); Serial.println(code);
  String body = http.getString();
  Serial.print("Server says: "); Serial.println(body);
  
  //closes connection and frees memory
  http.end();
}

//function for receiving messages
void checkInbox(const char* device_id) {
  //web browser inside pico
  HTTPClient http;
  //builds url
  String url = "http://" + String(SERVER_IP) + ":5000/inbox?device_id=" + device_id;
  
  http.begin(url);
  //sends reques: pico --> server
  int code = http.GET();

  //success code (400 = not found, 500 = server error)
  if (code == 200) {
    String response = http.getString();
    Serial.println(response);   // for now,it will just print on serial monitor
    // later: will extract the message and display on screen
  }

  http.end();
}

const char* menuItems[MENU_COUNT] = {
  "send",
  "display",
  "end"
};

void refresh() {
  float currentMoisture = analogRead(1);
  char buffer[10];

  dtostrf(currentMoisture, 6, 2, buffer);

  //update lights

  // lcd.clear();
  // lcd.setCursor(0, 1);
  // lcd.print(buffer);

  delay(1500);
}

int updatePosi(int posi, int mod) {
  return (posi + mod + MENU_COUNT) % MENU_COUNT;
}

void updateMenu(int pos) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menuItems[pos]);
}

void usermenu() {
  static State menuState = tomohana;  // persists across calls
  static int posi = 0;

  switch (menuState) {

    case send:
      //call the refresh
      //Holly's server send functions
      // sendMessage("lilbud_01", "LOVE");
      refresh();
      lcd.setCursor(0,0);
      lcd.print("message out ");
      menuState = tomohana;
      break;

    case display:
      //check and display messages
      //get message
      //print message to lcd
      refresh();
      lcd.setCursor(0,0);
      lcd.print("message in");
      menuState = tomohana;
      break;

    case tomohana:
      updateMenu(posi);

      while (true) {
        if (getButton(BTN_UP)) {
          posi = updatePosi(posi, 1);
          updateMenu(posi);
        }

        if (getButton(BTN_DOWN)) {
          posi = updatePosi(posi, -1);
          updateMenu(posi);
        }

        if (getButton(BTN_SELECT)) {
          switch (posi) {
            case 0:
              menuState = send;
              //send message logic
              break;
            case 1:
              menuState = display;
              //display message logic
              break;
            case 2:
              menuState = tomohana;
              break; // idle?
            case 3:
              menuState = end;
              break;
          }
          break;
        }

        delay(10);  // debounce / CPU relief
      }
      break;

    case end:
      lcd.clear();
      lcd.print("Exiting...");
      while (!getButton(BTN_DOWN)) {
        delay(10);
      }
      menuState = tomohana;
      break;
  }
}

int alphafilter(float nMoisture, float potMoisture = 0){
  int alpha = .4;
  int fMoisture = (nMoisture*alpha + (1-alpha)*potMoisture);
  return fMoisture;
}

int readSensor(){
  int pin = 1;
  int cMoisture = analogRead(pin);
  potMoisture = alphafilter(cMoisture);
  return cMoisture;
}

void setup() {
  pinMode(7,INPUT);
  Wire.begin();          // Uses A4 (SDA), A5 (SCL) on Uno
  lcd.init();            // Initialize LCD
  lcd.backlight();       // Turn on backlight

  lcd.setCursor(0, 0);   // Column 0, Row 0
  lcd.print("Welcome to");

  lcd.setCursor(0, 1);   // Column 0, Row 1
  lcd.print("Lil' Bud");
}
void loop() {
  delay(1500);
  while(!getButton(BTN_SELECT)){
    refresh();
  }
  usermenu();

}


