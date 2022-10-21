#include <Arduino.h>

#include "FS.h"
#include "SD_MMC.h"

#define BUFSIZE 20

char buffer[BUFSIZE];
uint16_t bufIndex;

void initMicroSDCard() {
  // Start the MicroSD card
 
  Serial.println("Mounting MicroSD Card");
  if (!SD_MMC.begin()) {
    Serial.println("MicroSD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No MicroSD Card found");
    return;
  }
 
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.print("< to file: ");
    Serial.println(path);
    Serial.print("Appending Message >");
    Serial.print(message);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void write2SD(char c){
  buffer[bufIndex++] = c;
  if(bufIndex >= BUFSIZE - 2){
    buffer[bufIndex] = 0;
    appendFile(SD_MMC, "/test.txt", buffer);
    bufIndex = 0;
  }
}

void setup(){
  Serial.begin(115200);
  initMicroSDCard();
  writeFile(SD_MMC, "/test.txt", " hum1   hum2   temp1   temp2\r");
  bufIndex = 0;
}

void loop(){  
  while (Serial.available()){
    //Serial.print(".");
    char c = Serial.read();
    write2SD(c);
    Serial.print(c);
  }
  //write2SD('\r');
  delay(50);
}