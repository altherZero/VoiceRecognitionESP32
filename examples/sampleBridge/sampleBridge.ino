/**
  ******************************************************************************
  * @file    vr_sample_bridge_esp32.ino
  * @author  JiapengLi (Modified for ESP32)
  * @brief   This file provides a demonstration on 
              how to control VoiceRecognitionModule using serial commands
  ******************************************************************************
  * @note:
        Use this sample to try the command of VoiceRecognition Module.
    Eg:
       1. Enable Arduino Serial monitor "Send with newline" feature, Baud rate 115200.
       2. Input "01" to "check recognizer", 
       3. input "31" to "clear recognizer",
       4. input "30 00 02 04" to "load record 0, record 2, record 4"
  ******************************************************************************
  * @section  HISTORY
    
    2013/06/17    Initial version.
    2026/12/01    Modified for ESP32 with HardwareSerial
  */
  
#include "VoiceRecognitionV3.h"

/**        
  Connection for ESP32 (using Serial2 by default)
  ESP32 DevKit       VoiceRecognitionModule
  ============       ====================
  3.3V or 5V  -----> VCC
  GND         -----> GND
  GPIO16 (RX2) ----> TX
  GPIO17 (TX2) ----> RX
  
  For custom pins:
  VR myVR(&Serial2, RX_PIN, TX_PIN);
*/
VR myVR(&Serial2,17,16);    // Using Serial2 (GPIO16=RX, GPIO17=TX)

// Uncomment below for custom pins:
// VR myVR(&Serial2, 13, 14);  // RX=13, TX=14

/***************************************************************************/
// Command analyze part
#define CMD_BUF_LEN      64+1
uint8_t cmd[CMD_BUF_LEN];
uint8_t cmd_cnt;
uint8_t *paraAddr;

uint8_t buf[400];
uint8_t buflen[32];

void setup(void)
{
  // Initialize VR module at 9600 baud (default)
  myVR.begin(9600);
  
  /** initialize Serial monitor */
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("======================================================"));
  Serial.println(F("Elechouse Voice Recognition V3 Module - ESP32 Bridge"));
  Serial.println(F("======================================================"));
  Serial.println();
  Serial.println(F("Examples (send without quotes):"));
  Serial.println(F("  \"01\"       - Check recognizer status"));
  Serial.println(F("  \"31\"       - Clear recognizer"));
  Serial.println(F("  \"30 00 02\" - Load records 0 and 2"));
  Serial.println(F("  \"20 00\"    - Train record 0"));
  Serial.println(F("  \"00\"       - Check system settings"));
  Serial.println(F("  \"10\"       - Restore default settings"));
  Serial.println();
  Serial.println(F("Common Commands:"));
  Serial.println(F("  00: Check system settings"));
  Serial.println(F("  01: Check recognizer"));
  Serial.println(F("  02: Check record training status"));
  Serial.println(F("  03 xx: Check signature"));
  Serial.println(F("  10: Restore default settings"));
  Serial.println(F("  11 xx: Set baud rate"));
  Serial.println(F("  12 xx: Set IO mode"));
  Serial.println(F("  13 xx: Set pulse width"));
  Serial.println(F("  14: Reset IO"));
  Serial.println(F("  15: Set auto load"));
  Serial.println(F("  20 xx [yy]: Train record"));
  Serial.println(F("  21 xx: Train with signature"));
  Serial.println(F("  22 xx: Set signature"));
  Serial.println(F("  30 xx [yy]: Load records"));
  Serial.println(F("  31: Clear recognizer"));
  Serial.println(F("  32 xx yy: Group commands"));
  Serial.println(F("  EE: Test commands"));
  Serial.println(F("======================================================"));
  Serial.println();
}

