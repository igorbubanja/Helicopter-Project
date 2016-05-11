// *******************************************************
// 
// button.c	** Version 2 **
//
// Support for a button on the Stellaris LM3S1968 EVK
// See also buttonSet.c
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
#include "button.h"

// *******************************************************
// init_button: Initialise the button instance for the specific button & pin.
//  Enable the port and pin for the button for polling only. Initialise state 
//  of button state machine to 'out'. Assumes a call to 
//  SysCtlPeripheralEnable() has been made for the port.

void
initButton (button_t *button, unsigned long ulPort, unsigned char ucPin)
{
   button->iState = BUT_OUT;
   button->uiCnt = 0;			// Counter initialises to zero
   button->ulPort = ulPort;
   button->ucPin = ucPin;

   	// Configure the pin for input with pull up (normally one)
   GPIOPinTypeGPIOInput (ulPort, ucPin);
   GPIODirModeSet (ulPort, ucPin, GPIO_DIR_MODE_IN);
   GPIOPadConfigSet (ulPort, ucPin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

