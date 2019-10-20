/*
    This file is part of nd - The Nova Debugger
    Copyright (C) 2006-7 Emil Sarlija
    e-mail: emil AT chookfest DOT net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License  
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    Read the file GPL for more information

Nova instruction disassembly routine

*/

#include <stdio.h>

#include "reg_flags.h"

int disasm(int addr)
	{
	int diasm_two_acc(unsigned int instruction);
	int diasm_no_acc(unsigned int instruction);
	int diasm_in_out(unsigned int instruction);
	int diasm_one_acc(unsigned int instruction);
	
	unsigned int instruct;

	instruct = RAM[addr];
	
	if (instruct > 0x7fff)
		diasm_two_acc(instruct);
	else if (instruct < 0x2000)
		diasm_no_acc(instruct);
	else if (instruct > 0x5fff)
		diasm_in_out(instruct);
	else
		diasm_one_acc(instruct);
	return 0;
	}

int diasm_two_acc(unsigned int instruction)
	{
	static char *mnemonic[]={"COM","NEG","MOV","INC","ADC","SUB","ADD","AND"};
	static char *carry_op[]={"", "Z", "O", "C"};
	static char *shift_op[]={"", "L", "R", "S"};
	static char *skip_op[]={"",",SKP",",SZC",",SNC",",SZR",",SNR",",SEZ",",SBN"};
	
	unsigned int source, dest, opcode; 
	unsigned int shift, carry, noload, skip, trapnum;

	
	opcode = (instruction >> 8) & 7; 	/* Get Op-Code */
	source = (instruction >> 13) & 3; /* Get source Accumulator */
	dest = (instruction >> 11) & 3; 	/* Get destination Accumulator */
	shift = (instruction >> 6) & 3; 
	carry = (instruction >> 4) & 3;
	noload = (instruction >> 3) & 1;
	skip = instruction & 7;
	
	if ((noload == 1) && (skip == 0)) /* Check if it's a TRAP instruction */
		{
		trapnum = (instruction >> 4) & 0x7f;
		printf("TRAP %u %u,%u\n", trapnum, source, dest);
		}
	else	
		{	
		printf("%s%s%s", mnemonic[opcode], carry_op[carry], shift_op[shift]);
			
		if(noload == 1)
			printf("#");
		
		printf(" %u,%u", source, dest); /* Print Source and Destination Accumulators */
	
		printf("%s", skip_op[skip]);
		}	
	printf("\n");
	return 0;	
	}
	
int diasm_no_acc(unsigned int instruction)
	{
	static char *mnemonic[]={"JMP", "JSR", "ISZ", "DSZ"};
		
	unsigned int opcode, indirect, index;
	unsigned char disp;
	
	opcode = (instruction >> 11) & 3;
	indirect = (instruction >>10) & 1;
	index = (instruction >> 8) & 3;
	disp = instruction & 0xff;
	
	printf("%s ", mnemonic[opcode]);
	
	if (indirect == 1)
		printf("@");
		
	if (index == 0)
		printf("%o", disp);
	else if (index == 1)
		{
		printf(".");
		if (disp == 0)
			;
		else
			{
			if (disp < 128)
				printf("+");
			printf("%d", (signed char)disp);
			}
		}
	else
		{
		if (disp == 0)
			;
		else if (disp < 128)
			printf("+");
		printf("%d,%d", (signed char)disp, index);
		}
		
	printf("\n");

	return 0;
	}

int diasm_in_out(unsigned int instruction)
	{
	static char *mnemonic[]={"NIO", "DIA", "DOA", "DIB", "DOB", "DIC", "DOC", "SKP"};
	static char *f_op[]={"", "S", "C", "P"};
	static char *t_op[]={"BN","BZ", "DN", "DZ"};
	static char *dev_code[]={"00", "MDV", "MMPU", "MMU1", "04", "05", "MCAT", "MCAR", \
			"TTI", "TTO", "PTR", "PTP", "RTC", "PLT", "CDR", "LPT", "DSK", \
			"ADCV", "MTA", "DACV", "DCM", "25", "26", "27", "QTY", "IBM1", \
			"IBM2", "DKP", "MUX8", "CRC", "IPB", "IVT", "DPI", "DPO", \
			"DIO", "DIOT", "MXM", "45", "MCAT1", "MCAR1", "TTI1", "TTO1", \
			"PTR1", "PTP1", "RTC1", "PLT1", "CDR1", "LPT1", "DSK1", "ADCV1", \
			"MTA1", "DACV1", "64", "65", "66", "67", "QTY1", "71", "72", \
			"DKP1", "FPU1", "FPU2", "FPU", "CPU"};
		
	unsigned int accum, opcode, control, device;
	
	accum = (instruction >> 11) & 3;
	opcode = (instruction >> 8) & 7;
	control = (instruction >> 6) & 3;
	device = instruction & 0x3f;
	
	if ((opcode == 6) && (device == 077)) /* Quick and dirty HALT instruction */
		printf("HALT");		/* To be removed and cleaned up - eventually */
	else if (opcode == 0)
		printf("%s%s %s", mnemonic[opcode], f_op[control], dev_code[device]);
	else if (opcode == 7)
		printf("%s%s %s", mnemonic[opcode], t_op[control], dev_code[device]);
	else
		printf("%s%s %u,%s", mnemonic[opcode], f_op[control], accum, dev_code[device]);
	
	printf("\n");
	return 0;
	}

int diasm_one_acc(unsigned int instruction)
	{
	unsigned int opcode, accum, indirect, index;
	unsigned char disp; 
	
	opcode = (instruction >> 13) & 3;
	accum = (instruction >> 11) & 3;
	indirect = (instruction >>10) & 1;
	index = (instruction >> 8) & 3;
	disp = instruction & 0xff;
	
	if (opcode == 1)
		printf("LDA %u,", accum);
	else
		printf("STA %u,", accum);
	
	if (indirect == 1)
		printf("@");
		
	if (index == 0)
		printf("%o", disp);
	else if (index == 1)
		{
		printf(".");
		if (disp == 0)
			;
		else
			{
			if (disp < 128)
				printf("+");
			printf("%d", (signed char)disp);
			}
		}
	else
		{
		if (disp < 128)
			printf("+");
		printf("%d,%d", (signed char)disp, index);
		}
		
	printf("\n");
	return 0;
	}