void loop(void)
{
  int len, i, ret, index;
  
  /** receive Serial command */
  len = receiveCMD();
  if (len > 0) {
    printSeperator();  
    if (!checkCMD(len)) {    
      ret = convertCMD(buf, len);
      if (ret > 0) {
        Serial.print("> Sending: ");
        myVR.writehex(buf, ret);
        myVR.send_pkt(buf, ret);
        Serial.println();
      } else {
        /** received command is invalid */
        Serial.write(cmd, len);
        Serial.println(F(" - Invalid hex format"));
      }    
    } else {
      /** received command is invalid */
      Serial.print("> ");
      Serial.write(cmd, len); 
      Serial.println(F(" - Invalid characters"));
    }
  }
  
  /** receive all packets at once */
  len = 0;
  index = 0;
  memset(buflen, 0, sizeof(buflen));
  
  while (1) {
    ret = myVR.receive_pkt(buf + len, 50);
    if (ret > 0) {
      len += ret;
      buflen[index] = ret;
      index++;
      if (index >= 32) break; // Prevent overflow
    } else {
      break;
    }
  }
  
  if (index > 0) {
    len = 0;
    for (i = 0; i < index; i++) {
      Serial.print("< Response: ");
      myVR.writehex(buf + len, buflen[i]);
      Serial.println();
      
      // Try to interpret common responses
      interpretResponse(buf + len, buflen[i]);
      
      len += buflen[i];
    }
  }
}

/**
  @brief   Interpret common responses
  @param   buf  -> response buffer
           len  -> response length
*/
void interpretResponse(uint8_t *buf, int len) {
  if (len < 4) return;
  
  switch (buf[2]) {
    case FRAME_CMD_VR:
      Serial.print("  Voice recognized! Group: 0x");
      Serial.print(buf[3], HEX);
      Serial.print(", Record: ");
      Serial.print(buf[4]);
      Serial.print(", Position: ");
      Serial.println(buf[5]);
      if (buf[6] > 0) {
        Serial.print("  Signature: ");
        for (int i = 0; i < buf[6]; i++) {
          Serial.print((char)buf[7 + i]);
        }
        Serial.println();
      }
      break;
      
    case FRAME_CMD_CHECK_BSR:
      Serial.print("  Recognizer - Valid: ");
      Serial.print(buf[3]);
      Serial.print("/");
      Serial.println(buf[11]);
      Serial.print("  Loaded: ");
      for (int i = 0; i < 7; i++) {
        if (buf[4 + i] != 0xFF) {
          Serial.print(buf[4 + i]);
          Serial.print(" ");
        }
      }
      Serial.println();
      break;
      
    case FRAME_CMD_CHECK_SYSTEM:
      Serial.print("  System - Baud: ");
      Serial.print(buf[4]);
      Serial.print(", IO Mode: ");
      Serial.print(buf[5]);
      Serial.print(", Pulse Width: ");
      Serial.print(buf[6]);
      Serial.print(", Auto Load: ");
      Serial.println(buf[7] == 0xFF ? "Disabled" : "Enabled");
      break;
      
    case FRAME_CMD_PROMPT:
      Serial.print("  Prompt: ");
      for (int i = 3; i < len - 1; i++) {
        Serial.print((char)buf[i]);
      }
      Serial.println();
      break;
      
    case FRAME_CMD_TRAIN:
      Serial.print("  Train result - Success: ");
      Serial.println(buf[3]);
      for (int i = 0; i < (len - 4) / 2; i++) {
        Serial.print("    Record ");
        Serial.print(buf[4 + i * 2]);
        Serial.print(": ");
        switch (buf[5 + i * 2]) {
          case 0x00: Serial.println("Trained"); break;
          case 0xFE: Serial.println("Timeout"); break;
          case 0xFF: Serial.println("Out of range"); break;
          default: Serial.println("Unknown"); break;
        }
      }
      break;
      
    case FRAME_CMD_LOAD:
      Serial.print("  Load result - Success: ");
      Serial.println(buf[3]);
      for (int i = 0; i < (len - 4) / 2; i++) {
        Serial.print("    Record ");
        Serial.print(buf[4 + i * 2]);
        Serial.print(": ");
        switch (buf[5 + i * 2]) {
          case 0x00: Serial.println("Loaded"); break;
          case 0xFC: Serial.println("Already loaded"); break;
          case 0xFD: Serial.println("Recognizer full"); break;
          case 0xFE: Serial.println("Untrained"); break;
          case 0xFF: Serial.println("Out of range"); break;
          default: Serial.println("Unknown"); break;
        }
      }
      break;
  }
}

