
#include "gw.h"
#include "pltfrm.h"
#include "gw_uart.h"
#include "dis.h"

#include<string.h>

#include <AltSoftSerial.h>
#include <SoftwareSerial.h>
const int RX1 = 8;
const int TX1 = 9;


AltSoftSerial serialGW(RX1, TX1);
SoftwareSerial SerialGSM(10, 11);

//#define SLOW_CLOCK
const char FIELD1[] PROGMEM = "field1=";

char response[20];
//#define GW1

#ifdef GW1
#define API_KEY1 "08YLY5OJHJMBRYRU" // gw1 
#define API_KEY2 "8C9IUJEKG7EMZEZV" //gw1
#define API_KEY3 "TJLEDTD26UNO6O3Y" //gw1
#define API_KEY4 "F101S4IZ4H8T6R2D"// gw1
#else
#define API_KEY1 "R6A9FZWU2WSKGY3H" //gw2
#define API_KEY2 "USOGS2X4OXAYMMYA" //gw2
#define API_KEY3 "KEY3"             //gw2
#define API_KEY4 "KEY4"             //gw2
#endif

short Fields = 8; //valCheck = 0; // Give the no: of fields to be uploaded. Max 8 and min 0 in the numerical order of fields.

String API_KEY;

#define UART_RX_BUFF_LEN 128

int verbose = 0;

long GW_uartRxCnt = 0;
long GW_msgTotLen = UART_FRAME_HDR_LEN;
long GW_msgPyldLen = 0;
long GW_rcvdMsgCnt = 0;

unsigned char GW_uartRxBuff[UART_RX_BUFF_LEN];


char GW_printBuff[64];

#define GW_print(...) \
  do \
  { \
    sprintf(GW_printBuff, __VA_ARGS__); \
    Serial.print(GW_printBuff); \
  } while(0)


/*
 ********************************************************************




 ********************************************************************
*/
unsigned char TLV_get(unsigned char *buff_p, unsigned char len, unsigned char type,
                      unsigned char *pyldLen_p, unsigned char **pyldBuff_pp)
{
  int buffLen = len;
  unsigned char rc = 0;

  if (buffLen < DIS_TLV_HDR_SZ)
    return 0;

  // Get the tlv type

  while (buffLen >= DIS_TLV_HDR_SZ)
  {
    unsigned char tlvPyldLen = *(buff_p + DIS_TLV_TYPE_FIELD_LEN);

    if (*buff_p == type)
    {
      *pyldLen_p = tlvPyldLen;
      *pyldBuff_pp = (buff_p + DIS_TLV_HDR_SZ);
      rc = 1;
      break;
    }
    else
    {
      buff_p += (DIS_TLV_HDR_SZ + tlvPyldLen);
      buffLen -= (DIS_TLV_HDR_SZ + tlvPyldLen);
    }
  }

  return rc;
}


float __latestVcc;
int __latestVccSet = 0;


/*
 ********************************************************************




 ********************************************************************
*/
unsigned short GW_ntohs(unsigned char *buff_p)
{
  short u16Val = *buff_p;
  u16Val = (u16Val << 8) | buff_p[1];
  return u16Val;
}


/*
 ********************************************************************




 ********************************************************************
*/
void GW_htonl(unsigned char *buff_p, unsigned long val)
{
  buff_p[3] = (val >> 24) & 0xff;
  buff_p[2] = (val >> 16) & 0xff;
  buff_p[1] = (val >> 8) & 0xff;
  buff_p[0] = (val) & 0xff;
}


/*
 ********************************************************************




 ********************************************************************
*/
void GW_htons(unsigned char *buff_p, unsigned short val)
{
  buff_p[0] = (val >> 8) & 0xff;
  buff_p[1] = (val) & 0xff;
}


/*
 ********************************************************************




 ********************************************************************
*/
unsigned long GW_ntohl(unsigned char *buff_p)
{
  unsigned long u32Val = *buff_p;
  u32Val <<= 8;
  u32Val |= buff_p[1];
  u32Val <<= 8;
  u32Val |= buff_p[2];
  u32Val <<= 8;
  u32Val |= buff_p[3];

  return u32Val;
}



