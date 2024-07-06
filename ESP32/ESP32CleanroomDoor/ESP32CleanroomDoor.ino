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

/*--Master UIDs----------------------------------------------------------------------------------------------------------------------------------*/
byte MasterKeys[MAX_MSTR][4]{
{0x89, 0xEA, 0x95, 0x15},
{0x89, 0xEA, 0x95, 0x15},
{0x89, 0xEA, 0x95, 0x15}
};

/*--Task Handlers--------------------------------------------------------------------------------------------------------------------------------*/
TaskHandle_t REG_TASK;
TaskHandle_t BUTTON_TASK;
TaskHandle_t CARD_TASK;
TaskHandle_t OPENDOOR_TASK;

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
  WiFi.begin("upaep wifi", "");
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
  xTaskCreate(GrantAccess,"OPENDOOR_TASK",3000,NULL,4,&OPENDOOR_TASK);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Reads the RFID Card
* @retval none
*/
void ReadCard(void *parameter) {
  bool MasterTrigger = false;
  while (1){
    //Silence Buzzer
    noTone(BUZZER);

    //Checks if there is a Card Present
    String newUID;
    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
        Serial.print("Card UID:");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          newUID += String((mfrc522.uid.uidByte[i], HEX));
        }
        Serial.println();

        

        if (MasterTrigger == true){    //Mandar UID a GoogleSheets
          Serial.print("Modo pro");
          Serial.println("Envio de uid: " + newUID);
          String Send_URL = WEB_APP_URL + "?uid=-2&add=0" + newUID;     // Modify URL and send ID
          sendData(Send_URL);     // Sends ID to database
          MasterTrigger = false;

        } else{                       //Checkar registro
          for(int i=0; i<MAX_MSTR; i++){
            if(memcmp(mfrc522.uid.uidByte,MasterKeys[i], 4) == 0){
              MasterTrigger = true;
            }
            else{
              MasterTrigger = false;
            }
          }
          // Check if the UID is accepted
          int pos = checkAcceptedUID(mfrc522.uid.uidByte, mfrc522.uid.size);
          if (pos != -1) {
            xTaskNotify(OPENDOOR_TASK, NULL, eNoAction);                      //Grant Access
            
            String Send_URL = WEB_APP_URL + "?uid=" + registeredIDs[pos] + "&add=0";     // Modify URL and send ID
            sendData(Send_URL);     // Sends ID to database

          } else {
            Serial.println("Access denied");  //Deny access
          }
          mfrc522.PICC_HaltA(); // Ends reading
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
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

      xTaskNotify(OPENDOOR_TASK, NULL, eNoAction);        //Grant Access

      String Exit_URL = WEB_APP_URL + "?uid=" + "-1&add=0";     //Register Exit time
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
    for (int i=0; i<49; i++) {
      if(readData(i) == false){
        break;
      }
    }
    
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
