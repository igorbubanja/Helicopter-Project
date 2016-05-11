//*****************************************************************************
//
// single_ended.c - Example demonstrating how to configure the ADC for
// single ended operation.
//
// Copyright (c) 2010-2011 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 6852 of the Stellaris Firmware Development Package.
//
//*****************************************************************************

#include "my_adc.h"  // header file for my_adc.c

//Function prototypes
void intialise(void);
void updateDisplay(uint16_t);

// Global variables
char strBuf[bufsize];

//*****************************************************************************
//
//! This example is designed as an introduction to your
//! "voltaliser" project. In your first lab you will compile, link,
//! and program your EVK-LM3S1968 dev board to acquire
//! a voltage signal and show the equivilant binary code
//! on the OLED display. 
//!
//! This example shows how to setup ADC0 as a single ended input and take a
//! single sample on ADC0/B1.
//!
//! This example uses the following peripherals and I/O signals.
//! - ADC0 peripheral (Pin 63 on your board header)
//! - GND (Pin 58 on your board header)

//! Author: Dr. Steve Weddell
//! ECE, University of Canterbury
//! January 31, 2012
//! Data acquisition example for ENCE361
//!

void initialise(void)
{
  SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);	

  //
  // The ADC0 peripheral must be enabled for use.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
  
  //
  // For this example ADC0 is used with AIN0 on port B1. This
  // was given by the LM3S1968 data sheet.
  // Therefore, GPIO port B needs to be enabled
  // so these pins can be used.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  
  
  // Select the analog ADC function for these pins.
  // The LM3S1968 data sheet was consulted to see which functions are 
  // allocated per pin.
  GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_1); 
  
  // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
  // will do a single sample when the processor sends a signal to start the
  // conversion.  Each ADC module has 4 programmable sequences, sequence 0
  // to sequence 3.  This example is arbitrarily using sequence 3.
  ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  
  //
  // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
  // single-ended mode (default) and configure the interrupt flag
  // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
  // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
  // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
  // sequence 0 has 8 programmable steps.  Since we are only doing a single
  // conversion using sequence 3 we will only configure step 0.  For more
  // information on the ADC sequences and steps, reference the datasheet.
  ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);    
                             
    //
  // Since sample sequence 3 is now configured, it must be enabled.
  ADCSequenceEnable(ADC0_BASE, 3);     
  
  //
  // Clear the interrupt status flag.  This is done to make sure the
  // interrupt flag is cleared before we sample.
  ADCIntClear(ADC0_BASE, 3);                          
	
	
  // intialise the OLED display
  RIT128x96x4Init(1000000);	
}

void updateDisplay(uint16_t iValue)
{
	
     usprintf(strBuf, "Value ADC0: %u", iValue); // try %d
	 
     RIT128x96x4Clear(); // gets rid of the residual digits
     RIT128x96x4StringDraw( strBuf, 30, 50, 15); 	
}


int
main(void)
{
	uint16_t uiValue = 10;
	unsigned long ulValue[1];
    
    initialise();

    
    while(1)
    {
        //
        // Trigger the ADC conversion.
        ADCProcessorTrigger(ADC0_BASE, 3);  
        
        //
        // Wait for conversion to be completed.
        while(!ADCIntStatus(ADC0_BASE, 3, false))  
        {
        }            
  
        // Read ADC Value.
        ADCSequenceDataGet(ADC0_BASE, 3, ulValue);
        uiValue = (unsigned int) ulValue[0];
        
        updateDisplay(uiValue);
        
 		SysCtlDelay(SysCtlClockGet() / 12);
    }
}
