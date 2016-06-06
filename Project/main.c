//*****************************************************************************
//
// Program for Helicopter Project
//
// Authors:  Igor Bubanja and Danny Kim
//*****************************************************************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pwm.h"
#include "inc/hw_ints.h"
#include "inc/lm3s1968.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "driverlib/debug.h"
#include "string.h"
#include "drivers/rit128x96x4.h"
#include "stdio.h"
#include "driverlib/adc.h"
#include "adc.h"
#include "math.h"
#include "driverlib/uart.h"
#include "drivers/rit128x96x4.h"
#include "utils/uartstdio.h"

/* CONSTANTS */
 #define MAX_32BIT_VAL 0xFFFFFFFF
#define SYSTICK_RATE_HZ 50000ul
#define BAUD_RATE 9600ul
#define ALTITUDE_RANGE 800 // 800 mV


int main_desired; //desired altitude in percent
int main_measured; //measured percentage altitude

int main_duty; //duty cycle of the main motor

//constants to be used for he pid control of the main motor
float main_kp;
float main_ki;
float main_kd;
//error values for the main motor
float main_errorp;
float main_errori;
float main_errord;

int tail_desired; //desired yaw in degrees
int tail_measured; //measured yaw in degrees

int tail_duty; //duty cycle of the tail motor

//constants to be used for he pid control of the tail motor
float tail_kp;
float tail_ki;
float tail_kd;
//error values for the tail motor
float tail_errorp;
float tail_errori;
float tail_errord;


volatile unsigned long sysTick;
int findingYaw ; //a flag that indicates if the helicopter is finding the reference yaw. 0=yes, 1=no

// current status of heli
// state = 0: on the ground
// state = 1: flying
// state = 2: landing
static int state = 0;

// record max voltage when select button is pressed
static int max_voltage;

static int yaw;
volatile unsigned long yawIntCount;
#define YAW_CONSTANT 0.8035f

static unsigned long period; //used for the pwm
#define PWM_DEFAULT_FREQUENCY 150 // Hz
void heliStart(void)
{
	main_desired = 0; //desired height is set to 0 when the helicopter is started
	tail_desired = 0;
	main_duty=15;

	max_voltage = (getADC() * 1000 - 868) / 337; // record the height for landing (highest voltage in mV)

	//set the tail motor duty cycle
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, period * tail_duty / 100);
	//set the main motor duty cycle
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, period * main_duty / 100);

	//enable the pwm
	PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT | PWM_OUT_4_BIT, true);
}

void
UARTSend (char *pucBuffer) //a function that sends a word through UART
{
    //
    // Loop while there are more characters to send.
    //
    while(*pucBuffer)
    {
        //
        // Write the next character to the UART Tx FIFO.
        //
        UARTCharPut(UART0_BASE, *pucBuffer);
        pucBuffer++;
    }
}
void ButtonIntHandler(void) //interrupt handler for the buttons
{




	// clean the current interrupt state
	GPIOPinIntClear (GPIO_PORTB_BASE, GPIO_PIN_5| GPIO_PIN_6 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_1);

	//read pin values
	int up = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);
	int down = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_6);
	int ccw = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3);
	int cw = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_2);
	int select = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4);
	int reset = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_1);

	if (!select) //the select is inverted because the buttons are active low i.e. should perform their function of a low voltage is read on the corresponding pin
		{
			if (state == 0) //if the helicopter is on the ground
			{
				heliStart();
				state = 1; //change the state to flying
			}
			else if (state == 1)
			{
				state = 2; //is select is pressed when the helicopter is flying, start landing
			}
			else if (state == 2)
			{
				// ignore any input until landed
			}

		}
		if (!up)
		{
			if(main_desired < 100)
			main_desired += 10; //increase the desired altitude by 10%
		}
		if (!down)
		{
			if(main_desired > 0)
			main_desired -= 10;
		}
		if (!ccw)
		{
			tail_desired -= 15; //decrease the desired yaw by 15 degrees
		}
		if (!cw)
		{
			tail_desired += 15;
		}
		if (!reset)
		{
			UARTSend("SoftReset");
			SysCtlReset();
		}
}
void buttonsInit(void)
{
	// regsiter the handler for port G into the vector table
	GPIOPortIntRegister (GPIO_PORTB_BASE, ButtonIntHandler);

	// enable the PG3 to PG7 to read the five buttons
	GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4, GPIO_STRENGTH_2MA,
	                     GPIO_PIN_TYPE_STD_WPU);

	// setup the pin change interrupt
	GPIOIntTypeSet (GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4, GPIO_FALLING_EDGE);

	// enable the pin change interrupt
	GPIOPinIntEnable (GPIO_PORTB_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4);


	IntEnable (INT_GPIOB);
}

