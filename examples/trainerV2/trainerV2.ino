/**
 ******************************************************************************
 * @file    vr_sample_train_esp32.ino
 * @author  JiapengLi (Modified for ESP32)
 * @brief   This file provides a demonstration on 
 * how to train VoiceRecognitionModule to record your voice
 * MODIFIED: Each record is trained 5 times for better accuracy
 ******************************************************************************
 * @note:
 * Use serial command to control VoiceRecognitionModule. '
 * All commands are case insensitive. Default serial baud rate 115200.
 * 
 * COMMAND        FORMAT                        EXAMPLE                    Comment
 * 
 * train          train (r0) (r1)...            train 0 2 45               Train records (5 times each)
 * load           load (r0) (r1) ...            load 0 51 2 3              Load records
 * clear          clear                         clear                      remove all records in Recognizer
 * record         record / record (r0) (r1)...  record / record 0 79       Check record train status
 * vr             vr                            vr                         Check recognizer status
 * getsig         getsig (r)                    getsig 0                   Get signature of record (r)
 * sigtrain       sigtrain (r) (sig)            sigtrain 0 ZERO            Train one record(r) with signature(sig)
 * settings       settings                      settings                   Check current system settings
 ******************************************************************************
 * @section  HISTORY
 * 
 * 2013/06/13    Initial version.
 * 2023/12/01    Modified for ESP32 with HardwareSerial
 * 2023/12/01    Modified to train each record 5 times
 */

#include "VoiceRecognitionV3.h"

/**        
 * Connection for ESP32 (using Serial2 by default)
 * ESP32 DevKit       VoiceRecognitionModule
 * ============       ====================
 * 3.3V or 5V  -----> VCC
 * GND         -----> GND
 * GPIO17 (RX2) ----> TX
 * GPIO16 (TX2) ----> RX
 * 
 * For custom pins:
 * VR myVR(&Serial2, RX_PIN, TX_PIN);
 */
VR myVR(&Serial2,17,16);    // Using Serial2 (GPIO17=RX, GPIO16=TX)

// Uncomment below for custom pins:
// VR myVR(&Serial2, 13, 14);  // RX=13, TX=14

/***************************************************************************/
/** declare print functions */
void printSeperator();
void printSignature(uint8_t *buf, int len);
void printVR(uint8_t *buf);
void printLoad(uint8_t *buf, uint8_t len);
void printTrain(uint8_t *buf, uint8_t len);
void printCheckRecognizer(uint8_t *buf);
void printUserGroup(uint8_t *buf, int len);
void printCheckRecord(uint8_t *buf, int num);
void printCheckRecordAll(uint8_t *buf, int num);
void printSigTrain(uint8_t *buf, uint8_t len);
void printSystemSettings(uint8_t *buf, int len);
void printHelp(void);

/***************************************************************************/
// command analyze part
#define CMD_BUF_LEN      64+1
#define CMD_NUM     10
typedef int (*cmd_function_t)(int, int);
uint8_t cmd[CMD_BUF_LEN];
uint8_t cmd_cnt;
uint8_t *paraAddr;
int receiveCMD();
int checkCMD(int len);
int checkParaNum(int len);
int findPara(int len, int paraNum, uint8_t **addr);
int compareCMD(uint8_t *para1 , uint8_t *para2, int len);

int cmdTrain(int len, int paraNum);
int cmdLoad(int len, int paraNum);
int cmdTest(int len, int paraNum);
int cmdVR(int len, int paraNum);
int cmdClear(int len, int paraNum);
int cmdRecord(int len, int paraNum);
int cmdSigTrain(int len, int paraNum);
int cmdGetSig(int len, int paraNum);
int cmdSettings(int len, int paraNum);
int cmdHelp(int len, int paraNum);

/** cmdList, cmdLen, cmdFunction has correspondence */
const char cmdList[CMD_NUM][10] = {  // command list table
  {"train"},
  {"load"}, 
  {"clear"},
  {"vr"},
  {"record"},
  {"sigtrain"},
  {"getsig"},
  {"settings"},
  {"test"},
  {"help"}
};

const char cmdLen[CMD_NUM] = {    // command length
  5,  //  {"train"},
  4,  //  {"load"}, 
  5,  //  {"clear"},
  2,  //  {"vr"},
  6,  //  {"record"},
  8,  //  {"sigtrain"},
  6,  //  {"getsig"},
  8,  //  {"settings"},
  4,  //  {"test"},
  4   //  {"help"}
};

