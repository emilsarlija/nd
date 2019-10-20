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

Nova instruction simulation routine

*/

#include <stdio.h>
#include <stdlib.h>

unsigned short RAM[32768] = {0};/* Memory */
unsigned int AC[4] = {0};	/* Accumulators */
unsigned int C = 0;		/* Carry Flag */
unsigned int SR = 0;		/* Switch Register */
unsigned int PC = 0;		/* Program Counter */
unsigned int SP = 0;		/* Stack Pointer */
unsigned int FP = 0;		/* Frame Pointer */
unsigned int ION = 0;		/* Interrupt On flag */

unsigned char dev_busy[64];	/* Device BUSY flags */
unsigned char dev_done[64];	/* Device DONE flags */
unsigned char dev_pulse[64];	/* Device pulse status */

unsigned short dev_A[64];	/* Device register A */
unsigned short dev_B[64];	/* Device register B */
unsigned short dev_C[64];	/* Device register C */

unsigned char run; /* 0=halt, 1=run */
unsigned char step; /* 0=halt, 1=step */
unsigned char halt = 0; /* indicates the execution of a halt instruction */

unsigned int addr_calc(unsigned int instruction);

int emul(void)
	{
	int two_acc(unsigned int instruction);
	int no_acc(unsigned int instruction);
	int in_out(unsigned int instruction);
	int one_acc(unsigned int instruction);
	
	unsigned int instruct, op;

	while (run || step)
		{
		instruct = RAM[PC];
		op = instruct >> 13;
		/* ideally you'd test most frequent first (profile some code) */
		if (op & 4)
			two_acc(instruct);
		else if (op == 0)
			no_acc(instruct);
		else if (op == 3)
			in_out(instruct);
		else
			one_acc(instruct);
	
		PC++;	/* Increment Program Counter */
		PC &= 077777;
		step = 0;	/* Turn off single stepping */
		}
	return EXIT_SUCCESS;
	}


int two_acc(unsigned int instruction) /* Two Accumulator-Multiple Operation instruction emulation */
	{
	unsigned int shifter = 0; 	/* Shifter */
	unsigned int source, dest, opcode; 
	unsigned int carry, shift, noload, skip;
	unsigned int temp;
		
	source = (instruction >> 13) & 3; /* Get source Accumulator */
	dest = (instruction >> 11) & 3; /* Get destination Accumulator */
	opcode = (instruction >> 8) & 7; /* Get Op-Code */
	shift = (instruction >> 6) & 3; 
	carry = (instruction >> 4) & 3;
	noload = (instruction >> 3) & 1;
	skip = instruction & 7;
	
	if ((noload == 1) && (skip == 0)) 		/* Check if it's a TRAP instruction */
		{
		RAM[046] = PC;	/* Store current PC in RAM location 046 */
		PC = RAM[047] - 1; /* Indirect jump to 047 */
		}
	else	
		{
		switch (carry) 	/* Decode Carry */
			{
			case 0:	/* Do nothing */
				break;
			case 1:	/* Clear */
				C = 0;
				break;
			case 2:	/* Set */
				C = 1;
				break;
			case 3:	/* Invert */
				C ^= 1;
				break;
			}
		
		switch (opcode)	/* Decode the Op-Code */
			{
			case 0:	/* COM */
				shifter = AC[source] ^ 0xffff;
				break;
			case 1:	/* NEG */
				shifter = (AC[source] ^ 0xffff) + 1;
				break;
			case 2:	/* MOV */
				shifter = AC[source];
				break;
			case 3:	/* INC */
				shifter = ++AC[source]; /* use incremented value */
				break;
			case 4:	/* ADC */
				shifter = (AC[source] ^ 0xffff) + AC[dest];
				break;
			case 5:	/* SUB */
				shifter = (AC[source] ^ 0xffff) + 1 + AC[dest];
				break;
			case 6:	/* ADD */
				shifter = AC[source] + AC[dest];
				break;
			case 7:	/* AND */
				shifter = AC[source] & AC[dest];
				break;
			}
	
		if (shifter > 0xffff)	/* Check for overflow */
			{
			C ^= 1;	/* Complement carry bit */
			shifter &= 0xffff;	/* Destroy all but lower word */
			}
		
		switch (shift)	/* Decode Shift */
			{
			case 0:	/* No shift */
				break;
			case 1:	/* Shift left */
				shifter = (shifter << 1) | C; /* Place old carry in LSB */
				C = shifter >> 16; /* set carry to old MSB */
				break;
			case 2:	/* Shift right */
				shifter |= C << 16; /* Place old carry in 17th bit */
				C = shifter & 1; /* set carry to old LSB */
				shifter >>= 1;	
				break;
			case 3:	/* Swap bytes */
				temp = shifter >> 8;	/* Get high byte (safe if value is < 0x10000) */
				shifter = (shifter << 8) | temp;	/* Put low byte high, and high byte low */
				break;
			}
		
		shifter &= 0xffff;/* Destroy all but lower word */	
		
		if (!noload) /* Decode No-Load */
			AC[dest] = shifter;
		
		switch (skip) /* Decode Skip */
			{
			case 0:
				break;
			case 1: /* SKP */
				PC++;
				break;
			case 2:	/* SZC */
				if (C == 0)
					PC++;
				break;
			case 3:	/* SNC */
				if (C == 1)
					PC++;
				break;
			case 4:	/* SZR */
				if (shifter == 0)
					PC++;
				break;
			case 5:	/* SNR */
				if (shifter != 0)
					PC++;
				break;
			case 6:	/* SEZ */
				if ((C == 0) || (shifter == 0))
					PC++;
				break;
			case 7:	/* SBN */
				if ((C == 0) && (shifter == 0))
					PC++;
				break;
			}
		}
	return 0;		
	}
	
int no_acc(unsigned int instruction)
	{
	unsigned int opcode, address;
	
	opcode = (instruction >> 11) & 3;
	address = addr_calc(instruction);
	
	switch (opcode) /* Decode instruction */
		{
		case 0: /* JMP */
			PC = address - 1;
			break;
		case 1: /* JSR */
			AC[3] = ++PC;
			PC = address - 1;
			break;
		case 2: /* ISZ */
			RAM[address] = (RAM[address] + 1) & 0xffff;
			if (RAM[address] == 0)
				PC++;
			break;
		case 3: /* DSZ */
			RAM[address] = (RAM[address] - 1) & 0xffff;
			if (RAM[address] == 0)
				PC++;
			break;
		}
	return 0;
	}
	
int one_acc(unsigned int instruction)
	{
	unsigned int opcode, accum, address;
	
	opcode = (instruction >> 13) & 3;
	accum = (instruction >> 11) & 3;
	address = addr_calc(instruction);
	
	if (opcode == 1) /* LDA */
		AC[accum] = RAM[address];
	else /* STA */
		RAM[address] = AC[accum];
	
	return 0;
	}
	