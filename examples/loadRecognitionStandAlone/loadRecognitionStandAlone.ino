/**
 * Minimal Voice Recognition for ESP32
 * Just loads records 0-6 and shows what's recognized
 */

#include "VoiceRecognitionV3.h"

VR myVR(&Serial2,17,16);  // Using Serial2

bool aux1=0,fan=0,lamp=0,clk=0;
uint8_t recordNum;
unsigned long t=0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Voice Recognition - ESP32");
  
  // Initialize VR module
  myVR.begin(9600);
  delay(1000);
  
  // Clear and load records 0-6
  myVR.clear();  
  myVR.load(uint8_t (0));
  myVR.load(uint8_t (1));
  
  Serial.println("Ready! Speak a command...");
  Serial.println("==========================");
}

void loop() {
  uint8_t buf[10];
  int result = myVR.recognize(buf, 50);
  
  if (result >0) {
    recordNum = buf[1];
    uint8_t position = buf[2];
    if(recordNum==0||recordNum==1){
      Serial.print("hey robot -> ");//prende ojos en verde
      aux1=1;
      t=millis();
      myVR.clear();      
      myVR.load(uint8_t (12));
      myVR.load(uint8_t (6));
      myVR.load(uint8_t (7));
    }    
  }
  if(aux1){
    switch(recordNum){
      default:
        if(millis()-t>15000){
          aux1=0;
          myVR.clear();  
          myVR.load(uint8_t (0));
          myVR.load(uint8_t (1));
          Serial.println("timeOut");//timeout
        }
        break;
      case 12://lampara
        aux1=0; 
        t=millis();      
        Serial.print("lampara -> ");
        myVR.clear();        
        myVR.load(uint8_t (2));
        myVR.load(uint8_t (3));
        myVR.load(uint8_t (4));
        break;
      case 6://ventilador
        aux1=0;
        t=millis();
        Serial.print("ventilador -> ");
        myVR.clear();
        myVR.load(uint8_t (5));
        myVR.load(uint8_t (4));
        break;
      case 7://reloj
        aux1=0;
        t=millis();
        Serial.print("reloj -> ");
        myVR.clear();        
        myVR.load(uint8_t (8));
        myVR.load(uint8_t (9));
        myVR.load(uint8_t (10));
        myVR.load(uint8_t (11));
        break;
    }
  }
  if(lamp){
    switch(recordNum){
      default:
        if(millis()-t>15000){
          aux1=0;
          lamp=0;
          myVR.clear();  
          myVR.load(uint8_t (0));
          myVR.load(uint8_t (1));
          Serial.println("timeOut");//timeout
        }
        break;
      case 2://luz amarilla 
        break;
      case 3://luz blanca
        break;
      case 4://apagao
        break;
    }
  }
}