cmd_function_t cmdFunction[CMD_NUM] = {      // command handle function(function pointer table)
  cmdTrain,
  cmdLoad,
  cmdClear,
  cmdVR,
  cmdRecord,
  cmdSigTrain,
  cmdGetSig,
  cmdSettings,
  cmdTest,
  cmdHelp,
};

/***************************************************************************/
/** temporary data */
uint8_t buf[255];
uint8_t records[7]; // save record

// NEW: Track training attempts
int trainingAttempts[50] = {0}; // Track how many times each record was trained
const int TRAINING_REPEATS = 5; // Train each record 5 times

void setup(void)
{
  // Initialize VR module at 9600 baud (default)
  myVR.begin(9600);
  
  /** initialize Serial monitor */
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F("======================================================"));
  Serial.println(F("Elechouse Voice Recognition V3 Module - Train Sample"));
  Serial.println(F("======================================================"));
  Serial.println();
  Serial.println(F("IMPORTANT: Each record will be trained 5 times for better accuracy!"));
  Serial.println();

  printSeperator();
  Serial.println(F("Usage:"));
  printSeperator();
  printHelp();
  printSeperator();
  
  cmd_cnt = 0;
  
  // Check initial connection
  Serial.println(F("\nChecking module connection..."));
  uint8_t sysSettings[5];
  if (myVR.checkSystemSettings(sysSettings) > 0) {
    Serial.println(F("✓ Module connected successfully!"));
  } else {
    Serial.println(F("✗ Module not responding. Check connections!"));
    Serial.println(F("  VR TX -> ESP32 RX (GPIO16)"));
    Serial.println(F("  VR RX -> ESP32 TX (GPIO17)"));
    Serial.println(F("  VR VCC -> 3.3V or 5V"));
    Serial.println(F("  VR GND -> GND"));
  }
}

void loop(void)
{
  int len, paraNum, paraLen, i;

  /** receive Serial command */
  len = receiveCMD();
  if (len > 0) {
    /** check if the received command is valid */
    if (!checkCMD(len)) {

      /** check parameter number of the received command  */
      paraNum = checkParaNum(len);

      /** display the received command back */
      Serial.print("> ");
      Serial.write(cmd, len);
      Serial.println();

      /** find the first parameter */
      paraLen = findPara(len, 1, &paraAddr);

      /** compare the received command with command in the list */
      for (i = 0; i < CMD_NUM; i++) {
        /** compare command length */
        if (paraLen == cmdLen[i]) {
          /** compare command content */
          if (compareCMD(paraAddr, (uint8_t *)cmdList[i], paraLen) == 0) {
            /** call command function */
            if (cmdFunction[i](len, paraNum) != 0) {
              printSeperator();
              Serial.println(F("Command Format Error!"));
              printSeperator();
            }
            break;
          }
        }
      }

      /** command is not supported*/
      if (i == CMD_NUM) { 
        printSeperator();
        Serial.println(F("Unknown command"));
        Serial.println(F("Type 'help' for available commands"));
        printSeperator();
      }
    } else {
      /** received command is invalid */
      printSeperator();
      Serial.println(F("Command format error - invalid characters"));
      printSeperator();
    }
  }

  /** try to receive recognize result */
  int ret;
  ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    /** voice recognized, print result */
    printVR(buf);
  }
}

/**
 * @brief   receive command from Serial.
 * @param   NONE.
 * @retval  command length, if no command receive return -1.
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
 * @brief   compare two commands, case insensitive.
 * @param   para1  -->  command buffer 1
 * para2  -->  command buffer 2
 * len    -->  buffer length
 * @retval  0  --> equal
 * -1  --> unequal
 */
int compareCMD(uint8_t *para1 , uint8_t *para2, int len)
{
  int i;
  uint8_t res;
  for (i = 0; i < len; i++) {
    res = para2[i] - para1[i];
    if (res != 0 && res != 0x20) {
      res = para1[i] - para2[i];
      if (res != 0 && res != 0x20) {
        return -1;
      }
    }
  }
  return 0;
}

/**
 * @brief   Check command format.
 * @param   len  -->  command length
 * @retval  0  -->  command is valid
 * -1  -->  command is invalid
 */
