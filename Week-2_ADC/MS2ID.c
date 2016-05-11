//*********************************
// MileStone 2
// Author: Bubanja Igor, Kim Danny
//*********************************

#include "buttonSet.h"
#include "button.h"
#include "stdio.h"
#include "stdlib.h"
#include "circBuf.h"
#include "utils/ustdlib.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/adc.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "drivers/rit128x96x4.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE 200
#define SAMPLE_RATE_HZ 10000

//*****************************************************************************
// Global variables
//*****************************************************************************
long degrees;
long yaw;
int previous_state;
static circBuf_t g_inBuffer;		// Buffer of size BUF_SIZE integers (sample values)
static unsigned long g_ulSampCnt;	// Counter for the interrupts
//*****************************************************************************
// The interrupt handler for the for the pin change interrupt. Note that
// the SysTick counter is decrementing.
//*****************************************************************************

void PinChangeIntHandler (void)
{
int current_state;
unsigned long A;
unsigned long B;

//
// Clear the interrupt (documentation recommends doing this early)

GPIOPinIntClear (GPIO_PORTF_BASE, GPIO_PIN_5|GPIO_PIN_7);
A= GPIOPinRead (GPIO_PORTF_BASE, GPIO_PIN_5);
B= GPIOPinRead (GPIO_PORTF_BASE, GPIO_PIN_7);

if (!A && !B) {
	current_state = 00;
}

if (!A && B) {
	current_state = 01;
}

if (A && B) {
	current_state = 11;
}

if (A && !B) {
	current_state = 10;
}

if (previous_state == 10) {
	if (current_state == 00) {
		yaw++;
	}
    if (current_state == 11) {
    	yaw--;
    }
}

if (previous_state == 00) {
	if (current_state == 01) {
		yaw++;
	}
	if (current_state == 10) {
		yaw--;
	}
}

if (previous_state == 01) {
	if (current_state == 11) {
		yaw++;
	}
    if (current_state == 00) {
    	yaw--;
    }
}

if (previous_state == 11) {
	if (current_state == 10) {
		yaw++;
	}
    if (current_state == 01) {
    	yaw--; }
}

previous_state = current_state;
degrees = yaw * 360 / 448;

}

// *******************************************************
void
SysTickIntHandler(void)
{
    //
    // Initiate a conversion
    //
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}
// *******************************************************
void
ADCIntHandler(void)
{
	unsigned long ulValue;

	//
	// Get the single sample from ADC0. (Yes, I know, a function call!!)
	ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
	//
	// Place it in the circular buffer (advancing write index)
	g_inBuffer.data[g_inBuffer.windex] = (int) ulValue;
	g_inBuffer.windex++;
	if (g_inBuffer.windex >= g_inBuffer.size)
		g_inBuffer.windex = 0;
	//
	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);
}
// *******************************************************
void
initClock (void)
{
  //
  // Set the clock rate. From Section 19.1 in stellaris_peripheral_lib_UG.doc:
  //  "In order to use the ADC, the PLL must be used; the PLL output will be
  //  used to create the clock required by the ADC." ADC rate = 8 MHz / 10.
  //  The processor clock rate = 20 MHz.
  SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);
  //
  // Set up the period for the SysTick timer.  The SysTick timer period is
  // set as a function of the system clock.
  SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
  //
  // Register the interrupt handler
  SysTickIntRegister(SysTickIntHandler);
  //
  // Enable interrupt and device
  SysTickIntEnable();
  SysTickEnable();
}
// *******************************************************
void
initADC (void)
{
  //
  // The ADC0 peripheral must be enabled for configuration and use.
  SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

  // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
  // will do a single sample when the processor sends a signal to start the
  // conversion.
  ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

  //
  // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
  // single-ended mode (default) and configure the interrupt flag
  // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
  // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
  // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
  // sequence 0 has 8 programmable steps.  Since we are only doing a single
  // conversion using sequence 3 we will only configure step 0.  For more
  // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
  ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                             ADC_CTL_END);

  //
  // Since sample sequence 3 is now configured, it must be enabled.
  ADCSequenceEnable(ADC0_BASE, 3);

  //
  // Register the interrupt handler
  ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

  //
  // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
  ADCIntEnable(ADC0_BASE, 3);
}
// *******************************************************
void
initPin (void)
{
    //
    // Register the handler for Port F into the vector table
    GPIOPortIntRegister (GPIO_PORTF_BASE, PinChangeIntHandler);
    //
    // Enable and configure the port and pin used:  input on PF5: Pin 27 and out put
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPadConfigSet (GPIO_PORTF_BASE, GPIO_PIN_5|GPIO_PIN_7, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);

    //
    // Set up the pin change interrupt (both edges)
    GPIOIntTypeSet (GPIO_PORTF_BASE, GPIO_PIN_5|GPIO_PIN_7, GPIO_BOTH_EDGES);
    //
    // Enable the pin change interrupt
    GPIOPinIntEnable (GPIO_PORTF_BASE, GPIO_PIN_5|GPIO_PIN_7);
    IntEnable (INT_GPIOF);	// Note: INT_GPIOF defined in inc/hw_ints.h
}
//*****************************************************************************
// Function to initalize the display
//*****************************************************************************
void
initDisplay (void)
{
  // intialise the OLED display
  RIT128x96x4Init(1000000);
}
//*****************************************************************************
// Function to display
//*****************************************************************************
void
display (long yaw, long altitude, long percentage)
{
	char string[30];

	RIT128x96x4StringDraw("Milestone 2", 5, 24, 15);
	usprintf(string, "yaw = %d  ", yaw);
	RIT128x96x4StringDraw(string, 5, 34, 15);
	usprintf(string, "altitude = %d  ", altitude);
	RIT128x96x4StringDraw(string, 5, 44, 15);
	usprintf(string, "percentage = %d  ", percentage);
	RIT128x96x4StringDraw(string, 5, 54, 15);
}
//*****************************************************************************
int main(void)
{
	yaw = 0;
	previous_state = 0;
	initPin ();
	initDisplay ();
	initClock ();
	initADC ();
	initCircBuf (&g_inBuffer, BUF_SIZE);
	unsigned int i;
	unsigned int value = 0;
    int flag = 0;
    long maxa; // Max Altitude
    long percentage;
    long altitude;
    long difference;
    difference = 341; //corresponds to 1V

	IntMasterEnable (); // Enable interrupts to the processor.


	while (1)
	{
		int sum = 0;

		for (i = 0; i < BUF_SIZE; i++)
		{
				value = readCircBuf (&g_inBuffer);
				sum = sum + value;
		}

		long avg = sum / BUF_SIZE;
			if(flag == 0 || avg > maxa)
			{
				maxa = avg;
				flag = 1;
			}

			altitude = maxa - ((2 * sum + BUF_SIZE) / (2 * BUF_SIZE));
			percentage = 100 + (((altitude - difference) * 100) / difference);

			display (degrees, altitude, percentage);
		}
}
