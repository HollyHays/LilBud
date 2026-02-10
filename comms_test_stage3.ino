//connect to WiFi
#include <WiFi.h>
//make HTTP request
#include <HTTPClient.h>
//converting JSON to LCD ready code
#include <ArduinoJson.h>

//address of flask server: change depending on wifi; runn app.py to get it
const char* SERVER_IP = "172.20.10.9" ; 
//set up wifi
const char* ssid = "iPhone (3)";
const char* password = "Albus1010$";
//test for sending between pot IDs
const char* DEVICE_ID = "lilbud_02"; //from_id (01 for pico on breadboard)
const char* TARGET_ID = "lilbud_01"; //to_id

//function for sending messages
void sendMessage(const char* from_id, const char* to_id, const char* msg) {
  //pico's web messenger
  HTTPClient http;
  //builds the url
  String url = "http://" + String(SERVER_IP) + ":5000/send";

  http.begin(url);
  //sending JSON data since Flask server expects JSON
  http.addHeader("Content-Type", "application/json");
  
  //message format that server understands
  String payload = String("{\"from_id\":\"") + from_id + 
                   "\",\"to_id\":\"" + to_id + 
                   "\",\"message\":\"" + msg + "\"}";
  
  //check
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

// Global variable to hold the last received message
String lastMsgText = "";
String lastSender = "";

//function for receiving messages
void checkInbox(const char* device_id) {
  //web browser inside pico
  HTTPClient http;
  //builds url
  String url = "http://" + String(SERVER_IP) + ":5000/inbox?device_id=" + String(device_id);
  
  http.begin(url);
  //sends request: pico --> server
  int code = http.GET();

  //success code (200 = OK)
  if (code == 200) {
    String response = http.getString();
    
    // Create a buffer for the JSON document
    // 512 bytes: plenty for a few short messages
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, response);

    if (!error) {
      JsonArray messages = doc["messages"];
      
      //If the array isn't empty, extract the first message
      if (messages.size() > 0) {
        lastSender = messages[0]["from_id"].as<String>();
        lastMsgText = messages[0]["message"].as<String>();

        //check
        Serial.println("--- New Message Received ---");
        Serial.print("From: "); Serial.println(lastSender);
        Serial.print("Msg: ");  Serial.println(lastMsgText);
      }
    } else {
      Serial.print("JSON Parse Failed: ");
      Serial.println(error.c_str());
    }
  }

  //closes connection and frees memory
  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("Connected!");

}

void loop() {
  //Send a test message every 10 seconds
  Serial.println("Sending test message...");
  sendMessage(DEVICE_ID, TARGET_ID, "Hello from Pico!");

  delay(2000);

  //Check for any incoming messages
  Serial.println("Checking for messages...");
  checkInbox(DEVICE_ID);

  //Wait 8 seconds before doing it all again
  delay(8000);

}
