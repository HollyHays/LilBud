//Notes: 
//JSON is a simple way to package data as text so computers can send it to eachother
//JSON is a labeled text format so the pico and server agree on what is being sent
//Flask is a small Python tool that lets a computer act like a web server.

//connect to WiFi
#include <WiFi.h>
//make HTTP request
#include <HTTPClient.h>

enum State{
  getMoisture,
  updateLights,
  getInput,
  checkMessage,
  idlePot,
  send,
  tomohana,
  display,
  end
  };

int potMoisture = 0;
//address of flask server: change depending on wifi; runn app.py to get it
const char* SERVER_IP = "172.20.10.9" ; //add IP here
//set up wifi
const char* ssid = "iPhone (3)";
const char* password = "Albus1010$";
//test for sending between pot IDs
const char* DEVICE_ID = "lilbud_01";
const char* TARGET_ID = "lilbud_02";

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

bool getButton(int pin){
  if (digitalRead(pin) == HIGH){
    while (digitalRead(pin) == HIGH){
      delay(100);
    }
    return 1;
  }
  return 0;
}

int updatePosi(int posi, int mod){
  int nPosi = posi+mod;
  if(nPosi > 3){
    nPosi = 0;
  }
  if(nPosi < 0){
    nPosi =3;
  }
  return nPosi;
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

void usermenu(){
    State menuState = tomohana;
    switch (menuState) {
      case send:
        //testing sending message
        //sendMessage("lilbud_01", "LOVE");
        menuState = tomohana;
        break;
      case display:
        break;
      case tomohana:
      int posi;
      posi = 1;
        while(1){
          if (getButton(1)){
            posi = updatePosi(posi,1);
          }
          if (getButton(2)){
            posi = updatePosi(posi,-1);
          }
          if (getButton(3)){
            if(posi == 0){
            }
            if(posi == 1){
              menuState = send;
              break;
            }
            if(posi == 2){
              menuState = display;
              break;
            }
            if(posi == 3){
              menuState = end;
              break;
            }
          }
       }
       break;
      case end:
        return;
        break;
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP());
}

State currentState = idlePot;
 
void loop() {
  sendMessage(DEVICE_ID, TARGET_ID, "LOVE");
  checkInbox(TARGET_ID);
//  switch (currentState) {
//    case getMoisture:
//      readSensor();
//      currentState = updateLights;
//      break;
//    case updateLights:
//      currentState = idlePot;
//      break;
//    case getInput:
//      usermenu();
//      break;
//    case checkMessage:
//        //check any messages on server
//        checkInbox("lilbud_02");
//        currentState = idlePot;
//      break;
//    case idlePot:
//      //if(){
//        //run logic to take any input into menu.
//      //}
//      break;
//    }
}
