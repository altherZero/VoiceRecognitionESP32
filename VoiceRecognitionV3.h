/**
  ******************************************************************************
  * @file    VoiceRecognitionV3.h
  * @author  Elechouse Team
  * @version V1.0
  * @date    2013-6-6
  * @brief   This file provides all the VoiceRecognitionV3 firmware functions.
  ******************************************************************************
    @note
         This driver is for elechouse Voice Recognition V3 Module
  ******************************************************************************
  * @section  HISTORY
  
    V1.0    Initial version.
    V1.1    Modified for ESP32 and HardwareSerial compatibility
    
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, ELECHOUSE SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 ELECHOUSE</center></h2>
  ******************************************************************************
  */

#ifndef VoiceRecognitionV3_h
#define VoiceRecognitionV3_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "HardwareSerial.h"

// Debug configuration - comment out to disable debug messages
// #define VR_DEBUG

#ifdef VR_DEBUG
  #define DBGSTR(message)     Serial.print(message)
  #define DBGBUF(buf, len)    Serial.write(buf, len)
  #define DBGLN(message)      Serial.println(message)
  #define DBGFMT(msg, fmt)    Serial.print(msg, fmt)
  #define DBGCHAR(c)          Serial.write(c)
#else
  #define DBGSTR(message)
  #define DBGBUF(buf, len)
  #define DBGLN(message)
  #define DBGFMT(msg, fmt)
  #define DBGCHAR(c)
#endif // VR_DEBUG

#define VR_DEFAULT_TIMEOUT            (1000)

/***************************************************************************/
#define FRAME_HEAD                     (0xAA)
#define FRAME_END                      (0x0A)

/***************************************************************************/
#define FRAME_CMD_CHECK_SYSTEM         (0x00)
#define FRAME_CMD_CHECK_BSR            (0x01)
#define FRAME_CMD_CHECK_TRAIN          (0x02)
#define FRAME_CMD_CHECK_SIG            (0x03)

#define FRAME_CMD_RESET_DEFAULT        (0x10)  //reset configuration
#define FRAME_CMD_SET_BR               (0x11)  //baud rate
#define FRAME_CMD_SET_IOM              (0x12)  //IO mode
#define FRAME_CMD_SET_PW               (0x13)  //pulse width
#define FRAME_CMD_RESET_IO             (0x14)  // reset IO OUTPUT          
#define FRAME_CMD_SET_AL               (0x15)  // Auto load

#define FRAME_CMD_TRAIN                (0x20)
#define FRAME_CMD_SIG_TRAIN            (0x21)
#define FRAME_CMD_SET_SIG              (0x22)

#define FRAME_CMD_LOAD                 (0x30)  //Load N records
#define FRAME_CMD_CLEAR                (0x31)  //Clear BSR buffer            
#define FRAME_CMD_GROUP                (0x32)  //
  #define FRAME_CMD_GROUP_SET              (0x00)  //
  #define FRAME_CMD_GROUP_SUGRP            (0x01)  //
  #define FRAME_CMD_GROUP_LSGRP            (0x02)  //
  #define FRAME_CMD_GROUP_LUGRP            (0x03)  //
  #define FRAME_CMD_GROUP_CUGRP            (0x04)  //

#define FRAME_CMD_TEST                 (0xEE)
  #define FRAME_CMD_TEST_READ              (0x01)  
  #define FRAME_CMD_TEST_WRITE             (0x00)  

#define FRAME_CMD_VR                   (0x0D)  //Voice recognized
#define FRAME_CMD_PROMPT               (0x0A)
#define FRAME_CMD_ERROR                (0xFF)

/***************************************************************************/

class VR {
public:
  // Constructor for ESP32 - uses HardwareSerial
  VR(HardwareSerial* serialPort = &Serial2, int rxPin = -1, int txPin = -1);
  
  static VR* getInstance() {
    return instance;
  }
  
