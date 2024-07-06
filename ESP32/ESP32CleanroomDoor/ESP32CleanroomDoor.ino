/*
 *  @file   ESP32CleanroomDoor.ino
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Emmanuel Isaac García Sanabria, Iván Ortiz De Lara, Alessia Sanchez Amezcua
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include "WiFi.h"           // WiFi Library for ESP32
#include "GSheets.h"        // Google Sheets custom header
#include "LockDoor.h"       // Lockdoor mechanism custom header

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN       22    // MFRC552 Reset PIN
#define SS_PIN        21    // MFRC552 SDA PIN
#define BUTTON        2     // Exit button PIN
#define EXTRACTOR     26    // Extractor activation PIN
#define AIR_COOLER    12    // AC activation PIN
#define LIGHTS        14    // Light activation PIN
#define DEHUMIDIFERS  27    // Dehumidifiers activation PIN

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN);

/*--Acceptance IDs-------------------------------------------------------------------------------------------------------------------------------*/
extern byte acceptedUIDs[MAX_USR][4];
extern String registeredIDs[MAX_USR];

/*--Task Handlers--------------------------------------------------------------------------------------------------------------------------------*/
TaskHandle_t REG_TASK;
TaskHandle_t BUTTON_TASK;
TaskHandle_t CARD_TASK;
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Setup function for ESP32
* @retval none
*/
void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(BUTTON, INPUT);

  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi");
  WiFi.begin("INFINITUMB9A9", "S2GQCG3fSb");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected.");
  Serial.println("------------");

  xTaskCreate(UpdateReg,"REG_TASK",10000,NULL,8,&REG_TASK);
  xTaskCreate(CheckButton,"BUTTON_TASK",10000,NULL,2,&BUTTON_TASK);
  xTaskCreate(ReadCard,"CARD_TASK",10000,NULL,2,&CARD_TASK);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Reads the RFID Card
* @retval none
*/
void ReadCard(void *parameter) {
  while (1){
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
          String Send_URL = WEB_APP_URL + "?uid=" + registeredIDs[pos];     // Modify URL and send ID
          sendData(Send_URL);     // Sends ID to database

        } else {
          Serial.println("Access denied");  //Deny access
        }
        mfrc522.PICC_HaltA(); // Ends reading
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Loop function for ESP32
* @retval none
*/
void CheckButton(void *parameter) {
  while(1){
    //Open door if button is pressed
    if (digitalRead(BUTTON) == HIGH) {

      GrantAccess();      //Grant Access

      String Exit_URL = WEB_APP_URL + "?uid=" + "-1";     //Register Exit time
      sendData(Exit_URL);
    }
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Updates the register of acceptable UIDs
* @retval none
*/
void UpdateReg(void *parameter){
  while(1){
    // Checks accepted Cards index starts from 0 to 49
    readData(0);
    vTaskDelay(pdMS_TO_TICKS(300000));
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Main loop function
* @retval none
*/
void loop(){
  Serial.printf("%s running on core %d (priorite %d)\n", "loop()", xPortGetCoreID(), uxTaskPriorityGet(NULL));
  while(1){
  }
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