const char set_Baud[] PROGMEM = "AT+IPR=9600";
const char stop_Echo[] PROGMEM = "ATE0";
const char set_CGATT[] PROGMEM = "AT+CGATT?";
const char cip_shut[] PROGMEM = "AT+CIPSHUT";
const char mux[] PROGMEM = "AT+CIPMUX=0";
const char cstt[] PROGMEM = "AT+CSTT=\"airtelgprs.com\"";
const char ciicr[] PROGMEM = "AT+CIICR";
const char cifsr[] PROGMEM = "AT+CIFSR";
const char ok[] PROGMEM = "OK";
const char con[] PROGMEM = "CONNECT";
    
const char* const string_table[] PROGMEM = {set_Baud, stop_Echo, set_CGATT, cip_shut, mux, cstt, ciicr, cifsr,ok,con};
char buffer[64];
    
void GSM_INIT()
{
    short k;
    for(k = 0; k< 8 ; k++)
    {
      strcpy_P(buffer, (char*)pgm_read_word(&(string_table[k]))); // Necessary casts and dereferencing
      SerialGSM.println(buffer);
      delay(100);
      short int i=0;
      while(!SerialGSM.available());
      while(SerialGSM.available())
      {
         response[i]=SerialGSM.read();
         i++;
      }
     Serial.println(response);
    }
    return; 
}

short int retry=0; //for cip retry

boolean cipstart()
{
  boolean sts = false;
  SerialGSM.print(F("AT+CIPSTART="));
  SerialGSM.print(F("\"TCP\""));   // DEFINED CON "TCP"
  SerialGSM.print(F(","));
  SerialGSM.print(F("\"api.thingspeak.com\""));  // DEFINED API "api.thingspeak.com"
  SerialGSM.print(F(","));
  SerialGSM.println(F("\"80\"")); // DEFINED PORT "80"
  delay(2000);

  short int i=0;
  while(!SerialGSM.available());
  while(SerialGSM.available())
  {
    response[i]=SerialGSM.read();
    i++;
  }
  Serial.println(response);
  strcpy_P(buffer, (char*)pgm_read_word(&(string_table[9]))); // PROGMEM "CONNECT"
  delay(200);
  if((strstr(response, buffer)!= NULL || strstr(response, "ALREADY")!= NULL) && strstr(response, "FAIL")== NULL ) 
  {
    sts=true;
  }
  else
  {
    Serial.println(F("CIP RETRY"));
    retry++;
    memset(response, 0, sizeof(response));
    memset(buffer, 0, sizeof(buffer));
    if(retry==3)
    {
      retry=0;
      Serial.println(F("AT+CFUN=1,1 :  Resetting Modem"));
      SerialGSM.println(F("AT+CFUN=1,1"));
      delay(16000);
      GSM_INIT();
    }
    cipstart();
  }
  memset(response, 0, sizeof(response));
  memset(buffer, 0, sizeof(buffer));
  return sts;
}

void mac_api(unsigned long a, unsigned long b, unsigned long c )
{
  #ifdef GW1
  if ( a == 0x16 && b == 0x3f && c == 0xea) //10(16):3f(63):ea(234)
  {
    API_KEY = API_KEY1;
    a = 0; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 1"));
  }

  if ( a == 0x11 && b == 0x24 && c == 0x3f) // 11(17):24(36):3f(63)
  {
    API_KEY = API_KEY2;
    a = 0 ; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 2"));
  }
   if ( a == 0x10 && b == 0x35 && c == 0xeb) // 10:35:eb
  {
    API_KEY = API_KEY3;
    a = 0 ; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 3"));
  }
  
  if ( a == 0x11 && b == 0x48 && c == 0x8f) //11:48:8f
  {
    API_KEY = API_KEY4;
    a = 0; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 4"));
  }
  
  #else
  
  if ( a == 0x11 && b == 0x48 && c == 0x99) //11:48:99
  {
    API_KEY = API_KEY1;
    a = 0; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 5"));
  }

  if ( a == 0x10 && b == 0x6f && c == 0x17) // 10:6f:17
  {
    API_KEY = API_KEY2;
    a = 0 ; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 6"));
  }
   if ( a == 0x11 && b == 0x1b && c == 0x78) // 11:1b:78
  {
    API_KEY = API_KEY3;
    a = 0 ; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 7"));
  }
  
  if ( a == 0x10 && b == 0x5e && c == 0x8e) //10:5e:8e
  {
    API_KEY = API_KEY4;
    a = 0; b = 0; c = 0;
    Serial.println(F("UP FROM NODE 8"));
  }
  
  #endif
}




