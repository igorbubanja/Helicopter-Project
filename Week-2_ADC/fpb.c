//*****************************************************************************
//
// pulseInrpt.c - Program to monitor the width of pulse in a digital waveform.
//
// Author:  P.J. Bones	UCECE
// Last modified:	17.3.2015
//
//*****************************************************************************
// Lab 3 sheet for ENCE361 2012: Step 6. Write an interrupt service routine to 
// read the SysTick register operating as a decrementing wrap-on-zero counter, 
// using SysTickValueGet()  for every 0->1  or 1->0  transition on an EVK header 
// pin (Pin 27 = PF5).  Maintain a circular buffer of size NUM_TICKS
// with NUM_TICKS = 20, say. Produce a continually updating display 
// of the mean pulse duration in msec over the contents of the buffer.  
// Remember that the 24-bit counter value will wrap once it reaches zero.
//*****************************************************************************

//************************************************************************

//

// pwmGen.c - Example code which generates a single PWM output

//            with duty cycle controlled by UP and DOWN buttons.

//

// Author:  peizhao,Qiu

// Last modified: 2.4.2016

//

//************************************************************************

#include "inc/hw_memmap.h"

#include "inc/hw_types.h"

#include "inc/hw_ints.h"

#include "driverlib/adc.h"

#include "driverlib/debug.h"

#include "driverlib/pwm.h"

#include "driverlib/gpio.h"

#include "driverlib/pwm.h"

#include "driverlib/systick.h"

#include "driverlib/sysctl.h"

#include "driverlib/interrupt.h"

#include "drivers/rit128x96x4.h"

#include "buttonSet.h"

#include "button.h"

#include "stdio.h"

#include "stdlib.h"

//*****************************************************************************

// Constants

//*****************************************************************************

#define MAX_24BIT_VAL 0X0FFFFFF

#define SYSTICK_RATE_HZ 1000ul

#define PWM4_RATE_HZ 150

#define PWM_DIV_CODE SYSCTL_PWMDIV_4

#define PWM_DIVIDER 4

#define PWM4_DEF_DUTY 50

//*****************************************************************************

// Global variables

//*****************************************************************************

volatile static unsigned long g_ulIntCnt; // Monitors interrupts

unsigned long frequence;

unsigned long last_count;

//*****************************************************************************

//

// The interrupt handler for the for the pin change interrupt. Note that

//  the SysTick counter is decrementing.

//

//*****************************************************************************

void

PinChangeIntHandler(void)

{

//

// Clear the interrupt (documentation recommends doing this early)

	GPIOPinIntClear(GPIO_PORTF_BASE, GPIO_PIN_5);

//

// Count interrupts

	g_ulIntCnt++;

}

//*******************************************************************

// ISR for the SysTick interrupt (used for button debouncing).

//*******************************************************************

void

SysTickIntHandler(void)

{

	updateButtons();

	unsigned long current_count;

	current_count = g_ulIntCnt;

	frequence = ((current_count - last_count) * 289 * 200 / 460 / 210); //500+g_ulIntCnt;

	last_count = current_count;

	updateButtons();

}

//*****************************************************************************

// Initialisation functions: clock, GPIO pin, display, buffer

//*****************************************************************************

void

initClock(void)

{

	//

	// Set the clock rate @ 3125000 Hz (minimum possible). The wrap-around

	//  period is then 5.36871 sec.

	SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |

	SYSCTL_XTAL_8MHZ);

	//

	// Set up the period for the SysTick timer to get the maximum span.

	SysTickPeriodSet(MAX_24BIT_VAL);

	// Register the interrupt handler

	SysTickIntRegister(SysTickIntHandler);

	//

	// Enable interrupt and device

	SysTickIntEnable();

	SysTickEnable();

}

// *******************************************************

void

initPin(void)

