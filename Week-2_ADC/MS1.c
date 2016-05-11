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

#include "inc/hw_ints.h" //from pulseinrpt

//************************************************************************
// Generates a single PWM signal at 100 Hz on PD1/PWM1: Pin 53
// Note that PWM Generator 0 drives output 'PWM1' (Ref LMS1968 datasheet)
//************************************************************************

// Constants
#define SYSTICK_RATE_HZ 10000ul

#define PWM1_RATE_HZ 150
#define PWM_DIV_CODE SYSCTL_PWMDIV_4
#define PWM_DIVIDER 4
#define PWM1_DEF_DUTY 50

//constants from email and other stuff
#define BUF_SIZE 20
#define TICKS_PER_BUT_CHK 100ul



// Globals (added from email)
volatile static unsigned long g_tickCnt;
volatile static unsigned long g_period;


//*******************************************************************
// ISR for the SysTick interrupt (used for button debouncing).
//*******************************************************************
/*
void
SysTickIntHandler (void)
{
	//
	// Poll the buttons
	updateButtons();
}
*/
void
SysTickIntHandler (void)
{
   g_tickCnt++;
   if (g_tickCnt % TICKS_PER_BUT_CHK == 0)
     updateButtons();          // poll buttons
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

void
 PinChangeIntHandler (void)
 {
     static unsigned long ulLastCnt;    // Retains previous count of ticks

     // Clear the interrupt (documentation recommends doing this early)
     GPIOPinIntClear (GPIO_PORTD_BASE, GPIO_PIN_0);
      // Calculate the period in ticks and prepare for next interrupt
     g_period = g_tickCnt - ulLastCnt;
     ulLastCnt = g_tickCnt;
 }

//******************************************************************
// Initialise the GPIO for the PWM output (Port F)
//******************************************************************
void
initPin (void)
{
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM (GPIO_PORTF_BASE, GPIO_PIN_2);

    //Placed from pulseInrpt.c

    // Register the handler for Port D into the vector table
    GPIOPortIntRegister (GPIO_PORTD_BASE, PinChangeIntHandler);
    //
    // Enable and configure the port and pin used:  input on PF5: Pin 27
    SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeGPIOInput (GPIO_PORTD_BASE, GPIO_PIN_0); // changed from GPIOPadConfigSet
    //
    // Set up the pin change interrupt (rising edges)
    GPIOIntTypeSet (GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
    //
    // Enable the pin change interrupt
    GPIOPinIntEnable (GPIO_PORTD_BASE, GPIO_PIN_0);
    IntEnable (INT_GPIOD);	// Note: INT_GPIOD defined in inc/hw_ints.h


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

// ISR for the SysTick interrupt (from email)

 // ISR for the for the pin change interrupt on rising edge of PD0. (from email)


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

    if(freq > 100 && freq < 300){
	usprintf(string, "Frequency = %7d", freq);
    }
    else{
    	usprintf(string, "Frequency = %7d", PWM1_RATE_HZ);
    }


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
	updateButtons();
    unsigned int duty = PWM1_DEF_DUTY;
    unsigned long period;
    unsigned long input_freq;



    initClock ();
    initPin ();
    initButSet (UP_B | DOWN_B, 1000);  //SYSTICK_RATE_HZ changed to 1000hz
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
    while(1)
    {
    	input_freq = 10000/g_period;

    	updateButtons();
    	if(input_freq <= 300 && input_freq >= 100){
    		period = SysCtlClockGet () / PWM_DIVIDER / input_freq;
    	}
    	else{
    		period = SysCtlClockGet () / PWM_DIVIDER / PWM1_RATE_HZ;
    	}



       // Background task: Check for button pushes and control the
       //  PWM duty cycle +/- 5% within the range 5% to 95%.
       if (checkBut (UP) && duty < 95)
       {
    	  duty += 5;
          PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * duty /100);
       }
       updateButtons();
       if (checkBut (DOWN) && duty > 5)
       {
    	  duty -= 5;
          PWMPulseWidthSet (PWM_BASE, PWM_OUT_4, period * duty /100);
       }
       updateButtons();
       display(input_freq, duty);
       PWMGenPeriodSet (PWM_BASE, PWM_GEN_2, period);
    }
}
