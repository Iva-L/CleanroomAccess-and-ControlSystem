/*
 *  @file   LockDoor.h
 *  @brief  Header file for opening the door using RFID
 *  @author Iván Ortiz De Lara
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <MFRC522.h>
#include <SPI.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN 22  // MFRC552 Reset PIN
#define SS_PIN  21  // MFRC552 SS SDA PIN
#define BUTTON  2   //Exit button PINs
#define LOCK    13  // DoorLock PIN
#define BUZZER  4   // Buzzer PIN

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
TaskHandle_t CARD_TASK;
TaskHandle_t OPENDOOR_TASK;

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

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
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Grants Access and opens the door
* @retval none
*/
void GrantAccess(void *parameter) {
  while (1){
    xTaskNotifyWait(0, 0x00, NULL, portMAX_DELAY);

    Serial.println("Access granted");
    digitalWrite(LOCK, HIGH);
    tone(BUZZER, 131, 3000);
    vTaskDelay(pdMS_TO_TICKS(3000));
    noTone(BUZZER);
    digitalWrite(LOCK, LOW);
  }
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
    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
        Serial.print("Card UID:");
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
        }

        if (MasterTrigger == true){    //Mandar UID a GoogleSheets
          MasterTrigger = false;

          String newUID = "";
          for (int i = 0; i < 4; i++) {
            newUID += String(mfrc522.uid.uidByte[i], HEX);
          }

          Serial.println("Envio de uid: " + newUID);
          String Send_URL = WEB_APP_URL + "?uid=-2&add=" + newUID;     // Modify URL and send ID
          sendData(Send_URL);     // Sends ID to database
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
          vTaskDelay(pdMS_TO_TICKS(1000));
        }
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

      xTaskNotify(OPENDOOR_TASK, NULL, eNoAction);        //Grant Access

      String Exit_URL = WEB_APP_URL + "?uid=" + "-1&add=0";     //Register Exit time
      sendData(Exit_URL);
    }
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/