/**
  @brief   receive command from Serial.
  @param   NONE.
  @retval  command length, if no command receive return -1.
*/
int receiveCMD()
{
  int ret;
  int len;
  unsigned long start_millis;
  
  if (!Serial.available()) {
    return -1;
  }
  
  start_millis = millis();
  
  while (1) {
    ret = Serial.read();
    if (ret > 0) {
      start_millis = millis();
      cmd[cmd_cnt] = ret;
      
      if (cmd[cmd_cnt] == '\n' || cmd[cmd_cnt] == '\r') {
        len = cmd_cnt;
        cmd_cnt = 0;
        if (len > 0) {
          return len;
        }
      }
      
      cmd_cnt++;
      if (cmd_cnt == CMD_BUF_LEN) {
        cmd_cnt = 0;
        return -1;
      }
    }
    
    if (millis() - start_millis > 100) {
      if (cmd_cnt > 0) {
        len = cmd_cnt;
        cmd_cnt = 0;
        return len;
      }
      cmd_cnt = 0;
      return -1;
    }
  }
}

/**
  @brief   Check command format.
  @param   len  -->  command length
  @retval  0  -->  command is valid
          -1  -->  command is invalid
*/
int checkCMD(int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (cmd[i] >= '0' && cmd[i] <= '9') {
      // Valid
    } else if (cmd[i] >= 'a' && cmd[i] <= 'f') {
      // Valid
    } else if (cmd[i] >= 'A' && cmd[i] <= 'Z') {
      // Valid
    } else if (cmd[i] == '\t' || cmd[i] == ' ' || cmd[i] == '\r' || cmd[i] == '\n') {
      // Valid whitespace
    } else {
      return -1;
    }
  }
  return 0;
}

/**
  @brief   Check the number of parameters in the command
  @param   len  -->  command length
  @retval  number of parameters
*/
int checkParaNum(int len)
{
  int cnt = 0, i;
  for (i = 0; i < len;) {
    if (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
      cnt++;
      while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
        i++;
      }
    }
    i++;
  }
  return cnt;
}

/**
  @brief   Find the specified parameter.
  @param   len       -->  command length
           paraIndex -->  parameter index
           addr      -->  return value. position of the parameter
  @retval  length of specified parameter
*/
int findPara(int len, int paraIndex, uint8_t **addr)
{
  int cnt = 0, i, paraLen;
  uint8_t dt;
  
  for (i = 0; i < len;) {
    dt = cmd[i];
    if (dt != '\t' && dt != ' ') {
      cnt++;
      if (paraIndex == cnt) {
        *addr = cmd + i;
        paraLen = 0;
        while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
          i++;
          paraLen++;
        }
        return paraLen;
      } else {
        while (cmd[i] != '\t' && cmd[i] != ' ' && cmd[i] != '\r' && cmd[i] != '\n') {
          i++;
        }
      }
    } else {
      i++;
    }
  }
  return -1;
}