void pwmInit(void)
{
	// set clock divider for pwn generator
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);


	// set PF2 for tail motor, and PD1 for main motor
	GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_1);
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);

	// compute the pwm period based on the system clock
	period = SysCtlClockGet() / PWM_DEFAULT_FREQUENCY ;

	// use PWM_GEN0 for PWM1 and PWM_GEN_2 for PWM4
	PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
	                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	PWMGenConfigure(PWM0_BASE, PWM_GEN_2,
	                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

	// set the pwm period
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, period);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, period);


	// set the duty cycle
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, (unsigned long)(period * 0 / 100));
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, (unsigned long)(period * 0 / 100));

	PWMGenEnable(PWM0_BASE, PWM_GEN_0);
	PWMGenEnable(PWM0_BASE, PWM_GEN_2);
}
void yawIntHandler(void)
{
	static unsigned char state;

	unsigned char B, A;

	yawIntCount ++;

	// clean the current interrupt state
	GPIOPinIntClear (GPIO_PORTF_BASE, GPIO_PIN_5 | GPIO_PIN_7);

	B = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_7);
	A = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_5);


	if (state == 0) // in state 0, A = 0, B = 0, clockwise
	{
		if (B != 0) // B->1
		{
			yaw++;
			state = 1;
		}
		else if (A != 0) // A->1
		{
			yaw--;
			state = 3;
		}
	}

	else if (state == 1) // in state 1, A = 0, B = 1, intermediate state
	{
		if (B == 0)
		{
			yaw--;
			state = 0;
		}
		else if (A != 0)
		{
			yaw++;
			state = 2;
		}
	}

	else if (state == 2)
	{
		if (B == 0)
		{
			yaw++;
			state = 3;
		}
		else if (A == 0)
		{
			yaw--;
			state = 1;
		}
	}

	else if (state == 3)
	{
		if (A == 0)
		{
			yaw++;
			state = 0;
		}
		else if (B != 0)
		{
			yaw--;
			state = 2;
		}
	}
}
void yawInit(void)
{
	// regsiter the handler for port F into the vector table
	GPIOPortIntRegister (GPIO_PORTF_BASE, yawIntHandler);

	// enable the PF5 and PF7 to read the pin changes
	GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_5 | GPIO_PIN_7, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_5 | GPIO_PIN_7, GPIO_STRENGTH_2MA,
		                     GPIO_PIN_TYPE_STD_WPU);

	// set pin change interrupt
	GPIOIntTypeSet (GPIO_PORTF_BASE, GPIO_PIN_5 | GPIO_PIN_7, GPIO_BOTH_EDGES);

	// enable the pin change interrupt
	GPIOPinIntEnable (GPIO_PORTF_BASE, GPIO_PIN_5 | GPIO_PIN_7);

	IntEnable (INT_GPIOF);

}

// systick counter
void SysTickIntHandler (void)
{

	if (sysTick % 250 == 0)
	{
		// ADCProcessorTrigger(ADC0_BASE, 3);
		ADC0_PSSI_R = ADC_PSSI_SS3;
	}

    sysTick ++;
}

