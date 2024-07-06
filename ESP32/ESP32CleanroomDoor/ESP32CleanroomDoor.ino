/*
 *  @file   ESP32CleanroomDoor.ino
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Emmanuel Isaac García Sanabria, Iván Ortiz De Lara, Alessia Sanchez Amezcua
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include "WiFi.h"           // WiFi Library for ESP32
#include "GSheets.h"        // Google Sheets custom header
#include "LockDoor.h"       // Lockdoor mechanism custom header
#include <UbidotsEsp32Mqtt.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN       22    // MFRC552 Reset PIN
#define SS_PIN        21    // MFRC552 SDA PIN
#define BUTTON        2     // Exit button PIN
#define EXTRACTOR     26    // Extractor activation PIN
#define AIR_COOLER    12    // AC activation PIN
#define LIGHTS        14    // Light activation PIN
#define DEHUMIDIFERS  27    // Dehumidifiers activation PIN

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define RST_PIN 22  // MFRC552 Reset PIN
#define SS_PIN  21  // MFRC552 SS SDA PIN
#define BUTTON  2  //Exit button PIN
#define EXTRACTOR 26 // Extractor activation PIN 
#define AIR_COOLER 12 // AC  activation PIN
#define LIGHTS 14 // Lights activation PIN
#define DEHUMIDIFERS 27 // Dehimidifers activation PIN

/*--WiFi and ubidots--------------------------------------------------------------------------------------------------------------------------------------*/
#define TOKEN "BBUS-wc3ibpEdsyoo7zP6C3Mncv0l81Qnb9"     //Ubidots TOKEN
#define WIFISSID "Totalplay-BCA8"  // SSID  
#define WIFIPASS "BCA83BAETKTSDEb3"  //Wifi Pass

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
MFRC522 mfrc522(SS_PIN, RST_PIN); 
Ubidots ubidots(TOKEN);

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
* Callback function for ubidots
* @param none
* @retval none
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if( String(topic) == "/v2.0/devices/cleanroom/lights/lv"){ 
    Serial.println();
    Serial.print("Command lights: ");
    bool command1 = *payload - 48;
    Serial.println(command1);
    digitalWrite(LIGHTS, command1);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/extractor/lv"){ 
    Serial.println();
    Serial.print("Command extractor: ");
    bool command2 = *payload - 48;
    Serial.println(command2);
    digitalWrite(EXTRACTOR, command2);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/dehumidifer/lv"){ 
    Serial.println();
    Serial.print("Command dehimidifer: ");
    bool command3 = *payload - 48;
    Serial.println(command3);
    digitalWrite(DEHUMIDIFERS, command3);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/air_cooler/lv"){ 
    Serial.println();
    Serial.print("Command air cooler: ");
    bool command4 = *payload - 48;
    Serial.println(command4);
    digitalWrite(AIR_COOLER, command4);
  }
  
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Setup function for ESP32
* @retval none
*/
void setup() {
  pinMode(LOCK, OUTPUT);
  pinMode(BUTTON, INPUT);

  pinMode(EXTRACTOR, OUTPUT);
  pinMode(AIR_COOLER, OUTPUT);
  pinMode(LIGHTS, OUTPUT);
  pinMode(DEHUMIDIFERS, OUTPUT);
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFISSID,WIFIPASS);
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
  ubidots.subscribeLastValue("cleanroom","extractor");
  ubidots.subscribeLastValue("cleanroom","dehumidifer");
  ubidots.subscribeLastValue("cleanroom","air_cooler");
  Serial.println("DONE");

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

/**
* Function to subscribe to ubidots and act phisically
* @param none
* @retval none
*/
void ubiloop(){
  if (!ubidots.connected()) {
    ubidots.reconnect();
    ubidots.subscribeLastValue("cleanroom","lights");
    ubidots.subscribeLastValue("cleanroom","extractor");
    ubidots.subscribeLastValue("cleanroom","dehumidifer");
    ubidots.subscribeLastValue("cleanroom","air_cooler");
  }
  ubidots.loop();
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/