const char AT_CIPSEND[] PROGMEM = "AT+CIPSEND";
const char post_hdr1[] PROGMEM = "POST /update HTTP/1.1";
const char post_hdr2[] PROGMEM = "Host: api.thingspeak.com";
const char post_hdr3[] PROGMEM = "Connection: close";
const char post_hdr4[] PROGMEM = "X-THINGSPEAKAPIKEY: ";
const char post_hdr5[] PROGMEM = "Content-Type: application/x-www-form-urlencoded";
const char content_length[] PROGMEM = "Content-Length:";
const char cok[] PROGMEM = "CLOSE OK";

const char* const post_data_cmd_string[] PROGMEM = {AT_CIPSEND, post_hdr1, post_hdr2, post_hdr3, post_hdr4, post_hdr5,content_length,cok};




unsigned short __crc16(unsigned char *buff_p, unsigned int len)
{
  unsigned long ckSum = 0;

  while (len > 1)
  {
    unsigned short tmp = *buff_p;
    tmp = (tmp << 8) | (*(buff_p + 1));
    ckSum = ckSum + tmp;
    buff_p += 2;
    len -= 2;
  }

  if (len > 0)
    ckSum += (*buff_p);

  while (ckSum >> 16)
  {
    ckSum = (ckSum & 0xffff) + (ckSum >> 16);
  }

  return (unsigned short)(~ckSum);
}

float value[8];

