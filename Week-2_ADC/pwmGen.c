//************************************************************************
//
// pwmGen.c - Example code which generates a single PWM output
//            with duty cycle controlled by UP and DOWN buttons.
//
// Author:  P.J. Bones	UCECE
// Last modified:	29.4.2015
//
//************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "buttonSet.h"
#include "button.h"

#include "drivers/rit128x96x4.h" // added for Display
#include "utils/ustdlib.h"

//************************************************************************
// Generates a single PWM signal at 100 Hz on PD1/PWM1: Pin 53
// Note that PWM Generator 0 drives output 'PWM1' (Ref LMS1968 datasheet)
//************************************************************************

// Constants
#define SYSTICK_RATE_HZ 1000ul
#define PWM1_RATE_HZ 150
#define PWM_DIV_CODE SYSCTL_PWMDIV_4
#define PWM_DIVIDER 4
#define PWM1_DEF_DUTY 50

//*******************************************************************
// ISR for the SysTick interrupt (used for button debouncing).
//*******************************************************************
void
SysTickIntHandler (void)
{
	//
	// Poll the buttons
	updateButtons();
}

//*******************************************************************
// Initialise the clock
//*******************************************************************
void
initClock (void)
{
    //
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                    SYSCTL_XTAL_8MHZ);
}

//*******************************************************************
// Initialise the SysTick interrupts
//*******************************************************************
void
initSysTick (void)
{
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet (SysCtlClockGet() / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister (SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable ();
    SysTickEnable ();
}

//******************************************************************
// Initialise the GPIO for the PWM output (Port D)
//******************************************************************
void
initPin (void)
{
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM (GPIO_PORTF_BASE, GPIO_PIN_2);
}

//******************************************************************
// Initialise the PWM generator (PWM1) 
//******************************************************************
void
initPWMchan (void)
{
	unsigned long period;

    SysCtlPeripheralEnable (SYSCTL_PERIPH_PWM);
    //
    // Compute the PWM period based on the system clock. 
    //
        SysCtlPWMClockSet (PWM_DIV_CODE);
    PWMGenConfigure (PWM_BASE, PWM_GEN_2,
        PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    period = SysCtlClockGet () / PWM_DIVIDER / PWM1_RATE_HZ;
    PWMGenPeriodSet (PWM_BASE, PWM_GEN_2, period);
    PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * PWM1_DEF_DUTY / 100);
    //
    // Enable the PWM output signal.
    //
    PWMOutputState (PWM_BASE, PWM_OUT_4_BIT, true);
    //
    // Enable the PWM generator.
    //
    PWMGenEnable (PWM_BASE, PWM_GEN_2);
}
void
initDisplay (void)
{
  // intialise the OLED display
  RIT128x96x4Init(1000000);
}

void
display(int freq, int duty)
{
	char string[40];

    RIT128x96x4StringDraw("M1.8", 5, 24, 15);
	usprintf(string, "Duty Cycle = %5d", duty);
    RIT128x96x4StringDraw(string, 5, 44, 15);
	usprintf(string, "Frequency = %7d", freq);
    RIT128x96x4StringDraw(string, 5, 54, 15);
    usprintf(string, "%7d", SysTickValueGet());
    RIT128x96x4StringDraw(string, 5, 64, 15);
}

/*
void
initTimer()
{
	TimerConfigure(TIMER0_BASE, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT |
	TIMER_CFG_B_CAP_COUNT));
}
*/


int
main (void)
{
    unsigned int duty = PWM1_DEF_DUTY;
    unsigned long period;
    int freq = 71; // PLACEHOLDER
    initClock ();
    initPin ();
    initButSet (UP_B | DOWN_B, SYSTICK_RATE_HZ);
    initPWMchan ();
    initSysTick ();

    initDisplay(); //added in for m1.8

    //
    // Enable interrupts to the processor.
    IntMasterEnable();
    //
    // Compute the PWM period in terms of the PWM clock
    period = SysCtlClockGet () / PWM_DIVIDER / PWM1_RATE_HZ;
    //
    // Loop forever while controlling the PWM duty cycle.
    while (1)
    {
       // Background task: Check for button pushes and control the
       //  PWM duty cycle +/- 5% within the range 5% to 95%.
       if (checkBut (UP) && duty < 95)
       {
    	  duty += 5;
          PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * duty /100);
       }
       if (checkBut (DOWN) && duty > 5)
       {
    	  duty -= 5;
          PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * duty /100);
       }
       display(freq, duty);
    }
}
