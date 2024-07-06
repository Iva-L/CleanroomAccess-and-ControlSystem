/*
 *  @brief  This program was made for the acces door of the clean room in UPAEP using RFID ans an IoT app
 *  @author Emmanuel Isaac García Sanabria, Iván Ortiz De Lara, Alessia Sanchez Amezcua
*/
/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <SPI.h>
#include <MFRC522.h>
#include "WiFi.h"
#include <HTTPClient.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN 22       // MFRC552 Reset PIN
#define SS_PIN 21        // MFRC552 SS SDA PIN
#define LOCK 13          // DoorLock PIN
#define BUZZER 4         // Buzzer PIN
#define BUTTON 2         //Exit button PIN
#define EXTRACTOR 26     // Extractor activation PIN
#define AIR_COOLER 12    // AC  activation PIN
#define LIGHTS 14        // Lights activation PIN
#define DEHUMIDIFERS 27  // Dehimidifers activation PIN

/*--Constant Defines-----------------------------------------------------------------------------------------------------------------------------*/
#define MAX_USR 50

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN);

/*--GoogleSheets Info----------------------------------------------------------------------------------------------------------------------------*/
const String WEB_APP_URL = "https://script.google.com/macros/s/AKfycbxiYW4zfcS4e_RgLtCBWFyxQM63XIDS3SUSOO3bRGPIITaGurdiBGJlOJOEfPbOzTnD/exec";
const String SHEET_NAME = "KeyLog";

/*--Acceptance IDs-------------------------------------------------------------------------------------------------------------------------------*/

byte acceptedUIDs[MAX_USR][4];
String registeredIDs[MAX_USR];

/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Setup function for ESP32
* @param none
* @retval none
*/
void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(BUTTON, INPUT);

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
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Checks if the UID is acceptable
* @param uid the card UID
* @param size the size of UID
* @retval the accepted UID
*/
int checkAcceptedUID(byte *uid, byte size) {
  for (int i = 0; i < MAX_USR; i++) {
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
* @brief Reads Data from Google Sheets
* @retval none
*/
void readData(int n) {
  n += 1;
  HTTPClient http;
  String Read_Data_URL = WEB_APP_URL + "?uid=" + n;

  // HTTP GET Request.
  http.begin(Read_Data_URL.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Gets the HTTP status code.
  int httpCode = http.GET();

  // Getting response from google sheet.
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload : " + payload);
  }

  http.end();

  acceptedUIDs[n][0] = strtoul(payload.substring(0,2).c_str(), NULL, 16);
  acceptedUIDs[n][1] = strtoul(payload.substring(3,5).c_str(), NULL, 16);
  acceptedUIDs[n][2] = strtoul(payload.substring(6,8).c_str(), NULL, 16);
  acceptedUIDs[n][3] = strtoul(payload.substring(9,11).c_str(), NULL, 16);

  registeredIDs[n] = payload.substring(12,payload.length());
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Requests URL via HTTP
* @param Send_Data_URL the URL to request
* @retval none
*/
void sendData(String Send_Data_URL) {
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
* @brief Reads the RFID Card
* @param none
* @retval none
*/
void ReadCard() {
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

        GrantAccess();

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
* @brief Grants Access and opens the door
* @param none
* @retval none
*/
void GrantAccess() {
  Serial.println("Access granted");
  digitalWrite(LOCK, HIGH);
  tone(BUZZER, 131, 5000);
  noTone(BUZZER);
  digitalWrite(LOCK, LOW);
  delay(1000);
}

/**
* @brief Loop function for ESP32
* @param none
* @retval none
*/
void loop() {
  // Checks accepted Cards index starts from 0 to 49

  readData(2);

  //Reads Card if aviable
  ReadCard();

  //Open door if button is pressed
  if (digitalRead(BUTTON) == HIGH) {

    //Grant Access
    GrantAccess();

    //Register Exit time
    String Exit_URL = WEB_APP_URL + "?uid=" + "-1";
    sendData(Exit_URL);
  }
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
