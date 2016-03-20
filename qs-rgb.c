//*****************************************************************************
//
// qs-rgb.c - Quickstart for the EK-LM4F120XL Stellaris LaunchPad
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************

#include <math.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "drivers/rgb.h"
#include "rgb_commands.h"
#include "qs-rgb.h"

#include "driverlib/i2c.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>EK-LM4F120XL Quickstart Application (qs-rgb)</h1>
//!
//! A demonstration of the Stellaris LaunchPad (EK-LM4F120XL) capabilities.
//!
//! Press and/or hold the left button traverse toward the red end of the
//! ROYGBIV color spectrum.  Press and/or hold the right button to traverse
//! toward the violet end of the ROYGBIV color spectrum.
//!
//! Leave idle for 5 seconds to see a automatically changing color display
//!
//! Press and hold both left and right buttons for 3 seconds to enter
//! hibernation.  During hibernation last color on screen will blink on the
//! LED for 0.5 seconds every 3 seconds.
//!
//! Command line UART protocol can also control the system.
//!
//! Command 'help' to generate list of commands and helpful information.
//! Command 'hib' will place the device into hibernation mode.
//! Command 'rand' will initiate the pseudo-random sequence.
//! Command 'intensity' followed by a number between 0.0 and 1.0 will scale
//! the brightness of the LED by that factor.
//! Command 'rgb' followed by a six character hex value will set the color. For
//! example 'rgb FF0000' will produce a red color.
//
//*****************************************************************************


//*****************************************************************************
//
// Input buffer for the command line interpreter.
//
//*****************************************************************************
static char g_cInput[APP_INPUT_BUF_SIZE];

//*****************************************************************************
//
// Application state structure.  Gets stored to hibernate memory for 
// preservation across hibernate events.  
//
//*****************************************************************************
volatile sAppState_t g_sAppState;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


#include <stdint.h>
#define PROGMEM
#include "StartArray.h"

#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"

//*****************************************************************************
//
// Main function performs init and manages system.
//
// Called automatically after the system and compiler pre-init sequences.
// Performs system init calls, restores state from hibernate if needed and 
// then manages the application context duties of the system.
//
//*****************************************************************************
int
main(void)
{
    long lCommandStatus;
    
    //
    // Enable stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPUStackingEnable();

    //
    // Set the system clock to run at 40Mhz off PLL with external crystal as 
    // reference.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    //
    // Enable and Initialize the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);

    UARTprintf("----------------------------------\n");
    UARTprintf("Type 'help' for a list of commands\n");
    UARTprintf("> ");

	// Initialize the I2C
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);

    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);

	UARTprintf("Starting GBS8200...\n"); 

    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x17, false);
	for (int i = 0; i < sizeof(startArray); i ++)
	{
		I2CMasterDataPut(I2C0_MASTER_BASE, startArray[i]);
        I2CMasterControl(I2C0_MASTER_BASE,
			(i & 1) ? I2C_MASTER_CMD_BURST_SEND_FINISH : I2C_MASTER_CMD_BURST_SEND_START);
        while(I2CMasterBusy(I2C0_MASTER_BASE));
	}

	CMD_288p(0, NULL);

    //
    // spin forever and wait for carriage returns or state changes.
    //
    while(1)
    {

        UARTprintf("\n>");

        
        //
        // Peek to see if a full command is ready for processing
        //
        while(UARTPeek('\n') == -1)
        {
            //
            // millisecond delay.  A SysCtlSleep() here would also be OK.
            //
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));
            
        }
        
        //
        // a '\r' was detected get the line of text from the user.
        //
        UARTgets(g_cInput,sizeof(g_cInput));

        //
        // Pass the line from the user to the command processor.
        // It will be parsed and valid commands executed.
        //
        lCommandStatus = CmdLineProcess(g_cInput);

        //
        // Handle the case of bad command.
        //
        if(lCommandStatus == CMDLINE_BAD_CMD)
        {
            UARTprintf("Bad command!\n");
        }

        //
        // Handle the case of too many arguments.
        //
        else if(lCommandStatus == CMDLINE_TOO_MANY_ARGS)
        {
            UARTprintf("Too many arguments for command processor!\n");
        }
    }
}