void GW_sendDataToCloud(int snsrId, float valF)
{

  short len = 0;
  //valF *= 100;
  
  //GW_print(" <%ld %s> <snsrId - %d> \r\n", sensorVal, unit_p, snsrId);

  if (snsrId == PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID)
  {
    value[0] = valF;
  }
  else if (snsrId == PLTFRM_CC2D33S_1_RH_DEV_ID)
  {
    value[1] = valF;
  }
  else if (snsrId == PLTFRM_CC2D33S_1_TEMP_DEV_ID)
  {
    value[2] = valF;
  }
  else if (snsrId == PLTFRM_BMP280_1_PRESSURE_DEV_ID)
  {
    value[3] = valF;
  }
  else if (snsrId == PLTFRM_BMP280_1_TEMPERATURE_DEV_ID)
  {
    value[4] = valF;
  }
  else if (snsrId == PLTFRM_CHIRP_PWLA_1_DEV_ID)
  {
    value[5] = valF;
  }
  else if (snsrId == PLTFRM_DS18B20_1_DEV_ID)
  {
    value[6] = valF;
  }
  else if (snsrId == PLTFRM_LM75B_1_DEV_ID)
  {
    serialGW.end();
    delay(100);
    value[7] = valF;

    for (short i = 0 ; i < Fields ; i++)
    {
      String val;
      val = String(value[i]);
      len += val.length();
      //Serial.println(value[i]);
    }
    
    //Serial.println("Reached post!");
    GSM_INIT();  // Function returs nothing
    Serial.println(F("Now CIP start 2"));
    cipstart();   // function can return a true for success

      short k;
      for(k = 0; k< 6; k++)
      {
        strcpy_P(buffer, (char*)pgm_read_word(&(post_data_cmd_string[k]))); // Necessary casts and dereferencing, just copy.
        
        if(k == 4)
        {
          SerialGSM.print(buffer);
          SerialGSM.println(API_KEY);
        
          Serial.print(buffer);
          Serial.println(API_KEY);
          delay(500);
        }
        else
        {
          SerialGSM.println(buffer);
          Serial.println(buffer);
          delay(500);
        }  
       memset(buffer, 0, sizeof(buffer)); 
    }
    API_KEY = "\0";
    strcpy_P(buffer, (char*)pgm_read_word(&(post_data_cmd_string[6])));  // PROGMEM "Content-Length:"
    SerialGSM.print(buffer);
    memset(buffer, 0, 64); 
    if (Fields == 8)
    {
      SerialGSM.println(len + (Fields * strlen(FIELD1)) + (Fields - 1));
      Serial.println(len + (Fields * strlen(FIELD1)) + (Fields - 1));
      SerialGSM.println("");
      delay(500);
      
      SerialGSM.print(F("field1="));
      SerialGSM.print(value[0]);
      SerialGSM.print(F("&"));
      
      SerialGSM.print(F("field2="));
      SerialGSM.print(value[1]);
      SerialGSM.print(F("&"));
      
      SerialGSM.print(F("field3="));
      SerialGSM.print(value[2]);
      SerialGSM.print(F("&"));
      
      SerialGSM.print(F("field4="));
      SerialGSM.print(value[3]);
      SerialGSM.print(F("&"));
      
      SerialGSM.print(F("field5="));
      SerialGSM.print(value[4]);
      SerialGSM.print("&");

      SerialGSM.print(F("field6="));
      SerialGSM.print(value[5]);
      SerialGSM.print(F("&"));

      SerialGSM.print(F("field7="));
      SerialGSM.print(value[6]);
      SerialGSM.print(F("&"));

      SerialGSM.print(F("field8="));
      SerialGSM.println(value[7]);
      delay(500);
    }
  
    SerialGSM.println((char)26);
    delay(2000);
    //while(!SerialGSM.available());
    //while(SerialGSM.available())
    //{
      //Serial.write(SerialGSM.read());
    //}

    
    SerialGSM.println(F("AT+CIPCLOSE=?"));
    delay(100);
    SerialGSM.println(F("AT+CIPCLOSE=1"));
    delay(500);
    int i=0;
    while(!SerialGSM.available());
    while(SerialGSM.available())
    {
       response[i]=SerialGSM.read();
       i++;
    }
    strcpy_P(buffer, (char*)pgm_read_word(&(post_data_cmd_string[7]))); // PROGMEM "CLOSE OK"
    short int close_count=0;
    while(strstr(response, buffer)== NULL)
    {
       close_count++;
       SerialGSM.println(F("AT+CIPCLOSE=?"));
       delay(100);
       SerialGSM.println(F("AT+CIPCLOSE=1"));
       delay(200);
       i=0;
       while(!SerialGSM.available());
       while(SerialGSM.available())
       { 
       response[i]=SerialGSM.read();
       i++;
       }
       if(close_count==3)
       {
        break;
       }
    }
    memset(response, 0, sizeof(response));
    memset(buffer, 0, sizeof(buffer));
     
     SerialGSM.println(F("AT+CIPSHUT"));
     delay(100);
     while(!SerialGSM.available());
     while(SerialGSM.available())
     {
       response[i]=SerialGSM.read();
       i++;
     }
     strcpy_P(buffer, (char*)pgm_read_word(&(string_table[8]))); // PROGMEM "OK"
     close_count=0;
     while(strstr(response, buffer)== NULL)
     {
       close_count++;
       SerialGSM.println(F("AT+CIPSHUT"));
       delay(100);
       i=0;
       while(!SerialGSM.available());
       while(SerialGSM.available())
       { 
       response[i]=SerialGSM.read();
       i++;
       }
       if(close_count==3)
       {
        break;
       }
     }
    memset(response, 0, sizeof(response));
    memset(buffer, 0, sizeof(buffer));
     len = 0;
     retry=0;
     close_count=0;
     serialGW.begin(38400);
 }
}