// initilization for system
void sysInit(void)
{
	// clock divider have to be no more than 10, otherwise CPU will not run
	SysCtlClockSet (SYSCTL_SYSDIV_1 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
				   SYSCTL_XTAL_8MHZ);


	// At current configuration, pre-scalar set to 6 and SYSTICK_RATE set to 1e6,
	// it will takes 71 minutes to run out of 32 bit unsigned integer
	SysTickPeriodSet (SysCtlClockGet() / SYSTICK_RATE_HZ);

	// Register the interrupt handler
	SysTickIntRegister (SysTickIntHandler);

	// Enable interrupt and device
	SysTickIntEnable ();
	// Enable SysTick device (no interrupt)
	SysTickEnable ();
}

void pinReset(void)
{
	SysCtlPeripheralReset (SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralReset (SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralReset (SYSCTL_PERIPH_UART0);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralReset (SYSCTL_PERIPH_GPIOG);
}

void pinInit(void)
{

	// enable system peripherals
	SysCtlPeripheralEnable (SYSCTL_PERIPH_ADC0);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_PWM0);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable (SYSCTL_PERIPH_GPIOG);


}



void main_motor_control(void)
{
	int measured_voltage;

	unsigned long sysTick_new;

	float dt;
	float error;
	float feedback;

	static unsigned long sysTick_old;
	static float error_old;

	measured_voltage = (getADC() * 1000 - 868) / 337;
	main_measured = 100 - 100 * (measured_voltage - (max_voltage - ALTITUDE_RANGE)) / ALTITUDE_RANGE; //convert to a percentage


	// calculate dt
	sysTick_new = sysTick;
	dt = (sysTick_new - sysTick_old) / (float)SYSTICK_RATE_HZ;

	// calculate errors
	error = main_desired - main_measured;

	// calculate proportional error
	main_errorp = main_kp * error;

	// calculate integrated error
	main_errori += main_ki * (error + error_old) * dt / 2 ;

	// calculate derivative error
	main_errord = main_kd * (error - error_old) / dt;

	feedback = main_errorp + main_errori + main_errord;

	// store last record error. This is to calculate the integral error
	error_old = error;
	sysTick_old = sysTick_new; //set the old value for systick

	// set the duty cycle
	main_duty = (int)round(feedback);

	// keep the duty cycle between 2 and 98%
	if (main_duty < 2 && state != 2)
	{
		main_duty = 2;
	}
	else if (main_duty > 98)
	{
		main_duty = 98;
	}

	//pwmDutyCycleSet(main_duty, PWM_OUT_1);
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, period * main_duty / 100);

}

void tail_motor_control(void)
{
	unsigned long sysTick_new;

	float dt;
	float error;
	float feedback;

	static unsigned long sysTick_old;
	static float error_old;

	tail_measured = yaw * YAW_CONSTANT; // in degrees


	// calculate dt
	sysTick_new = sysTick;
	dt = (sysTick_new - sysTick_old) / (float)SYSTICK_RATE_HZ;

	// calculate errors
	error = tail_desired - tail_measured;

	tail_errorp = tail_kp * error;

	tail_errori += tail_ki * (error + error_old) / 2 * dt;

	tail_errord = tail_kd * (error - error_old) / dt;

	feedback = tail_errorp + tail_errori + tail_errord;

	// store last record error
	error_old = error;
	sysTick_old = sysTick_new;

	// set duty cycle
	tail_duty = (int)round(feedback);

	// keep the duty cycle between 2 and 98%
	if (tail_duty < 2)
	{
		tail_duty = 2;
	}
	else if (tail_duty > 98)
	{
		tail_duty = 98;
	}

	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_4, period * tail_duty / 100); //set the pwm suty cycle using the new tail_duty value
}


