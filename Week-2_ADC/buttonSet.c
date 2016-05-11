// *******************************************************
// 
// buttonSet.c	** Version 2 **
//
// Support for the set of buttons on the Stellaris LM3S1968 EVK
// See also button.c
// P.J. Bones UCECE
// Last modified:  28.2.2013
// 
// *******************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "driverlib/debug.h"
#include "drivers/rit128x96x4.h"
#include "buttonSet.h"
#include "button.h"

// *******************************************************
// Data on buttons for Stellaris LM3S1968 EVK (on port G)
// See:   \Resources\LM3S1968_micro_board_doc.pdf,  Pages 25, 27.     
//  UP_SWn = PG3: Pin 52 
//  DOWN_SWn = PG4: Pin 30
//  LEFT_SWn = PG5: Pin31
//  RIGHT_SWn = PG6/PhA1 : Pin 34
//  SELECT_SWn = PG7/PhB1: Pin 33

static button_t butArray[5];
static unsigned int butDetect;	// Num ticks for detection
static unsigned int butHoldoff;	// Num ticks for holdoff

// *******************************************************
// initButSet: Initialise the button instances for the specific 
//  buttons specified.  Argument = bit pattern formed by UP_B,
//  DOWN_B, etc., ORed together.  Should only be called once the 
//  SysCtlClock frequency is set.  'tickRateHz' is the rate of
//  SysTick interrupts (set externally). 
void
initButSet (unsigned char buttons, unsigned int tickRateHz)
{
   if (buttons)
   {
      // Call required before any button is initialised.
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
      // Calculate and store the delays in ticks.
      butDetect = BUT_DETECT * tickRateHz / 1000;
      butHoldoff = BUT_HOLDOFF * tickRateHz / 1000;
   }
   if (buttons & UP_B)
      initButton (&butArray[UP], GPIO_PORTG_BASE, GPIO_PIN_3);
   if (buttons & DOWN_B)
      initButton (&butArray[DOWN], GPIO_PORTG_BASE, GPIO_PIN_4);
   if (buttons & LEFT_B)
      initButton (&butArray[LEFT], GPIO_PORTG_BASE, GPIO_PIN_5);
   if (buttons & RIGHT_B)
      initButton (&butArray[RIGHT], GPIO_PORTG_BASE, GPIO_PIN_6);
   if (buttons & SELECT_B)
      initButton (&butArray[SELECT], GPIO_PORTG_BASE, GPIO_PIN_7);
}


// *******************************************************
// updateButtons: Function designed to be called from the SysTick
//  interrupt handler.  It has no return type or argument list. A
//  call to initButSet() is required before the first call to this.
// Note that the inputs are normally 1 (true), active 0.
void
updateButtons (void)
{
   unsigned char ucPin;
   int iSt;
   unsigned butIndex;

   for (butIndex = 0; butIndex < 5; butIndex++)
   {
      iSt = butArray[butIndex].iState;
      if (iSt >= BUT_OUT)
      {	// button is active, so read it and convert to positive logic
         ucPin = !(unsigned char) GPIOPinRead (butArray[butIndex].ulPort, 
            butArray[butIndex].ucPin);	// Now 1 = pushed
         if (iSt == BUT_OUT)
         {		// Look for butDetect consecutive nonzero reads
            if (!ucPin)
               butArray[butIndex].uiCnt = 0;
            else
               butArray[butIndex].uiCnt += 1;
            if (butArray[butIndex].uiCnt >= butDetect)
            {
               butArray[butIndex].iState = BUT_PUSHED;
               butArray[butIndex].uiCnt = 0;
            }
         }
         else
         {      // Look for butHoldoff consecutive zero reads
            if (ucPin)
               butArray[butIndex].uiCnt = 0;
            else
               butArray[butIndex].uiCnt += 1;
            if (butArray[butIndex].uiCnt >= butHoldoff)
            {
               butArray[butIndex].iState = BUT_OUT;
               butArray[butIndex].uiCnt = 0;
            }
         }
      }
   }
}


// *******************************************************
// checkBut: Checks the specified individual button and returns true 
//  (1) if the button is active and has state BUT_PUSHED (newly pushed) 
//  and modifies state to BUT_IN. Returns false (0) otherwise.
//  Values allowed for the argument are:
//  UP = 0, DOWN, LEFT, RIGHT, SELECT
unsigned char 
checkBut (unsigned char button)
{
   if (butArray[button].iState == BUT_PUSHED)
   {
      butArray[button].iState = BUT_IN;
      return 1;
   }
   return 0;
}


// *******************************************************
// anyButPushed: Checks the current set of active buttons and returns 
//  true (>0) if any of them have state = BUT_PUSHED. Value returned 
//  is an ORed set of the bits representing the button(s) newly pushed,
//  e.g., if "UP" and "SELECT" have been recently pushed, the 
//  value returned is (UP_B | SELECT_B). Otherwise returns false (0). 
//  State of any pushed button is altered to BUT_IN.
unsigned char 
anyButPushed (void)
{
   unsigned char bitPat = 0;
   unsigned char mask, butIndex;

   for (butIndex = 0, mask = 1; butIndex < 5; butIndex++, mask = mask<<1)
      if (butArray[butIndex].iState == BUT_PUSHED)
      {
         butArray[butIndex].iState = BUT_IN;
         bitPat |= mask;
      }
   return bitPat;
}


// *******************************************************
// enableBut: Alters the state of the specified button to BUT_OUT, 
//  if it was previously BUT_INACTIVE, otherwise makes no change.
void
enableBut (unsigned char button)
{
   if (butArray[button].iState == BUT_INACTIVE)
      butArray[button].iState = BUT_OUT;
}


// *******************************************************
// disableBut: Alters the state of the specified button to 
//  BUT_INACTIVE.
void
disableBut (unsigned char button)
{
   butArray[button].iState = BUT_INACTIVE;
}


