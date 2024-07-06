/*
 *  @file   LockDoor.h
 *  @brief  Main file with Task to access and control the cleanroom in UPAEP using RFID and mobile app
 *  @author Iván Ortiz De Lara
*/

/*--Libraries------------------------------------------------------------------------------------------------------------------------------------*/
#include <MFRC522.h>
#include <SPI.h>

/*--PIN Defines----------------------------------------------------------------------------------------------------------------------------------*/
#define LOCK          13    // DoorLock PIN
#define BUZZER        4     // Buzzer PIN

/*--Acceptance IDs-------------------------------------------------------------------------------------------------------------------------------*/
extern byte acceptedUIDs[MAX_USR][4];
extern String registeredIDs[MAX_USR];
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
void GrantAccess() {
  Serial.println("Access granted");
  digitalWrite(LOCK, HIGH);
  tone(BUZZER, 131, 5000);
  delay(5000);
  noTone(BUZZER);
  digitalWrite(LOCK, LOW);
}
/*-----------------------------------------------------------------------------------------------------------------------------------------------*/