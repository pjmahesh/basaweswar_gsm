/*
 * File Name: pltfrm.h
 * Author: ram krishnan (rkris@wisense.in)
 * Created: Nov/16/2017
 *
 *
 * Copyright (c) <2018>, <ram krishnan>
 * All rights reserved.
 *
 * File cannot be copied and/or distributed without the express
 * permission of the author.
 */

#ifndef __PLTFRM_H__
#define __PLTFRM_H__


// --------------------- Devices --------------------------


// Serial EEPROM
#define PLTFRM_AT24C01C_1_DEV_ID    0x1           // I2C
#define PLTFRM_AT24C01C_1_I2C_ADDR  0x0  

// Serial EEPROM with EUI-64 and 128 bit serial number
#define PLTFRM_AT24MAC602_DEV_CNT  1
#define PLTFRM_AT24MAC602_1_DEV_ID    0x2
#define PLTFRM_AT24MAC602_1_I2C_ADDR  0x0
   
// Temperature sensors   
#define PLTFRM_LM75B_1_DEV_ID       0x9            // I2C
#define PLTFRM_LM75B_1_I2C_ADDR     0x0
#define PLTFRM_LMP75B_DEV_CNT       0x1            // Number of such devices in the system


// #define PLTFRM_NTC_THERMISTOR_1_DEV_ID   0xb       // Vishay - NTCALUG02A (analog)
// #define PLTFRM_NTC_THERMISTOR_2_DEV_ID   0xc       // Vishay - NTCALUG02A (analog)





// RADIO
#define PLTFRM_CC2520_1_DEV_ID      0x19           // SPI
#define PLTFRM_CC1200_1_DEV_ID      0x1a           // SPI
#define PLTFRM_CC1101_1_DEV_ID      0x1b           // SPI


// LEDs
#define PLTFRM_LED_1_DEV_ID           0x40         // GPIO
#define PLTFRM_LED_2_DEV_ID           0x41         // GPIO



#define PLTFRM_DS18B20_1_DEV_ID     0x6c   // 1 - wire
#define PLTFRM_DS18B20_DEV_CNT      0x1

#define PLTFRM_BMP280_1_DEV_ID  0xbc  //  I2C
#define PLTFRM_BMP280_1_PRESSURE_DEV_ID  0xbc  //  I2C
#define PLTFRM_BMP280_1_TEMPERATURE_DEV_ID  0xbd  //  I2C


#define PLTFRM_ON_CHIP_VCC_SENSOR_DEV_ID    0x78      // ADC_10 channel
#define PLTFRM_ON_CHIP_VCC_SENSOR_DEV_CNT   0x1

#define PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_ID    0x79      // ADC_10 channel
#define PLTFRM_ON_CHIP_TEMP_SENSOR_DEV_CNT   0x1

#define PLTFRM_CHIRP_PWLA_1_DEV_ID  0x91
#define PLTFRM_CHIRP_PWLA_DEV_CNT 1


#define PLTFRM_CC2D33S_1_RH_DEV_ID      0xb0  // I2C
#define PLTFRM_CC2D33S_1_TEMP_DEV_ID    0xb1  // I2C

#define PLTFRM_I2C_SW_BUS_1_DEV_ID  0xe0
#define PLTFRM_I2C_SW_BUS_2_DEV_ID  0xe1

#define PLFRM_SPI_HW_BUS_1_DEV_ID   0xe8

#define PLTFRM_WS_VEH_DET_1_DEV_ID  0xea


#define PLTFRM_WPDS_DEV_ID  0xfb
#define PLTFRM_32KHZ_CRYSTAL_DEV_ID  0xfe



// --------------------------------------------------------- 
   
   
// ------------- Buses (I2C, SPI, 1-WIRE etc) --------------   
// I2C (software)
#define PLTFRM_I2C_SW_BUS_1_ID      0x80

   
// I2C (hardware) 
#define PLFRM_I2C_HW_BUS_1_ID      0x88

#define PLTFRM_INV_I2C_BUS_ID      0xff
#define PLTFRM_INV_I2C_ADDR        0xff
 
// SPI (hardware)
#define PLFRM_SPI_HW_BUS_1_ID      0x90

// 1 WIRE (software)
#define PLTFRM_1WIRE_SW_BUS_1_ID   0x98
   
// UART
#define PLTFRM_UART_HW_1_DEV_ID    0xa0
// ---------------------------------------------------------   


// ---------------------------------------------------------

typedef enum
{
   PLTFRM_GPIO_PORT_1 = 0,
   PLTFRM_GPIO_PORT_2 = 1,
   PLTFRM_GPIO_PORT_3 = 2,
   PLTFRM_GPIO_PORT_4 = 3,

} PLTFRM_gpioPortId_t;


typedef enum
{
   PLTFRM_GPIO_PORT_PIN_0,
   PLTFRM_GPIO_PORT_PIN_1,
   PLTFRM_GPIO_PORT_PIN_2,
   PLTFRM_GPIO_PORT_PIN_3,
   PLTFRM_GPIO_PORT_PIN_4,
   PLTFRM_GPIO_PORT_PIN_5,
   PLTFRM_GPIO_PORT_PIN_6,
   PLTFRM_GPIO_PORT_PIN_7
} PLTFRM_gpioPortPin_t;

#endif

