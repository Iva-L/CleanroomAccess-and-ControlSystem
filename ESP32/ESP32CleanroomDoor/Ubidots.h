/*
 *  @file   Ubidots.h
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Emmanuel Isaac García Sanabria
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <UbidotsEsp32Mqtt.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define EXTRACTOR 26 // Extractor activation PIN 
#define AIR_COOLER 12 // AC  activation PIN
#define LIGHTS 14 // Lights activation PIN
#define DEHUMIDIFERS 27 // Dehimidifers activation PIN

/*--WiFi-----------------------------------------------------------------------------------------------------------------------------------------*/
#define TOKEN "BBUS-wc3ibpEdsyoo7zP6C3Mncv0l81Qnb9"     //Ubidots TOKEN
#define WIFISSID "Totalplay-BCA8"  // SSID  
#define WIFIPASS "BCA83BAETKTSDEb3"  //Wifi Pass

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
Ubidots ubidots(TOKEN);

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Callback function for ubidots
* @param none
* @retval none
*/
void callback(char* topic, byte* payload, unsigned int length) {
    // Serial.print("Message arrived [");
    // Serial.print(topic);
    // Serial.print("] ");
  if( String(topic) == "/v2.0/devices/cleanroom/lights/lv"){ 
    // Serial.println();
    // Serial.print("Command lights: ");
    bool command1 = *payload - 48;
    // Serial.println(command1);
    digitalWrite(LIGHTS, command1);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/extractor/lv"){ 
    // Serial.println();
    // Serial.print("Command extractor: ");
    bool command2 = *payload - 48;
    // Serial.println(command2);
    digitalWrite(EXTRACTOR, command2);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/dehumidifer/lv"){ 
    // Serial.println();
    // Serial.print("Command dehimidifer: ");
    bool command3 = *payload - 48;
    // Serial.println(command3);
    digitalWrite(DEHUMIDIFERS, command3);
  }
  if( String(topic) == "/v2.0/devices/cleanroom/air_cooler/lv"){ 
    // Serial.println();
    // Serial.print("Command air cooler: ");
    bool command4 = *payload - 48;
    // Serial.println(command4);
    digitalWrite(AIR_COOLER, command4);
  }
  
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

void UbiConnect(){
  Serial.println(" Initializing Ubidots Connection...");
  ubidots.connectToWifi(WIFISSID, WIFIPASS);
  ubidots.setDebug(false);                        // Pass a true or false bool value to activate debug messages
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  Serial.println(" Initializing Ubidots Connection...");
  ubidots.subscribeLastValue("cleanroom","lights");
  ubidots.subscribeLastValue("cleanroom","extractor");
  ubidots.subscribeLastValue("cleanroom","dehumidifer");
  ubidots.subscribeLastValue("cleanroom","air_cooler");
  Serial.println("DONE");
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* Function to subscribe to ubidots and act phisically
* @param none
* @retval none
*/
void ubiloop(void *parameter){
  while (1){
    if (!ubidots.connected()) {
      ubidots.reconnect();
      ubidots.subscribeLastValue("cleanroom","lights");
      ubidots.subscribeLastValue("cleanroom","extractor");
      ubidots.subscribeLastValue("cleanroom","dehumidifer");
      ubidots.subscribeLastValue("cleanroom","air_cooler");
    }
    ubidots.loop();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/