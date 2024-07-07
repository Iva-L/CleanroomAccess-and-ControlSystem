/*
 *  @file   Ubidots.h
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Emmanuel Isaac García Sanabria
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <UbidotsEsp32Mqtt.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define EXTRACTOR       26   // Extractor activation PIN 
#define AIR_COOLER      12   // AC  activation PIN
#define LIGHTS          14   // Lights activation PIN
#define DEHUMIDIFERS    27   // Dehimidifers activation PIN

/*--WiFi and Ubidots-----------------------------------------------------------------------------------------------------------------------------*/
#define TOKEN "BBUS-wc3ibpEdsyoo7zP6C3Mncv0l81Qnb9"   //Ubidots TOKEN

/*--Variable Labels------------------------------------------------------------------------------------------------------------------------------*/
char *var_labels[] = {"lights", "extractor", "dehumidifer", "air_cooler"};

/*--Device Labels--------------------------------------------------------------------------------------------------------------------------------*/
#define DEVICE_LABEL "cleanroom"

/*--Last Values----------------------------------------------------------------------------------------------------------------------------------*/
#define DIMENSION_OF(x) (sizeof(x)/sizeof(x[0]))
float var_last_values[DIMENSION_OF(var_labels)] = {};

/*--Objects--------------------------------------------------------------------------------------------------------------------------------------*/
Ubidots ubidots(TOKEN);

/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Callback function for ubidots
* @param topic topic of callback
* @param payload message from ubidots
* @param length lenth of message
* @retval none
*/
void callback(char* topic, byte* payload, unsigned int length) {
  char *topic_cpy = strdup(topic);
  char *payload_str = (char *) malloc(length + sizeof(""));
  char *topic_item = strtok(topic_cpy, "/");
  char *label = NULL;
  float value = NAN;
  size_t index = DIMENSION_OF(var_labels);
  size_t i;

  memcpy(payload_str, payload, length);
  payload_str[length] = '\0';

  while ((NULL != topic_item) && (NULL == label)) {
    for (i = 0; i < DIMENSION_OF(var_labels); i++) {
      if (0 == strcmp(var_labels[i], topic_item)) {
        label = topic_item;
        value = atof(payload_str);
        index = i;
        break;
      }
    }
    topic_item = strtok(NULL, "/");
  }

  if (index < DIMENSION_OF(var_labels)) {
    var_last_values[index] = value;
    
    Serial.print(label);
    Serial.print(": ");
    Serial.println(var_last_values[index]);
  }

  free(topic_cpy);
  free(payload_str);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Suscribes to Last value from a variable
* @param labels all variable labels
* @param n_labels number of variable labels
* @retval none
*/
void subscribe_to_vars(char **labels, size_t n_labels) {
  for (size_t i = 0; i < n_labels; i++) {
    char *label = labels[i];
    ubidots.subscribeLastValue(DEVICE_LABEL, label);
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

void UbiConnect(){
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  subscribe_to_vars(var_labels, DIMENSION_OF(var_labels));
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Turns string into integer
* @param str the string variable
* @param h position 0 of string
* @retval the string as an integer
*/
constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Updates values of appliance PINS
* @retval none
*/
void updateVars(){
  for (int i = 0; i < DIMENSION_OF(var_labels); i++) {
    
    switch (str2int(var_labels[i])) {
      case str2int("lights"):
        digitalWrite(LIGHTS, var_last_values[i]);
        break;
      case str2int("extractor"):
        digitalWrite(EXTRACTOR, var_last_values[i]);
        break;
      case str2int("dehumidifer"):
        digitalWrite(DEHUMIDIFERS, var_last_values[i]);
        break;
      case str2int("air_cooler"):
        digitalWrite(AIR_COOLER, var_last_values[i]);
        break;
    }
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Checks connection to ubidots and suscribes to values
* @retval none
*/
void ubiloop(void *parameter){
  while (1){
    if (!ubidots.connected()) {
      ubidots.reconnect();
      subscribe_to_vars(var_labels, DIMENSION_OF(var_labels));
    }
    updateVars();
    ubidots.loop();
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/