void GW_processNodeMsg(unsigned int msgType, unsigned char *buff_p, long msgLen)
{
  long srcShortAddr, off = 0;
  unsigned long disMsgType;
  unsigned char *extAddr_p;

  GW_rcvdMsgCnt ++;


  if (msgLen < LPWMN_MAC_SHORT_ADDR_LEN
      + LPWMN_MAC_EXT_ADDR_LEN
      + LPWMN_MSG_RSSI_LEN
      + LPWMN_MSG_CORR_LQI_LEN)
    return;

  srcShortAddr = buff_p[off];
  srcShortAddr = (srcShortAddr << 8) | buff_p[off + 1];

  off += LPWMN_MAC_SHORT_ADDR_LEN;

  GW_print("[%lu] Received msg from node <%05lu / %02lx:%02lx:%02lx:%02lx:%02lx:%02lx:%02lx:%02lx> \r\n",
           GW_rcvdMsgCnt,
           srcShortAddr,
           (unsigned long)buff_p[off],
           (unsigned long)buff_p[off + 1],
           (unsigned long)buff_p[off + 2],
           (unsigned long)buff_p[off + 3],
           (unsigned long)buff_p[off + 4],
           (unsigned long)buff_p[off + 5],
           (unsigned long)buff_p[off + 6],
           (unsigned long)buff_p[off + 7]);

  mac_api(buff_p[off + 5], buff_p[off + 6], buff_p[off + 7]);
  extAddr_p = buff_p + off;
  buff_p += LPWMN_MAC_EXT_ADDR_LEN;

  {
    int rssi;
    unsigned int lqi_corr;

    rssi = (signed char)buff_p[off];
    lqi_corr = buff_p[off + 1];
    GW_print("RSSI %d dBm / LQI %u \r\n", (int)rssi, lqi_corr);
  }

  off += (LPWMN_MSG_RSSI_LEN + LPWMN_MSG_CORR_LQI_LEN);

  msgLen -= (LPWMN_MAC_SHORT_ADDR_LEN
             + LPWMN_MAC_EXT_ADDR_LEN
             + LPWMN_MSG_RSSI_LEN
             + LPWMN_MSG_CORR_LQI_LEN);

  if (msgLen < 1)
    return;

  disMsgType = buff_p[off];

  off += DIS_MSG_TYPE_SZ;
  msgLen -= DIS_MSG_TYPE_SZ;
  buff_p += off;

  if (msgLen <= 0xff)
  {
    unsigned char rc, tlvLen1, *buff1_p;



    rc = TLV_get(buff_p, msgLen, DIS_TLV_TYPE_SENSOR_OUTPUT_LIST, &tlvLen1, &buff1_p);
    if (rc == 0)
    {
      GW_print("Could not find DIS_TLV_TYPE_SENSOR_OUTPUT_LIST !! \r\n");
      return;
    }
    else
    {
      if (verbose)
        GW_print("Found DIS_TLV_TYPE_SENSOR_OUTPUT_LIST \r\n");

      while (1)
      {
        unsigned char tlvLen2, *buff2_p;

        rc = TLV_get(buff1_p, tlvLen1, DIS_TLV_TYPE_SENSOR_OUTPUT, &tlvLen2, &buff2_p);
        if (rc == 0)
        {
          if (verbose)
            GW_print("Could not find another DIS_TLV_TYPE_SENSOR_OUTPUT TLV !! \r\n");
          break;
        }
        else
        {
          unsigned char tlvLen3, *buff3_p;
          int snsrId, scaleFactor = DIS_DATA_SCALE_CENTI;


          buff1_p += (tlvLen2 + DIS_TLV_HDR_SZ);

          if (verbose)
            GW_print("Found DIS_TLV_TYPE_SENSOR_OUTPUT TLV .... val-fld-len<%d> \r\n", tlvLen2);

          rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_SENSOR_ID, &tlvLen3, &buff3_p);
          if (rc == 0)
            continue;
          else
          {
            if (tlvLen3 == DIS_SENSOR_ID_FIELD_SZ)
            {
              snsrId = *buff3_p;
              if (verbose)
                GW_print("Sensor Id <0x%x> \r\n", snsrId);
            }
            else
              continue;
          }

          rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_DATA_SCALE_FACTOR, &tlvLen3, &buff3_p);
          if (rc)
          {
            if (tlvLen3 == DIS_DATA_SCALE_FACTOR_FIELD_SZ)
            {
              scaleFactor = *buff3_p;
              if (verbose)
                GW_print("Found Scale factor <%d> \r\n", scaleFactor);
              if (!(scaleFactor >= DIS_DATA_SCALE_TERA && scaleFactor <= DIS_DATA_SCALE_FEMTO))
                scaleFactor = DIS_DATA_SCALE_NONE;
            }
          }

          rc = TLV_get(buff2_p, tlvLen2, DIS_TLV_TYPE_VALUE, &tlvLen3, &buff3_p);
          if (rc == 0)
            continue;
          else
          {
            long snsrOp;
            signed short snsrOp16;
            char *unit_p = " ";

            if (verbose)
              GW_print("Found DIS_TLV_TYPE_VALUE TLV .... val-fld-len<%d> \r\n", tlvLen3);

            switch (tlvLen3)
            {
              case 1:
                snsrOp = (int)(*buff3_p);
                break;

              case 2:
                {
                  snsrOp16 = GW_ntohs(buff3_p);
                  snsrOp = snsrOp16;
                }
                break;

              case 4:
                snsrOp = GW_ntohl(buff3_p);
                break;

              default:
                break;
            }
                float valF = snsrOp;

              switch (scaleFactor)
              {
                case DIS_DATA_SCALE_MICRO:
                  valF /= 1000;
                  valF /= 1000;
                  break;

                case DIS_DATA_SCALE_MILLI:
                  valF /= 1000;
                  break;


                case DIS_DATA_SCALE_TENTH_MILLI:
                  valF /= 1000;
                  valF /= 10;
                  break;

                case DIS_DATA_SCALE_CENTI:
                  valF /= 100;
                  break;

                case DIS_DATA_SCALE_DECI:
                  valF /= 10;
                  break;

                default:
                  break;
              }


            switch (snsrId)
            {
              case PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID:
                GW_print("+[Node_Voltage] ");
                unit_p = "Volts";
                scaleFactor = DIS_DATA_SCALE_MILLI;
                break;


              case PLTFRM_LM75B_1_DEV_ID:
                GW_print("+[Temp_LM75B]   ");
                unit_p = "Deg C";
                scaleFactor = DIS_DATA_SCALE_CENTI;
                break;

              case PLTFRM_CC2D33S_1_RH_DEV_ID:
                GW_print("+[RH_CC2D33S]   "); 
                unit_p = "NA";
                scaleFactor = DIS_DATA_SCALE_CENTI;
                break;

              case PLTFRM_CC2D33S_1_TEMP_DEV_ID:
                GW_print("+[Temp_CC2D33S] ");
                unit_p = "Deg C";
                scaleFactor = DIS_DATA_SCALE_CENTI;
                break;

              case PLTFRM_BMP280_1_PRESSURE_DEV_ID:
                GW_print("+[BMP_280_Pressure] ");
                unit_p = "mBar";
                scaleFactor = DIS_DATA_SCALE_CENTI;
                break;

              case PLTFRM_DS18B20_1_DEV_ID:
                GW_print("+[DS18B20] ");
                unit_p = "Deg C";
                scaleFactor = DIS_DATA_SCALE_TENTH_MILLI;
                break;

              case PLTFRM_BMP280_1_TEMPERATURE_DEV_ID:
                GW_print("+[Temp_BMP280] ");
                unit_p = "Deg C";
                scaleFactor = DIS_DATA_SCALE_CENTI;
                break;

              case PLTFRM_CHIRP_PWLA_1_DEV_ID:
                GW_print("+[Soil_Moisture] ");
                scaleFactor = DIS_DATA_SCALE_NONE;
                unit_p = "NA";
                break;

              case PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_ID:
                GW_print("+[Temp_MSP430]  ");
                unit_p = "deg C";
                break;

              default:
                GW_print("[Unknown]");
                break;
            }


            if (snsrId == PLTFRM_LM75B_1_DEV_ID
                || snsrId == PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID
                || snsrId == PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_ID
                || snsrId == PLTFRM_CC2D33S_1_RH_DEV_ID
                || snsrId == PLTFRM_CC2D33S_1_TEMP_DEV_ID
                || snsrId == PLTFRM_BMP280_1_TEMPERATURE_DEV_ID
                || snsrId == PLTFRM_BMP280_1_PRESSURE_DEV_ID
                || snsrId == PLTFRM_CHIRP_PWLA_1_DEV_ID
                || snsrId == PLTFRM_DS18B20_1_DEV_ID)
            {
             
              Serial.println(valF);

              GW_sendDataToCloud(snsrId, valF);
            }

          }
        }
      }
    }
  }

  return;
}