{

	//

	// Register the handler for Port F into the vector table

	GPIOPortIntRegister(GPIO_PORTF_BASE, PinChangeIntHandler);

	//

	// Enable and configure the port and pin used:  input on PF5: Pin 27 and out put

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_5, GPIO_STRENGTH_2MA,

	GPIO_PIN_TYPE_STD_WPU);

	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);

	//

	// Set up the pin change interrupt (both edges)

	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_5, GPIO_BOTH_EDGES);

	//

	// Enable the pin change interrupt

	GPIOPinIntEnable(GPIO_PORTF_BASE, GPIO_PIN_5);

	IntEnable(INT_GPIOF); // Note: INT_GPIOF defined in inc/hw_ints.h

}

// *******************************************************

void

initDisplay(void)

{

	// intialise the OLED display

	RIT128x96x4Init(1000000);

}

//*****************************************************************************

//

// Function to display the mean interval in usec

//

//*****************************************************************************

void

display(long frequence, long duty)

{

	char string[30];

	RIT128x96x4StringDraw("Monitoring Pin 27", 5, 24, 15);

	RIT128x96x4StringDraw("frequence = ", 5, 34, 15);

	sprintf(string, "%8d HZ", frequence);

	RIT128x96x4StringDraw(string, 5, 44, 15);

	RIT128x96x4StringDraw("duty = ", 5, 54, 15);

	sprintf(string, "%8d per", duty);

	RIT128x96x4StringDraw(string, 5, 64, 15);

}

//*****************************************************************************

//

// Function to display the interrupt count

//*****************************************************************************

void

displayIntCnt(void)

{

	char string[30];

	sprintf(string, "Count = %d", g_ulIntCnt);

	RIT128x96x4StringDraw(string, 5, 74, 15);

}

//*****************************************************************************

//******************************************************************

// Initialise the PWM generator (PWM4)

//******************************************************************

void

initPWMchan(void)

{

	unsigned long period;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);

	//

	// Compute the PWM period based on the system clock.

	//

	SysCtlPWMClockSet(PWM_DIV_CODE);

	PWMGenConfigure(PWM_BASE, PWM_GEN_2,

	PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	period = SysCtlClockGet() / PWM_DIVIDER / PWM4_RATE_HZ;

	PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, period);

	PWMPulseWidthSet(PWM_BASE, PWM_OUT_4, period * PWM4_DEF_DUTY / 100);

	//

	// Enable the PWM output signal.

	//

	PWMOutputState(PWM_BASE, PWM_OUT_4_BIT, true);

	//

	// Enable the PWM generator.

	//

	PWMGenEnable(PWM_BASE, PWM_GEN_2);

}

int

main(void)

{

	unsigned long period;

	unsigned int duty = PWM4_DEF_DUTY;

	last_count = 0;

	initClock();

	initPin();

	initDisplay();

	initPWMchan();

	initButSet(UP_B | DOWN_B, SYSTICK_RATE_HZ);

//

// Enable interrupts to the processor.

	IntMasterEnable();

	while (1)

	{

		updateButtons();

//  check the input frequence if it was not in the range change the frequence

		if (frequence > 300 || frequence < 100)

		{

			frequence = 150;

		}

		period = SysCtlClockGet() / frequence / PWM_DIVIDER / PWM4_RATE_HZ* 150;


		updateButtons();

// Background task: Check for button pushes and control the

//  PWM duty cycle +/- 5% within the range 5% to 95%.

		if (checkBut(UP) && duty < 95)

		{

			duty += 5;

			PWMPulseWidthSet(PWM_BASE, PWM_OUT_4, period * duty / 100);

		}

		updateButtons();

		if (checkBut(DOWN) && duty > 5)

		{

			duty -= 5;

			PWMPulseWidthSet(PWM_BASE, PWM_OUT_4, period * duty / 100);

		}

		updateButtons();

		PWMGenPeriodSet(PWM_BASE, PWM_GEN_2, period);

		display(frequence, duty);

		displayIntCnt();

		updateButtons();

	}

}
