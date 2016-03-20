//*****************************************************************************
//
// rgb_commands.c - Command line functionality implementation
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

#include "qs-rgb.h"
#include "drivers/rgb.h"
#include "inc/hw_types.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "rgb_commands.h"

//*****************************************************************************
//
// Table of valid command strings, callback functions and help messages.
//
//*****************************************************************************
tCmdLineEntry g_sCmdTable[] =
{
    {"help",     CMD_help,      " : Display list of commands" },

    {"peek",     CMD_peek,      " : Peek GBS8200 register"},
    {"poke",     CMD_poke,      " : Poke GBS8200 register with value"},
    {"288p",     CMD_288p,      " : Program GBS8200 for 288p mode"},

    { 0, 0, 0 }
};

const int NUM_CMD = sizeof(g_sCmdTable)/sizeof(tCmdLineEntry);

//*****************************************************************************
//
// Command: help
//
// Print the help strings for all commands.
//
//*****************************************************************************
int
CMD_help (int argc, char **argv)
{
    int index;
    
    (void)argc;
    (void)argv;
    
    UARTprintf("\n");
    for (index = 0; index < NUM_CMD - 1; index++)
    {
      UARTprintf("%17s %s\n",
        g_sCmdTable[index].pcCmd,
        g_sCmdTable[index].pcHelp);
    }
    UARTprintf("\n"); 
    
    return (0);
}

#include "ProgramArray288p.h"

#include "driverlib/i2c.h"
#include "inc/hw_memmap.h"

int
CMD_peek(int argc, char** argv)
{
	unsigned long reg;
    if(argc == 2)
    {
        reg = ustrtoul(argv[1], 0, 16);

    		I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x17, false);
			// Switch bank command
			I2CMasterDataPut(I2C0_MASTER_BASE, 0xF0);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));
			// Param: bank number
			I2CMasterDataPut(I2C0_MASTER_BASE, reg / 256);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));

			// Send register address byte
			I2CMasterDataPut(I2C0_MASTER_BASE, reg & 0xFF);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));

			// Get register value
    		I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x17, true);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));

			int value = I2CMasterDataGet(I2C0_MASTER_BASE);

			UARTprintf("Reg %x value %x\n", reg, value);
    }

	return 0;
}


int
CMD_poke(int argc, char** argv)
{
	unsigned long reg;
    if(argc == 3)
    {
        reg = ustrtoul(argv[1], 0, 16);
        unsigned long val = ustrtoul(argv[2], 0, 16);

    		I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x17, false);
			// Switch bank command
			I2CMasterDataPut(I2C0_MASTER_BASE, 0xF0);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));
			// Param: bank number
			I2CMasterDataPut(I2C0_MASTER_BASE, reg / 256);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));
            SysCtlDelay(SysCtlClockGet() / (1000 / 3));

			// Send register address byte
			I2CMasterDataPut(I2C0_MASTER_BASE, reg & 0xFF);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));

			// Set register value
			I2CMasterDataPut(I2C0_MASTER_BASE, val & 0xFF);
        	I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        	while(I2CMasterBusy(I2C0_MASTER_BASE));

			UARTprintf("Reg %x set to %x\n", reg, val);
    }

	return 0;
}


int
CMD_288p(int argc, char **argv)
{
	UARTprintf("Configuring to PAL progressive"); 

    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x17, false);
	for (int bank = 0; bank < 6; bank++) {
		// Switch bank command
		I2CMasterDataPut(I2C0_MASTER_BASE, 0xF0);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
		while(I2CMasterBusy(I2C0_MASTER_BASE));
		// Param: bank number
		I2CMasterDataPut(I2C0_MASTER_BASE, bank);
		I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
		while(I2CMasterBusy(I2C0_MASTER_BASE));

		for (int i = 0; i < programArray288[bank].size; i++)
		{
			int io = (i + programArray288[bank].offset) & 0xFF;
			// Send them 16 bytes at a time, first giving the register number.
			if ((i % 16) == 0) {
				// Start of a new block
				I2CMasterDataPut(I2C0_MASTER_BASE, io);
				I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);
				while(I2CMasterBusy(I2C0_MASTER_BASE));

				I2CMasterDataPut(I2C0_MASTER_BASE, programArray288[bank].data[i]);
				I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
				while(I2CMasterBusy(I2C0_MASTER_BASE));
			} else if (i % 16 == 15) {
				// End of 16byte block, or end of bank
				I2CMasterDataPut(I2C0_MASTER_BASE, programArray288[bank].data[i]);
				I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
				while(I2CMasterBusy(I2C0_MASTER_BASE));
			} else {
				I2CMasterDataPut(I2C0_MASTER_BASE, programArray288[bank].data[i]);
				I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
				while(I2CMasterBusy(I2C0_MASTER_BASE));
			}
		}
	}

	return 0;
}
