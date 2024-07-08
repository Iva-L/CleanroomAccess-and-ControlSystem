/*
 *  @file   ESP32CleanroomDoor.ino
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Emmanuel Isaac García Sanabria, Iván Ortiz De Lara, Alessia Sanchez Amezcua
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include "WiFi.h"           // WiFi Library for ESP32
#include "GSheets.h"        // Google Sheets custom header
#include "LockDoor.h"       // Lockdoor mechanism custom header
#include "Ubidots.h"        // Ubidots custom header

/*--WiFi-----------------------------------------------------------------------------------------------------------------------------------------*/
#define WIFISSID "upaep wifi"    // Wifi SSID  
#define WIFIPASS ""       // Wifi Password

/*--Task Handlers--------------------------------------------------------------------------------------------------------------------------------*/
TaskHandle_t BUTTON_TASK;
TaskHandle_t UBILOOP_TASK;

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Setup function for ESP32
* @retval none
*/
void setup() {
  // Door PINS
  pinMode(LOCK, OUTPUT);
  pinMode(BUTTON, INPUT);

  //Appliance PINS
  pinMode(EXTRACTOR, OUTPUT);
  pinMode(AIR_COOLER, OUTPUT);
  pinMode(LIGHTS, OUTPUT);
  pinMode(DEHUMIDIFERS, OUTPUT);

  //Initialize serial communication
  Serial.begin(9600);

  //Initialize SPI communication
  SPI.begin();

  //Initialize MFRC522 module
  mfrc522.PCD_Init();

  //Initialize WiFi Connection
  WiFi.mode(WIFI_STA);                      //WiFI STA mode
  WiFi.begin(WIFISSID,WIFIPASS);            //Begin WiFi connection
  while (WiFi.status() != WL_CONNECTED) {   //Check if WiFi is connected
    delay(250);
    Serial.print(".");
  }

  //Initialize Ubidots connection
  UbiConnect();

  //Tasks Creation
  xTaskCreatePinnedToCore(UpdateReg,"REG_TASK",10000,NULL,8,&REG_TASK,0);
  xTaskCreatePinnedToCore(CheckButton,"BUTTON_TASK",6000,NULL,1,&BUTTON_TASK,1);
  xTaskCreatePinnedToCore(ReadCard,"CARD_TASK",10000,NULL,2,&CARD_TASK,1);
  xTaskCreatePinnedToCore(GrantAccess,"OPENDOOR_TASK",3000,NULL,3,&OPENDOOR_TASK,1);
  xTaskCreatePinnedToCore(ubiloop, "UBILOOP_TASK",30000,NULL,6,&UBILOOP_TASK,0);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Main loop function
* @retval none
*/
void loop(){
  while(1){
    delay(1000000);
  }
}
/*------------------------------------------------------------------------------------------------------------------------------------------------*/