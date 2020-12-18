/*
 * include.h
 *
 *  Created on: Dec 18, 2020
 *      Author: marcelobaez
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

//thu vien driver API

#include "driverlib/gpio.h"
#include "driverlib/can.h"
#include "driverlib/eeprom.h"
#include "driverlib/i2c.h"
#include "driverlib/lcd.h"

#include "driverlib/mpu.h"
#include "driverlib/pwm.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/qei.h"
#include "driverlib/fpu.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"

//thu vien ho tro phan cung
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_timer.h"
#include "utils/uartstdio.h"
#include "inc/hw_nvic.h"
//thu vien khai bao them
#include "timer.h"
#include "dht11.h"

//DHT_TypeDef th;

extern bool after2s;

#endif /* INCLUDE_H_ */