void heliLanding(void)
{
	static unsigned long sysTick_old;
	unsigned long sysTick_new;

	sysTick_new = sysTick;

	if (1000 * (sysTick_new - sysTick_old)/SYSTICK_RATE_HZ >= 200) // every 200ms the desired altitude is decreased by 2% until the measured altitude is under 5%. At this point the main motor turns off
	{
		if (main_measured > 5) // when the height is greater than 5%, decrease the desired altitude
		{
			main_desired -= 2;
		}
		else // when the height is less than 5%, the desired altitude is set to 0% and the pwm is disabled (i.e. motor turns off)
		{
			main_desired = 0;
			PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT | PWM_OUT_4_BIT, false);
			state = 0;
		}
		sysTick_old = sysTick_new;
	}
}
void referenceFindIntHandler(void){ //interrupt handler for finding the reference yaw
	GPIOPinIntClear (GPIO_PORTD_BASE, GPIO_PIN_0);

		//read pin values

		long yaw_reference = GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_0);


		if (!findingYaw && yaw_reference > 0){
			tail_measured = 0; //set the current yaw to 0
			yaw = 0;
			findingYaw = 1; //flag is set to 1 so that the helicopter stops spinning
			tail_desired = 0;
			main_desired = 0;

		}
}

void yawReferenceInitialise(void){
	// regsiter the handler for port D
	GPIOPortIntRegister (GPIO_PORTD_BASE, referenceFindIntHandler);

	// enable PD0 to read inputs
	GPIODirModeSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_DIR_MODE_IN);

	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA,
			 GPIO_PIN_TYPE_STD_WPU );

	// setup the pin change interrupt
	GPIOIntTypeSet (GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_BOTH_EDGES);

	// enable the pin change interrupt
	GPIOPinIntEnable (GPIO_PORTD_BASE, GPIO_PIN_0);

	IntEnable (INT_GPIOD);
}



void findReferenceYaw(void){ //makes the helicopter spin around until the interrupt from the yaw reference pin (PD0) is detected

		static unsigned long sysTick_old;
		unsigned long sysTick_new;

		sysTick_new = sysTick;

		if (1000 * (sysTick_new - sysTick_old)/SYSTICK_RATE_HZ >= 200) // every 200ms the desired yaw is decreased by 2 degrees. This causes the helicopter to spin around in a circle
		{
				tail_desired -= 2;
				sysTick_old = sysTick_new;
		}

}

void UART_display(void)
{ //this function displays some information about the helicopter and displays it using UART
	char buffer[128];
	sprintf(buffer, " Alt %d [%d] Yaw %d [%d] \n Main Duty: %d \n Tail Duty: %d\r",
			main_desired,
			main_measured,
			tail_desired,
			tail_measured,

			main_duty,
			tail_duty
			);
	UARTSend((const unsigned char *)buffer);
	//display the state
	char buffer1[128];
	if(state == 0){
		sprintf(buffer1, "state: on the ground    \n \n \n\r");
		UARTSend((const unsigned char *)buffer1);
	}
	else if(state == 1){
			sprintf(buffer1, "state: flying  \n \n \n \r");
			UARTSend((const unsigned char *)buffer1);
		}
	else if(state == 2){
			sprintf(buffer1, "state: landing  \n \n \n\r");
			UARTSend((const unsigned char *)buffer1);
		}
}

void consoleInit(void)
{
	GPIOPinTypeUART (GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTConfigSetExpClk (UART0_BASE, SysCtlClockGet(), BAUD_RATE,
						(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
						 UART_CONFIG_PAR_NONE));
	UARTFIFOEnable (UART0_BASE);
	UARTEnable (UART0_BASE);
}


int main(void)
{
	//initialisations
	pinReset();
	sysInit();
	pinInit();
	pwmInit();
	yawInit();
	ADCInit();
	buttonsInit();
	consoleInit();
	yawReferenceInitialise();

	//set constants for the pid control
	main_kp = 1.35;
	main_ki = 0.82;
	main_kd = 0.20;

	tail_kp = 0.90;
	tail_ki = 0.10;
	tail_kd = 0.20;


	// enable interrupts
	IntMasterEnable();


	while (1)
	{

		if (state == 1) //flying
		{
			main_motor_control();
			tail_motor_control();

			if(!findingYaw){
			findReferenceYaw(); //if the reference yaw hasn't been found, keep spinning the helicopter
			}

		}

		// stop
		else if (state == 0)
		{

		}

		// landing
		else if (state == 2)
		{
			heliLanding();
			main_motor_control();
			tail_motor_control();

		}

		UART_display(); //Display the information using UART


	}
}