int checkCMD(int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (cmd[i] > 0x1F && cmd[i] < 0x7F) {
      // Valid ASCII character
    } else if (cmd[i] == '\t' || cmd[i] == ' ' || cmd[i] == '\r' || cmd[i] == '\n') {
      // Valid whitespace
    } else {
      return -1;
    }
  }
  return 0;
}

/**
 * @brief   Check the number of parameters in the command
 * @param   len  -->  command length
 * @retval  number of parameters
 */
int checkParaNum(int len)
{
  int cnt = 0, i;
  for (i = 0; i < len; ) {
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
 * @brief   Find the specified parameter.
 * @param   len       -->  command length
 * paraIndex -->  parameter index
 * addr      -->  return value. position of the parameter
 * @retval  length of specified parameter
 */
int findPara(int len, int paraIndex, uint8_t **addr)
{
  int cnt = 0, i, paraLen;
  uint8_t dt;
  
  for (i = 0; i < len; ) {
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
 * @brief   Convert string to integer (simple atoi for single/double digits)
 */
int simpleAtoi(uint8_t *str, int len) {
  int result = 0;
  for (int i = 0; i < len; i++) {
    if (str[i] >= '0' && str[i] <= '9') {
      result = result * 10 + (str[i] - '0');
    } else {
      return 0;
    }
  }
  return result;
}

int cmdHelp(int len, int paraNum)
{
  if (paraNum != 1) {
    return -1;
  }
  printSeperator();
  printHelp();
  printSeperator();
  return 0;
}

/**
 * @brief   Train a single record multiple times
 * @param   record  --> record number to train
 * @param   times   --> how many times to train (default 5)
 */
void trainRecordMultipleTimes(uint8_t record, int times = TRAINING_REPEATS) {
  Serial.print(F("Training record "));
  Serial.print(record);
  Serial.print(F(" "));
  Serial.print(times);
  Serial.println(F(" times..."));
  
  int successCount = 0;
  
  for (int attempt = 1; attempt <= times; attempt++) {
    Serial.print(F("  Attempt "));
    Serial.print(attempt);
    Serial.print(F("/"));
    Serial.print(times);
    Serial.print(F(": "));
    
    uint8_t trainResult[10];
    int result = myVR.train(record, trainResult);
    
    if (result >= 0) {
      if (result == 0 || (result > 0 && trainResult[2] == 0x00)) {
        Serial.println(F("SUCCESS"));
        successCount++;
      } else {
        Serial.print(F("FAILED - Status: 0x"));
        Serial.println(trainResult[2], HEX);
      }
    } else {
      Serial.println(F("ERROR"));
    }
    
    // Wait before next attempt (if not last)
    if (attempt < times) {
      Serial.println(F("  Wait 2 seconds, then say the command again..."));
      delay(2000);
    }
  }
  
  // Update training attempts counter
  trainingAttempts[record] = successCount;
  
  Serial.print(F("  Result: "));
  Serial.print(successCount);
  Serial.print(F("/"));
  Serial.print(times);
  Serial.println(F(" successful trainings"));
}

/**
 * @brief   Handle "train" command - trains each record 5 times
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdTrain(int len, int paraNum)
{
  int i;
  if (paraNum < 2 || paraNum > 8) {
    return -1;
  }

  // Parse record numbers
  for (i = 2; i <= paraNum; i++) {
    findPara(len, i, &paraAddr);
    records[i-2] = simpleAtoi(paraAddr, findPara(len, i, &paraAddr));
    if (records[i-2] == 0 && *paraAddr != '0') {
      return -1;
    }
  }
  
  printSeperator();
  Serial.println(F("=== ENHANCED TRAINING MODE ==="));
  Serial.println(F("Each record will be trained 5 times"));
  Serial.println(F("Speak clearly and consistently each time"));
  Serial.println(F("================================"));
  
  // Train each record 5 times
  for (i = 0; i < paraNum-1; i++) {
    Serial.print(F("\n["));
    Serial.print(i+1);
    Serial.print(F("/"));
    Serial.print(paraNum-1);
    Serial.print(F("] "));
    
    trainRecordMultipleTimes(records[i], TRAINING_REPEATS);
    
    // Small delay between different records
    if (i < paraNum-2) {
      delay(1000);
    }
  }
  
  // Show summary
  Serial.println(F("\n=== TRAINING SUMMARY ==="));
  for (i = 0; i < paraNum-1; i++) {
    Serial.print(F("Record "));
    Serial.print(records[i]);
    Serial.print(F(": "));
    Serial.print(trainingAttempts[records[i]]);
    Serial.print(F("/"));
    Serial.print(TRAINING_REPEATS);
    Serial.println(F(" successful"));
  }
  
  printSeperator();
  return 0;
}

/**
 * @brief   Handle "load" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdLoad(int len, int paraNum)
{
  int i, ret;
  if (paraNum < 2 || paraNum > 8) {
    return -1;
  }

  for (i = 2; i <= paraNum; i++) {
    findPara(len, i, &paraAddr);
    records[i-2] = simpleAtoi(paraAddr, findPara(len, i, &paraAddr));
    if (records[i-2] == 0 && *paraAddr != '0') {
      return -1;
    }
  }
  
  ret = myVR.load(records, paraNum-1, buf);
  printSeperator();
  if (ret >= 0) {
    printLoad(buf, ret);
  } else {
    Serial.println(F("Load failed or timeout."));
  }
  printSeperator();
  return 0;
}

/**
 * @brief   Handle "clear" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdClear(int len, int paraNum)
{
  if (paraNum != 1) {
    return -1;
  }
  
  printSeperator();
  Serial.print(F("Clearing recognizer... "));
  
  if (myVR.clear() == 0) {
    Serial.println(F("DONE"));
  } else {
    Serial.println(F("FAILED or timeout."));
  }
  printSeperator();
  return 0;
}

/**
 * @brief   Handle "vr" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdVR(int len, int paraNum)
{
  int ret;
  if (paraNum != 1) {
    return -1;
  }
  
  ret = myVR.checkRecognizer(buf);
  printSeperator();
  
  if (ret > 0) {
    printCheckRecognizer(buf);
  } else {
    Serial.println(F("Check recognizer failed or timeout."));
  }
  
  printSeperator();
  return 0;
}

/**
 * @brief   Handle "record" command with enhanced info
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdRecord(int len, int paraNum)
{
  int ret;
  
  if (paraNum == 1) {
    // Check all records
    ret = myVR.checkRecord(buf);
    printSeperator();
    if (ret >= 0) {
      printCheckRecordAll(buf, ret);
      
      // Show training attempts info
      Serial.println(F("\nTraining Attempts History:"));
      for (int i = 0; i < 50; i++) {
        if (trainingAttempts[i] > 0) {
          Serial.print(F("  Record "));
          Serial.print(i);
          Serial.print(F(": "));
          Serial.print(trainingAttempts[i]);
          Serial.println(F(" times"));
        }
      }
    } else {
      Serial.println(F("Check record failed or timeout."));
    }
    printSeperator();
  } else if (paraNum < 9) {
    // Check specific records
    for (int i = 2; i <= paraNum; i++) {
      findPara(len, i, &paraAddr);
      records[i-2] = simpleAtoi(paraAddr, findPara(len, i, &paraAddr));
      if (records[i-2] == 0 && *paraAddr != '0') {
        return -1;
      }      
    }

    ret = myVR.checkRecord(buf, records, paraNum-1);
    printSeperator();
    if (ret >= 0) {
      printCheckRecord(buf, ret);
      
      // Show training attempts for these records
      Serial.println(F("\nTraining Attempts:"));
      for (int i = 0; i < paraNum-1; i++) {
        Serial.print(F("  Record "));
        Serial.print(records[i]);
        Serial.print(F(": "));
        Serial.print(trainingAttempts[records[i]]);
        Serial.println(F(" times"));
      }
    } else {
      Serial.println(F("Check record failed or timeout."));
    }
    printSeperator();
  } else {
    return -1;
  }
  return 0;
}

/**
 * @brief   Handle "sigtrain" command with 5 attempts
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdSigTrain(int len, int paraNum)
{
  int ret, sig_len;
  uint8_t *lastAddr;
  
  if (paraNum < 2) {
    return -1;
  }

  findPara(len, 2, &paraAddr);
  records[0] = simpleAtoi(paraAddr, findPara(len, 2, &paraAddr));
  if (records[0] == 0 && *paraAddr != '0') {
    return -1;
  }

  findPara(len, 3, &paraAddr);
  sig_len = findPara(len, paraNum, &lastAddr);
  sig_len += ((unsigned int)lastAddr - (unsigned int)paraAddr);

  printSeperator();
  Serial.print(F("Training record "));
  Serial.print(records[0]);
  Serial.println(F(" with signature (5 attempts)..."));
  
  int successCount = 0;
  for (int attempt = 1; attempt <= 5; attempt++) {
    Serial.print(F("  Attempt "));
    Serial.print(attempt);
    Serial.print(F("/5: "));
    
    ret = myVR.trainWithSignature(records[0], paraAddr, sig_len, buf);
    
    if (ret >= 0) {
      Serial.println(F("SUCCESS"));
      successCount++;
    } else {
      Serial.println(F("FAILED"));
    }
    
    if (attempt < 5) {
      Serial.println(F("  Wait 2 seconds, then repeat..."));
      delay(2000);
    }
  }
  
  Serial.print(F("  Result: "));
  Serial.print(successCount);
  Serial.println(F("/5 successful"));
  
  if (successCount > 0) {
    printSigTrain(buf, ret);
  } else {
    Serial.println(F("Train with signature failed."));
  }
  printSeperator();

  return 0;
}

/**
 * @brief   Handle "getsig" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdGetSig(int len, int paraNum)
{
  int ret;
  if (paraNum != 2) {
    return -1;
  }

  findPara(len, 2, &paraAddr);
  records[0] = simpleAtoi(paraAddr, findPara(len, 2, &paraAddr));
  if (records[0] == 0 && *paraAddr != '0') {
    return -1;
  }

  ret = myVR.checkSignature(records[0], buf);

  printSeperator();
  if (ret == 0) {
    Serial.print(F("Record "));
    Serial.print(records[0]);
    Serial.println(F(" has no signature."));
  } else if (ret > 0) {
    Serial.print(F("Record "));
    Serial.print(records[0]);
    Serial.print(F(" signature: "));
    printSignature(buf, ret);
    Serial.println();
  } else {
    Serial.println(F("Get signature error or timeout."));
  }
  printSeperator();

  return 0;
}

/**
 * @brief   Handle "test" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdTest(int len, int paraNum)
{
  printSeperator();
  Serial.println(F("TEST command is not supported in this sample."));
  printSeperator();
  return 0;
}

/**
 * @brief   Handle "settings" command
 * @param   len     --> command length
 * paraNum --> number of parameters
 * @retval  0 --> success
 * -1 --> Command format error
 */
int cmdSettings(int len, int paraNum)
{
  int ret;
  if (paraNum != 1) {
    return -1;
  }
  
  ret = myVR.checkSystemSettings(buf);
  printSeperator();
  
  if (ret > 0) {
    printSystemSettings(buf, ret);
  } else {
    Serial.println(F("Check system settings error or timeout"));
  }
  
  printSeperator();
  return 0;
}

/*****************************************************************************/
/**
 * @brief   Print signature, if the character is invisible, 
 * print hexible value instead.
 * @param   buf     --> command length
 * len     --> number of parameters
 */
void printSignature(uint8_t *buf, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    } else {
      Serial.print(F("["));
      Serial.print(buf[i], HEX);
      Serial.print(F("]"));
    }
  }
}

/**
 * @brief   Print recognition result
 * @param   buf  -->  VR module return value when voice is recognized.
 * buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
 * buf[1]  -->  number of record which is recognized. 
 * buf[2]  -->  Recognizer index(position) value of the recognized record.
 * buf[3]  -->  Signature length
 * buf[4]~buf[n] --> Signature
 */
void printVR(uint8_t *buf)
{
  Serial.println(F("╔════════════════════════════════════════╗"));
  Serial.println(F("║         VOICE RECOGNIZED!             ║"));
  Serial.println(F("╠════════════════════════════════════════╣"));
  
  Serial.print(F("║ Index: "));
  Serial.print(buf[2], DEC);
  Serial.println(F("                              ║"));
  
  Serial.print(F("║ Group: "));
  if (buf[0] == 0xFF) {
    Serial.print(F("NONE"));
  } else if (buf[0] & 0x80) {
    Serial.print(F("USER "));
    Serial.print(buf[0] & (~0x80), DEC);
  } else {
    Serial.print(F("SYSTEM "));
    Serial.print(buf[0], DEC);
  }
  Serial.println(F("                          ║"));
  
  Serial.print(F("║ Record: "));
  Serial.print(buf[1], DEC);
  Serial.println(F("                             ║"));
  
  Serial.print(F("║ Signature: "));
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  } else {
    Serial.print(F("NONE"));
  }
  Serial.println(F("                  ║"));
  
  Serial.println(F("╚════════════════════════════════════════╝"));
  Serial.println();
}

/**
 * @brief   Print separator. Print 60 '-'.
 */
void printSeperator()
{
  Serial.println(F("────────────────────────────────────────────"));
}

/**
 * @brief   Print recognizer status.
 * @param   buf  -->  VR module return value when voice is recognized.
 * buf[0]     -->  Number of valid voice records in recognizer
 * buf[i+1]   -->  Record number.(0xFF: Not loaded(Nongroup mode), or not set (Group mode)) (i= 0, 1, ... 6)
 * buf[8]     -->  Number of all voice records in recognizer
 * buf[9]     -->  Valid records position indicate.
 * buf[10]    -->  Group mode indicate(FF: None Group, 0x8n: User, 0x0n:System)
 */
void printCheckRecognizer(uint8_t *buf)
{
  Serial.println(F("RECOGNIZER STATUS:"));
  printSeperator();
  
  Serial.print(F("Total slots: "));
  Serial.println(buf[8], DEC);
  
  Serial.print(F("Loaded records: "));
  Serial.println(buf[0], DEC);
  
  if (buf[10] == 0xFF) {
    Serial.println(F("Group mode: NONE"));
  } else if (buf[10] & 0x80) {
    Serial.print(F("Group mode: USER "));
    Serial.println(buf[10] & 0x7F, DEC);
  } else {
    Serial.print(F("Group mode: SYSTEM "));
    Serial.println(buf[10], DEC);
  }
  
  Serial.println();
  Serial.println(F("Slot\tRecord\tStatus"));
  Serial.println(F("────\t──────\t──────"));
  
  for (int i = 0; i < 7; i++) {
    Serial.print(i, DEC);
    Serial.print(F("\t"));
    
    if (buf[i+1] == 0xFF) {
      if (buf[10] == 0xFF) {
        Serial.print(F("----\tEMPTY"));
      } else {
        Serial.print(F("----\tNOT SET"));
      }
    } else {
      Serial.print(buf[i+1], DEC);
      Serial.print(F("\t"));
      if (buf[9] & (1 << i)) {
        Serial.print(F("VALID"));
      } else {
        Serial.print(F("UNTRAINED"));
      }
    }
    Serial.println();
  }
}

/**
 * @brief   Print record train status.
 * @param   buf  -->  Check record command return value
 * buf[0]     -->  Number of checked records
 * buf[2i+1]  -->  Record number.
 * buf[2i+2]  -->  Record train status. (00: untrained, 01: trained, FF: record value out of range)
 * (i = 0 ~ buf[0]-1 )
 * num  -->  Number of trained records
 */
void printCheckRecord(uint8_t *buf, int num)
{
  Serial.print(F("Checked "));
  Serial.print(buf[0], DEC);
  Serial.println(F(" record(s):"));
  
  Serial.print(num, DEC);
  if (num != 1) {
    Serial.println(F(" records trained."));
  } else {
    Serial.println(F(" record trained."));
  }

  Serial.println();
  Serial.println(F("Record\tStatus"));
  Serial.println(F("──────\t──────"));
  
  for (int i = 0; i < buf[0]*2; i += 2) {
    Serial.print(buf[i+1], DEC);
    Serial.print(F("\t"));
    
    switch (buf[i+2]) {
      case 0x01:
        Serial.println(F("TRAINED"));
        break;
      case 0x00:
        Serial.println(F("UNTRAINED"));
        break;
      case 0xFF:
        Serial.println(F("OUT OF RANGE"));
        break;
      default:
        Serial.println(F("UNKNOWN"));
        break;
    }
  }
}

/**
 * @brief   Print record train status for all records.
 */
void printCheckRecordAll(uint8_t *buf, int num)
{
  Serial.println(F("All 255 records status:"));
  Serial.print(num, DEC);
  
  if (num != 1) {
    Serial.println(F(" records trained."));
  } else {
    Serial.println(F(" record trained."));
  }

  Serial.println();
  Serial.println(F("Record\tStatus"));
  Serial.println(F("──────\t──────"));
  
  int trainedCount = 0;
  for (int i = 0; i < 255; i++) {
    if (buf[i] == 0xF0) {
      continue;
    }
    
    if (buf[i] == 0x01) {
      trainedCount++;
      Serial.print(i, DEC);
      Serial.print(F("\t"));
      
      switch (buf[i]) {
        case 0x01:
          Serial.println(F("TRAINED"));
          break;
        case 0x00:
          Serial.println(F("UNTRAINED"));
          break;
        case 0xFF:
          Serial.println(F("OUT OF RANGE"));
          break;
        default:
          Serial.println(F("UNKNOWN"));
          break;
      }
    }
  }
  
  if (trainedCount == 0) {
    Serial.println(F("No trained records found."));
  }
}

/**
 * @brief   Print "load" command return value.
 * @param   buf  -->  "load" command return value
 * buf[0]    -->  number of records which are load successfully.
 * buf[2i+1]  -->  record number
 * buf[2i+2]  -->  record load status.
 * 00 --> Loaded 
 * FC --> Record already in recognizer
 * FD --> Recognizer full
 * FE --> Record untrained
 * FF --> Value out of range"
 * (i = 0 ~ (len-1)/2 )
 * len  -->  length of buf
 */
void printLoad(uint8_t *buf, uint8_t len)
{
  if (len == 0) {
    Serial.println(F("✓ All records loaded successfully."));
    return;
  } else {
    Serial.print(F("Successfully loaded: "));
    Serial.print(buf[0], DEC);
    Serial.println(F(" record(s)"));
  }
  
  Serial.println();
  Serial.println(F("Record\tResult"));
  Serial.println(F("──────\t──────"));
  
  for (int i = 0; i < len-1; i += 2) {
    Serial.print(buf[i+1], DEC);
    Serial.print(F("\t"));
    
    switch (buf[i+2]) {
      case 0:
        Serial.println(F("LOADED"));
        break;
      case 0xFC:
        Serial.println(F("ALREADY LOADED"));
        break;
      case 0xFD:
        Serial.println(F("RECOGNIZER FULL"));
        break;
      case 0xFE:
        Serial.println(F("UNTRAINED"));
        break;
      case 0xFF:
        Serial.println(F("OUT OF RANGE"));
        break;
      default:
        Serial.println(F("UNKNOWN"));
        break;
    }
  }
}

/**
 * @brief   Print "train" command return value.
 * @param   buf  -->  "train" command return value
 * buf[0]    -->  number of records which are trained successfully.
 * buf[2i+1]  -->  record number
 * buf[2i+2]  -->  record train status.
 * 00 --> Trained 
 * FE --> Train Time Out
 * FF --> Value out of range"
 * (i = 0 ~ len-1 )
 * len  -->  length of buf
 */
void printTrain(uint8_t *buf, uint8_t len)
{
  if (len == 0) {
    Serial.println(F("✓ Training completed successfully!"));
    return;
  } else {
    Serial.print(F("Successfully trained: "));
    Serial.print(buf[0], DEC);
    Serial.println(F(" record(s)"));
  }
  
  Serial.println();
  Serial.println(F("Record\tResult"));
  Serial.println(F("──────\t──────"));
  
  for (int i = 0; i < len-1; i += 2) {
    Serial.print(buf[i+1], DEC);
    Serial.print(F("\t"));
    
    switch (buf[i+2]) {
      case 0:
        Serial.println(F("TRAINED"));
        break;
      case 0xFE:
        Serial.println(F("TIMEOUT"));
        break;
      case 0xFF:
        Serial.println(F("OUT OF RANGE"));
        break;
      default:
        Serial.print(F("UNKNOWN "));
        Serial.println(buf[i+2], HEX);
        break;
    }
  }
}

/**
 * @brief   Print "sigtrain" command return value.
 * @param   buf  -->  "sigtrain" command return value
 * buf[0]  -->  number of records which are trained successfully.
 * buf[1]  -->  record number
 * buf[2]  -->  record train status.
 * 00 --> Trained 
 * F0 --> Trained, signature truncate
 * FE --> Train Time Out
 * FF --> Value out of range"
 * buf[3] ~ buf[len-1] --> Signature.
 * len  -->  length of buf
 */
void printSigTrain(uint8_t *buf, uint8_t len)
{
  if (len == 0) {
    Serial.println(F("✓ Training with signature completed!"));
    return;
  } else {
    Serial.print(F("Success: "));
    Serial.println(buf[0], DEC);
  }
  
  Serial.print(F("Record "));
  Serial.print(buf[1], DEC);
  Serial.print(F("\t"));
  
  switch (buf[2]) {
    case 0:
      Serial.println(F("TRAINED"));
      break;
    case 0xF0:
      Serial.println(F("TRAINED (signature truncated)"));
      break;
    case 0xFE:
      Serial.println(F("TIMEOUT"));
      break;
    case 0xFF:
      Serial.println(F("OUT OF RANGE"));
      break;
    default:
      Serial.print(F("UNKNOWN "));
      Serial.println(buf[2], HEX);
      break;
  }
  
  Serial.print(F("Signature: "));
  Serial.write(buf + 3, len - 3);
  Serial.println();
}

/**
 * @brief   Print "settings" command return value.
 * @param   buf  -->  "settings" command return value
 * len  -->  length of buf
 */

const unsigned int io_pw_tab[16] = {
  10,  15,  20,  25,  30,  35,  40,  45, 
  50,  75,  100, 200, 300, 400, 500, 1000
};

void printSystemSettings(uint8_t *buf, int len)
{
  Serial.println(F("SYSTEM SETTINGS:"));
  printSeperator();

  // Baud rate
  Serial.print(F("Baud Rate: "));
  switch (buf[0]) {
    case 0:
    case 3:
      Serial.println(F("9600"));
      break;
    case 1:
      Serial.println(F("2400"));
      break;
    case 2:
      Serial.println(F("4800"));
      break;
    case 4:
      Serial.println(F("19200"));
      break;
    case 5:
      Serial.println(F("38400"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }

  // IO Mode
  Serial.print(F("IO Mode: "));
  switch (buf[1]) {
    case 0:
    case 0xFF:
      Serial.println(F("PULSE"));
      break;
    case 1:
      Serial.println(F("TOGGLE"));
      break;
    case 2:
      Serial.println(F("CLEAR (when recognized)"));
      break;
    case 3:
      Serial.println(F("SET (when recognized)"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }

  // Pulse Width
  Serial.print(F("Pulse Width: "));
  if (buf[2] > 15) {
    Serial.println(F("UNKNOWN"));
  } else {
    Serial.print(io_pw_tab[buf[2]], DEC);
    Serial.println(F(" ms"));
  }

  // Auto Load
  Serial.print(F("Auto Load: "));
  if (buf[3] == 0 || buf[3] == 0xFF) {
    Serial.println(F("DISABLED"));
  } else {
    Serial.println(F("ENABLED"));
  }

  // Group Control
  Serial.print(F("Group Control: "));
  switch (buf[4]) {
    case 0:
    case 0xFF:
      Serial.println(F("DISABLED"));
      break;
    case 1:
      Serial.println(F("SYSTEM GROUP"));
      break;
    case 2:
      Serial.println(F("USER GROUP"));
      break;
    default:
      Serial.println(F("UNKNOWN"));
      break;
  }
}

void printHelp(void)
{
  Serial.println(F("COMMAND REFERENCE:"));
  printSeperator();
  Serial.println(F("train <r0> <r1>...      Train records 5 times each (for better accuracy)"));
  Serial.println(F("load <r0> <r1> ...      Load records into recognizer"));
  Serial.println(F("clear                   Clear all loaded records"));
  Serial.println(F("record [r0 r1...]       Check record training status"));
  Serial.println(F("vr                      Check recognizer status"));
  Serial.println(F("getsig <r>              Get signature of record"));
  Serial.println(F("sigtrain <r> <sig>      Train record with signature (5 times)"));
  Serial.println(F("settings                Check system settings"));
  Serial.println(F("help                    Show this help"));
  printSeperator();
  Serial.println(F("EXAMPLES:"));
  Serial.println(F("  train 0 1 2          Train records 0, 1, and 2 (5 times each)"));
  Serial.println(F("  load 0 2             Load records 0 and 2"));
  Serial.println(F("  record               Check all records"));
  Serial.println(F("  record 0 1           Check records 0 and 1"));
  Serial.println(F("  getsig 0             Get signature of record 0"));
  Serial.println(F("  sigtrain 0 LIGHT     Train record 0 with signature 'LIGHT' (5 times)"));
}