  typedef enum {
    PULSE = 0,
    TOGGLE = 1,
    SET = 2, 
    CLEAR = 3
  } io_mode_t;
  
  typedef enum {
    LEVEL0 = 0,
    LEVEL1,
    LEVEL2, 
    LEVEL3,
    LEVEL4,
    LEVEL5,
    LEVEL6,
    LEVEL7,
    LEVEL8,
    LEVEL9,
    LEVEL10,
    LEVEL11,
    LEVEL12,
    LEVEL13,
    LEVEL14,
    LEVEL15,
  } pulse_width_level_t;
  
  typedef enum {
    GROUP0 = 0,
    GROUP1,
    GROUP2,
    GROUP3,
    GROUP4,
    GROUP5,
    GROUP6,
    GROUP7,
    GROUP_ALL = 0xFF,
  } group_t;
  
  // Initialize with custom baud rate
  void begin(unsigned long baudRate = 9600);
  
  // Configuration functions
  int setBaudRate(unsigned long br);
  int setIOMode(io_mode_t mode);
  int resetIO(uint8_t *ios = NULL, uint8_t len = 1);
  int setPulseWidth(uint8_t level);
  int setAutoLoad(uint8_t *records = NULL, uint8_t len = 0);
  int disableAutoLoad();
  int restoreSystemSettings();
  int checkSystemSettings(uint8_t* buf);
  
  // Recognition functions
  int recognize(uint8_t *buf, int timeout = VR_DEFAULT_TIMEOUT);
  
  // Training functions
  int train(uint8_t *records, uint8_t len = 1, uint8_t *buf = NULL);
  int train(uint8_t record, uint8_t *buf = NULL);
  int trainWithSignature(uint8_t record, const void *buf, uint8_t len = 0, uint8_t *retbuf = NULL);
  
  // Record management
  int load(uint8_t *records, uint8_t len = 1, uint8_t *buf = NULL);
  int load(uint8_t record, uint8_t *buf = NULL);
  int clear();
  int setSignature(uint8_t record, const void *buf = NULL, uint8_t len = 0);
  int deleteSignature(uint8_t record);
  
  // Checking functions
  int checkSignature(uint8_t record, uint8_t *buf);
  int checkRecognizer(uint8_t *buf);
  int checkRecord(uint8_t *buf, uint8_t *records = NULL, uint8_t len = 0);
  
  /** Group control */
  int setGroupControl(uint8_t ctrl);
  int checkGroupControl();
  int setUserGroup(uint8_t grp, uint8_t *records, uint8_t len);
  int checkUserGroup(uint8_t grp, uint8_t *buf);
  int loadSystemGroup(uint8_t grp, uint8_t *buf = NULL);
  int loadUserGroup(uint8_t grp, uint8_t *buf = NULL);
  
  // Test function
  int test(uint8_t cmd, uint8_t *bsr);
  
  // Utility functions
  int writehex(uint8_t *buf, uint8_t len);
  
  /***************************************************************************/
  /** low level */
  int len(uint8_t *buf);
  int cmp(uint8_t *buf, uint8_t *bufcmp, int len);
  void cpy(char *buf, char *pbuf);
  void sort(uint8_t *buf, int len);
  int cleanDup(uint8_t *des, uint8_t *buf, int len);
  void send_pkt(uint8_t *buf, uint8_t len);
  void send_pkt(uint8_t cmd, uint8_t *buf, uint8_t len);
  void send_pkt(uint8_t cmd, uint8_t subcmd, uint8_t *buf, uint8_t len);
  int receive(uint8_t *buf, int len, uint16_t timeout = VR_DEFAULT_TIMEOUT);
  int receive_pkt(uint8_t *buf, uint16_t timeout = VR_DEFAULT_TIMEOUT);
  
  // Helper function to check if data is available
  int available();
  void flush();
  
private:
  static VR* instance;
  HardwareSerial* _serial;
  int _rxPin;
  int _txPin;
  unsigned long _baudRate;
};

#endif // VoiceRecognitionV3_h