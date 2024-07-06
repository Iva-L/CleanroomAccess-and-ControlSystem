/*
 *  @file   GSheets.h
 *  @brief  Header file for Google Sheets Client conection for sending and reciving data
 *  @author Alessia Sanchez Amezcua
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <HTTPClient.h>

/*--GoogleSheets Info----------------------------------------------------------------------------------------------------------------------------*/
const String WEB_APP_URL = "https://script.google.com/macros/s/AKfycbxiYW4zfcS4e_RgLtCBWFyxQM63XIDS3SUSOO3bRGPIITaGurdiBGJlOJOEfPbOzTnD/exec";
const String SHEET_NAME = "KeyLog";

/*--Constant Defines-----------------------------------------------------------------------------------------------------------------------------*/
#define MAX_USR 50    //Max users register

/*--Acceptance IDs-------------------------------------------------------------------------------------------------------------------------------*/
byte acceptedUIDs[MAX_USR][4];
String registeredIDs[MAX_USR];
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

/**
* @brief Reads Data from Google Sheets
* @retval none
*/
bool readData(int n) {
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

  if(payload[0] == ','){
    return false;
  } else {
    acceptedUIDs[n][0] = strtoul(payload.substring(0,2).c_str(), NULL, 16);
    acceptedUIDs[n][1] = strtoul(payload.substring(3,5).c_str(), NULL, 16);
    acceptedUIDs[n][2] = strtoul(payload.substring(6,8).c_str(), NULL, 16);
    acceptedUIDs[n][3] = strtoul(payload.substring(9,11).c_str(), NULL, 16);

    registeredIDs[n] = payload.substring(12,payload.length());
    return true;
  }
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/

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
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/