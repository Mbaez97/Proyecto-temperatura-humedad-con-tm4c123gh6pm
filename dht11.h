/*
 * dht11.h
 *
 *  Created on: Dec 14, 2020
 *      Author: marcelobaez
 */

#ifndef DHT11_H_
#define DHT11_H_
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//#include "inc/tm4c123gh6pm.h"
/////////////////////////////////////////////////////VERSION2/////////////////////////////////////////////////////////////////////////
#include "inc/tm4c123gh6pm.h"
//#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"
#define PORTD   0x40007000
#define DHT11_PIN   (*((volatile unsigned long *)(PORTD|(0x08<<2))))  //el pin PD3


struct dht11_T{
    unsigned char temperaturad;
    unsigned char temperatura;
    unsigned char umidaded;
    unsigned char umidade;
};
extern struct dht11_T sensor;
unsigned long DHT11_In( unsigned long * ptrValor);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////VERSION1/////////////////////////////////////////////////////////////////////////
//void GPIO_PORTE_Init(void);     //Port E init
//void DHT11_Init(void); //DHT11 init
//void DHT11_Master_Request(void);  //Master request
//int DHT11_Slave_Response(void); //Slave Response
//char DHT11_Rx_8_Data(void); //Receive Data
//int DHT11_Verify_Parity(int ); //Verify Parity
void delayMs(uint32_t ui32Ms);
void delayUs(uint32_t ui32Us);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION3/////////////////////////////////////////////////////////////////////////
//#define MCU_CLOCK SysCtlClockGet()
//
//#define DHT_WAIT_18ms ((MCU_CLOCK*18)/3000)
//#define DHT_WAIT_20us ((MCU_CLOCK*2)/300000)
//
//#define DHT_TIMEOUT    ((MCU_CLOCK*9)/100000) // 90us
//#define DHT_TIME_BIT_1 ((MCU_CLOCK*7)/100000) // 70us
//
//
//typedef struct {
//    float celsius_temp;
//    float humidity;
//} DHT_TypeDef;
//
//
//// Interface
//extern void DHT_init(void);
//extern uint8_t dht_readTH(DHT_TypeDef *);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////VERSION4/////////////////////////////////////////////////////////////////////////
//#include <stdint.h>
//#include <string.h>
//#include <stdbool.h>
//
//#include "inc/tm4c123gh6pm.h"

#define DATA (*((volatile uint32_t *)(0x42000000 + (0x400243FC-0x40000000)*32 + 3*4))) //PE3

#define LOW 3200  //  LOW x = Clock speed * 80 millseconds ; TODO : Need to be generic based on system clock i.e  Here x is 3200 ( = 40 MHz clock * 80 millisconds)
#define DATABITS 40
#define SETUPBITS 3
#define RESOLUTION 16

#define INTHMDSTRT 0
#define INTHMDEND 7
#define DECHMDSTRT 8
#define DECHMDEND 15
#define INTTEMPSTRT 16
#define INTTEMPEND 23
#define DECTEMPSTRT 24
#define DECTEMPEND 31
#define PARITYSTRT 32
#define PARITYEND 39

float temperature;
float humidity;
void initDHT11Hw ();
void initWTimer1A ();
void initSampler ();
void readSensordata ();
uint8_t getReading ();
void gpioEISR ();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif /* DHT11_H_ */
