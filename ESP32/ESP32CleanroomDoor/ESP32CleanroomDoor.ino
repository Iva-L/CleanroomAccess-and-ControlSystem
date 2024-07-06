/*
 *This program was made for the acces door of the clean room in UPAEP using RFID ans an IoT app
 *Authors:
 *Emmanuel Isaac García Sanabria
 *Iván ortiz de Lara
 *Alessia Sanchez Amezcua
*/
/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include <UbidotsEsp32Mqtt.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN 22  // MFRC552 Reset PIN
#define SS_PIN  21  // MFRC552 SS SDA PIN
#define LOCK    13  // DoorLock PIN
#define BUZZER  4  // Buzzer PIN
#define BUTTON  2  //Exit button PIN
#define EXTRACTOR 13 // Extractor activation PIN 
#define AIR_COOLER 12 // AC  activation PIN
#define LIGHTS 14 // Lights activation PIN
#define DEHUMIDIFERS 27 // Dehimidifers activation PIN

/*--WiFi and ubidots--------------------------------------------------------------------------------------------------------------------------------------*/
#define TOKEN "BBUS-wc3ibpEdsyoo7zP6C3Mncv0l81Qnb9"     //Ubidots TOKEN
#define WIFISSID "upaep wifi"  // SSID
#define WIFIPASS ""  //Wifi Pass

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN); 
Ubidots ubidots(TOKEN);

/*--GoogleSheets Info----------------------------------------------------------------------------------------------------------------------------*/
const String WEB_APP_URL = "https://script.google.com/macros/s/AKfycbyuBK_qrVVEab8mUQ6hpRLeCTnvgWgF6DvgdTrf68nZ5bHkJ4nacMt8S3P1qgGQztHp/exec";
const String SHEET_NAME = "KeyLog";

/*--Acceptance IDs-------------------------------------------------------------------------------------------------------------------------------*/
byte acceptedUIDs[3][4] = {
  {0xD7, 0x6F, 0x4E, 0x20},
  {0x47, 0x2A, 0x50, 0x20},
  {0x59, 0xDD, 0xB1, 0xB7}
};

String registeredIDs[3] = {
  "3448848",
  "3444657",
  "3521954"
};
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Callback function for ubidots
* @param none
* @retval none
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print("Command: ");
  bool command = *payload - 48;
  Serial.println(command);
  digitalWrite(LIGHTS, command);
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Setup function for ESP32
* @param none
* @retval none
*/
void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(EXTRACTOR, OUTPUT);
  pinMode(AIR_COOLER, OUTPUT);
  pinMode(LIGHTS, OUTPUT);
  pinMode(DEHUMIDIFERS, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init(); 
  Serial.println("Lectura del UID");
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi");
  WiFi.begin("upaep wifi", "");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");
  Serial.println("------------");

  Serial.println(" Initializing Ubidots Connection...");
  ubidots.connectToWifi(WIFISSID, WIFIPASS);
  ubidots.setDebug(true);                        // Pass a true or false bool value to activate debug messages
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  Serial.println(" Initializing Ubidots Connection...");
  ubidots.subscribeLastValue("cleanroom","lights");
  Serial.println("DONE");
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Checks if the UID is acceptable
* @param uid the card UID
* @param size the size of UID
* @retval the accepted UID
*/
int checkAcceptedUID(byte *uid, byte size) {
  for (int i = 0; i < 3; i++) {
    bool match = true;
    for (byte j = 0; j < size; j++) {
      if (uid[j] != acceptedUIDs[i][j]) {
        match = false;
        break;
      }
    }
    if (match) {
      return i;
    }
  }
  return -1;
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Requests URL via HTTP
* @param Send_Data_URL the URL to request
* @retval none
*/
void sendData(String Send_Data_URL){
  Serial.println("Send data to Google Spreadsheet...");
  Serial.print("URL : ");
  Serial.println(Send_Data_URL);

  // Writing data to Google Sheets
  HTTPClient http;
    
  // HTTP GET Request
  http.begin(Send_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  Serial.print("HTTP Status Code : ");
  Serial.println(httpCode);

  // Response from Google Sheets
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Response: " + payload);
  }

  http.end();
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Reads the RFID Card
* @param none
* @retval none
*/
void ReadCard(){
  //Silence Buzzer
  noTone(BUZZER);

  //Checks if there is a Card Oresent
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("Card UID:");
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
      }
      Serial.println();

      // Check if the UID is accepted
      int pos = checkAcceptedUID(mfrc522.uid.uidByte, mfrc522.uid.size);
      if (pos != -1) {
        
        //Grant Access
        Serial.println("Access granted");
        digitalWrite(LOCK, HIGH);
        tone(BUZZER, 131,5000);
        noTone(BUZZER);
        digitalWrite(LOCK, LOW);
        delay(1000);
        
        // Modify URL and send ID
        String Send_URL = WEB_APP_URL + "?uid=" + registeredIDs[pos];
        sendData(Send_URL);

      } else {
        //Deny access
        Serial.println("Access denied");
      }
      // Ends reading
      mfrc522.PICC_HaltA();
    }
  }
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Function to subscribe to ubidots and act phisically
* @param none
* @retval none
*/
void ubiloop(){
  if (!ubidots.connected()) {
    ubidots.reconnect();
    ubidots.subscribeLastValue("cleanroom","lights");
  }
  ubidots.loop();
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Loop function for ESP32
* @param none
* @retval none
*/
void loop() {
  ReadCard();
  ubiloop();
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