void GW_processRcvsMsg(unsigned int msgType, unsigned char *pyld_p, long pyldLen)
{
  Serial.print("---------------------------------------------------------------------------- \r\n");

  switch (msgType)
  {
    case LPWMN_GW_MSG_TYPE_EVENT:
    case LPWMN_GW_MSG_TYPE_RELAY_FROM_NODE:
      {
        GW_processNodeMsg(msgType, pyld_p, pyldLen);
      }
      break;

    default:
      break;
  }

  Serial.print("---------------------------------------------------------------------------- \r\n");
  return;
}



void GW_procRcvdByte(unsigned char rxByte)
{
  GW_uartRxBuff[GW_uartRxCnt] = rxByte;

  // GW_print("%02d:%02x\r\n", GW_uartRxCnt, rxByte);

  GW_uartRxCnt ++;

  if (GW_uartRxCnt == UART_FRAME_HDR_PYLD_CRC_FIELD_OFF)
  {
    unsigned short calcCrc16, rxdCrc16;

    //GW_print("Read %d bytes \r\n", GW_uartRxCnt);

    calcCrc16 = __crc16(GW_uartRxBuff, UART_FRAME_HDR_HDR_CRC_FIELD_OFF);
    rxdCrc16 = GW_uartRxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF];
    rxdCrc16 = (rxdCrc16 << 8) + (byte)GW_uartRxBuff[UART_FRAME_HDR_HDR_CRC_FIELD_OFF + 1];

    if (calcCrc16 != rxdCrc16)
    {
      int idx;
      GW_print("Hdr CRC mismatch <0x%x/0x%x> !!  \r\n", rxdCrc16, calcCrc16);
      for (idx = 0; idx < UART_FRAME_HDR_PYLD_CRC_FIELD_OFF - 1; idx++)
        GW_uartRxBuff[idx] = GW_uartRxBuff[idx + 1];
      GW_uartRxCnt = UART_FRAME_HDR_PYLD_CRC_FIELD_OFF - 1;
    }
    else
    {
      GW_print("Hdr CRC matches <0x%x/0x%x> \r\n", rxdCrc16, calcCrc16);
      GW_msgPyldLen = GW_uartRxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF];
      GW_msgPyldLen += (GW_msgPyldLen << 8) |  GW_uartRxBuff[UART_FRAME_HDR_PYLD_LEN_FIELD_OFF + 1];
      GW_msgTotLen = UART_FRAME_HDR_LEN + GW_msgPyldLen;
      GW_print("msg payload length<%d> / total-length<%d> \r\n", GW_msgPyldLen, GW_msgTotLen);
    }
  }
  else
  {
    if (GW_uartRxCnt == GW_msgTotLen)
    {
      unsigned short calcPyldCrc16, rxdPyldCrc16;
      unsigned int currMsgType;

      // Payload received ...
      GW_print("msg with total length<%d> received  ... \r\n",  GW_msgTotLen);
      if (GW_msgPyldLen > 0)
      {
        calcPyldCrc16 = __crc16(GW_uartRxBuff + UART_FRAME_HDR_LEN, GW_msgPyldLen);
        rxdPyldCrc16 = GW_uartRxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF];
        rxdPyldCrc16 = (rxdPyldCrc16 << 8) + (byte)GW_uartRxBuff[UART_FRAME_HDR_PYLD_CRC_FIELD_OFF + 1];
      }
      GW_print("<0x%x/0x%x> Payload CRC %s \r\n",
               rxdPyldCrc16, calcPyldCrc16, calcPyldCrc16 != rxdPyldCrc16 ? "mismatch !!" : "match");

      currMsgType = GW_uartRxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF];
      currMsgType = (currMsgType << 8) | GW_uartRxBuff[UART_FRAME_HDR_MSG_TYPE_FIELD_OFF + 1];


      GW_processRcvsMsg(currMsgType, GW_uartRxBuff + UART_FRAME_HDR_LEN, GW_msgPyldLen);
      memset(GW_uartRxBuff, 0 , sizeof(GW_uartRxBuff));
      GW_uartRxCnt = 0;
    }
  }
}



void setup()
{
  Serial.begin(38400);
  serialGW.begin(38400);
  SerialGSM.begin(9600);
  Serial.println(F("Setup Dec4 8:15pm"));
  //Serial.write(pgm_read_byte_near(FIELD8),strlen_P(FIELD8));
  //Serial.write(FIELD8,strlen_P(FIELD8));

}

void loop()
{
 // valCheck = 0; //maximum val will be no: of sensors in the node

  if(serialGW.available()> 0)
  {

    int inByte = serialGW.read();
    //Serial.println(inByte);
    GW_procRcvdByte(inByte);
    //sleepFlag=0;
  }

}