/**
  @brief   Convert hex string to integer
  @param   str  -->  hex string
  @retval  converted value
*/
uint32_t atoh(uint8_t *str)
{
  int i, ret;
  i = 0, ret = 0;
  
  while ((str[i] >= '0' && str[i] <= '9') || 
         (str[i] >= 'A' && str[i] <= 'F') || 
         (str[i] >= 'a' && str[i] <= 'f')) {
    
    if (i == 8) {
      ret = 0;
      break;
    }
    
    ret <<= 4;
    
    if (str[i] >= '0' && str[i] <= '9') {
      ret += (str[i] - '0');
    } else if (str[i] >= 'A' && str[i] <= 'F') {
      ret += (str[i] - 'A' + 0x0A);
    } else if (str[i] >= 'a' && str[i] <= 'f') {
      ret += (str[i] - 'a' + 0x0A);
    }
    
    i++;
  }
  
  return ret;
}

/**
  @brief   Convert command string to byte array
  @param   des  -->  destination buffer
           len  -->  command length
  @retval  number of bytes converted, -1 if error
*/
int convertCMD(uint8_t *des, int len)
{
  int i, paraNum, paraLen;
  paraNum = checkParaNum(len);
  
  for (i = 0; i < paraNum; i++) {
    paraLen = findPara(len, i + 1, &paraAddr);
    
    if (paraLen > 2) {
      return -1;
    }
    
    des[i] = atoh(paraAddr);
    
    if (des[i] == 0) {
      if (*paraAddr != '0') {
        return -1;
      }
      if (paraLen == 2 && *(paraAddr + 1) != '0') {
        return -1;
      }
    }
  }
  
  return paraNum;
}

/**
  @brief   Print separator line
*/
void printSeperator()
{
  for (int i = 0; i < 60; i++) {
    Serial.write('-');
  }
  Serial.println();
}

/**
  @brief   Interactive help menu (optional - call from setup or loop)
*/
void printHelpMenu() {
  printSeperator();
  Serial.println(F("VOICE RECOGNITION V3 - COMMAND REFERENCE"));
  printSeperator();
  Serial.println(F("SYSTEM COMMANDS:"));
  Serial.println(F("  00        - Check system settings"));
  Serial.println(F("  01        - Check recognizer status"));
  Serial.println(F("  02        - Check all record training status"));
  Serial.println(F("  02 XX     - Check specific record status"));
  Serial.println(F("  03 XX     - Check record signature"));
  Serial.println(F("  10        - Restore default settings"));
  Serial.println(F("  11 XX     - Set baud rate (0=9600,1=2400,2=4800,3=9600,4=19200,5=38400)"));
  Serial.println(F("  12 XX     - Set IO mode (0=PULSE,1=TOGGLE,2=SET,3=CLEAR)"));
  Serial.println(F("  13 XX     - Set pulse width (0-15)"));
  Serial.println(F("  14        - Reset IO"));
  Serial.println(F("  14 XX     - Reset specific IO"));
  Serial.println(F("  15        - Disable auto load"));
  Serial.println(F("  15 XX YY  - Set auto load records"));
  
  Serial.println(F("\nTRAINING COMMANDS:"));
  Serial.println(F("  20 XX        - Train record XX"));
  Serial.println(F("  20 XX YY ZZ  - Train multiple records"));
  Serial.println(F("  21 XX        - Train record XX with signature"));
  Serial.println(F("  22 XX        - Set/Delete signature for record XX"));
  
  Serial.println(F("\nRECOGNITION COMMANDS:"));
  Serial.println(F("  30 XX        - Load record XX"));
  Serial.println(F("  30 XX YY ZZ  - Load multiple records"));
  Serial.println(F("  31           - Clear recognizer"));
  
  Serial.println(F("\nGROUP COMMANDS:"));
  Serial.println(F("  32 00 XX     - Set group control (0=disable,1=user,2=system)"));
  Serial.println(F("  32 01        - Check group control"));
  Serial.println(F("  32 02 XX     - Load system group XX"));
  Serial.println(F("  32 03 XX     - Load user group XX"));
  
  Serial.println(F("\nTEST COMMANDS:"));
  Serial.println(F("  EE 00        - Write test pattern"));
  Serial.println(F("  EE 01        - Read test pattern"));
  printSeperator();
}