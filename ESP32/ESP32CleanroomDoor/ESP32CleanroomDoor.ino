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

/*--Task Handlers--------------------------------------------------------------------------------------------------------------------------------*/
TaskHandle_t BUTTON_TASK;
TaskHandle_t UBILOOP_TASK;

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

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

  UbiConnect();

  xTaskCreate(UpdateReg,"REG_TASK",10000,NULL,8,&REG_TASK);
  xTaskCreate(CheckButton,"BUTTON_TASK",6000,NULL,1,&BUTTON_TASK);
  xTaskCreate(ReadCard,"CARD_TASK",10000,NULL,2,&CARD_TASK);
  xTaskCreate(GrantAccess,"OPENDOOR_TASK",3000,NULL,3,&OPENDOOR_TASK);
  xTaskCreate(ubiloop, "UBILOOP_TASK",30000,NULL,6,&UBILOOP_TASK);
}

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief  Main loop function
* @retval none
*/
void loop(){
  while(1){
  }
}

/*------------------------------------------------------------------------------------------------------------------------------------------------*/