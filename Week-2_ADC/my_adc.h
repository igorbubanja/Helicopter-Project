// This is the header file for your first project example.
// For details on the purpose of this program, refer to my_adc.c

//! File: my_adc.h
//! Author: Dr. Steve Weddell
//! ECE, University of Canterbury
//! January 31, 2012
//! Data acquisition example for ENCE361

typedef unsigned char uint8_t;
typedef uint8_t uint_8; 

#define uint16_t unsigned short
#define uint32_t unsigned long

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "drivers/rit128x96x4.h"

#define bufsize 40
