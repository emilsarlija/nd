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

Nova I/O instruction simulation routine

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "version.h"
#include "reg_flags.h"

int ctrl_func(int cont, int devnum);

int in_out(unsigned int instruction)
	{
	int generic_io(int acc, int op, int ctrl, int dev);
	
	unsigned int accum, opcode, control, device, temp;
	int count;
	
	accum = (instruction >> 11) & 3;
	opcode = (instruction >> 8) & 7;
	control = (instruction >> 6) & 3;
	device = instruction & 0x3f;

	if (device == 077)
		{
		switch(opcode) /* all the spiffy CPU specific I/O instructions */
			{
			case 0: /* Enable or disable interrupts */
				switch(control)
					{
					case 1: /* INTEN */
						dev_busy[077] = 1; /* ION = 1 */
						break;
					case 2: /* INTDS */
						dev_busy[077] = 0; /* ION = 0 */
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
				break;
			case 1: /* READS */
				AC[accum] = SR;
				ctrl_func(control, device);
				break;
			case 2: /* Undocumented - perform generic I/O operation */
				generic_io(accum, opcode, control, device);
				break;
			case 3: /* INTA */
				/* do nothing until I implement interrupt handling code */
				break;
			case 4: /* MSKO */
				/* Priority mask code would go here */
				break;
			case 5: /* IORST */
				for(count = 0; count < 64; count++) /* clear all device Busy and Done flags */
					{
					dev_busy[count] = 0;
					dev_done[count] = 0;
					}
				/* Priority mask code would go here */
				ctrl_func(control, device);
				break;
			case 6: /* HALT */
				ctrl_func(control, device);
				run = 0; /* stop emulation */
				printf("\nHALT instruction encountered.\n");
				halt = 1;
				break;
			case 7: /* SKP */
				switch(control)
					{
					case 0: /* BN */
						if (dev_busy[077]) /* is ION = 1 ? */
							PC++;
						break;
					case 1: /* BZ */
						if (!dev_busy[077]) /* is ION = 0 ? */
							PC++;
						break;
					case 2: /* DN */
						break; /* Never skip, PSU is infallible, Power Fail Flag is always 0 */
					case 3: /* DZ */
						PC++; /* Always skip, PSU is infallible, Power Fail Flag is always 0 */
						break;
					}
				break;
			}
		}
	else if (device == 1)
		{
		switch(opcode) /* all the spiffy MUL/DIV and stack specific I/O instructions */
			{
			case 0: /* Frame Pointer instructions */
				switch(control)
					{
					case 0: /* MTFP */
						FP = AC[accum] & 077777;
						break;
					case 2: /* MFFP */
						AC[accum] = FP & 077777;
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
				break;
			case 2: /* Stack Pointer instructions */
				switch(control)
					{
					case 0: /* MTSP */
						SP = AC[accum] & 077777;
						break;
					case 2: /* MFSP */
						AC[accum] = SP & 077777;
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
				break;
			case 3: /* Push and Pop instructions */
				switch(control)
					{
					case 0: /* PSHA */
						++SP;
						SP &= 077777;
						RAM[SP] = AC[accum];
						break;
					case 2: /* POPA */
						AC[accum] = RAM[SP];
						--SP;
						SP &= 077777;
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
				break;
			case 5: /* Save and Return instructions */
				switch(control)
					{
					case 0: /* SAV */
						for(count = 0; count < 3; ++count) /* push AC0-2 on the stack */
							{
							++SP;
							SP &= 077777;
							RAM[SP] = AC[count];
							}
						++SP;
						SP &= 077777;
						RAM[SP] = FP & 077777; /* push Frame Pointer on the stack */
						++SP;
						SP &= 077777;
						RAM[SP] = AC[3] & 077777; /* push bits 1-15 of AC3 on the stack */
						if (C)
							RAM[SP] |= 0100000; /* set MSB of the pushed word if Carry is 1 */
						FP = SP; /* place Stack Pointer in the Frame Pointer */
						AC[3] = SP; /* place Stack Pointer in AC3 */
						break;
					case 2: /* RET */
						SP = FP; /* place Frame Pointer in the Stack Pointer */
						PC = (RAM[SP] & 077777) -1; /* pop the Program Counter */
						if ((RAM[SP] & 0100000) == 0100000)
							C = 1; /* set Carry id MSB of the popped word is 1 */
						--SP;
						SP &= 077777;
						FP = RAM[SP] & 077777; /* pop the frame pointer off the stack, clear MSB */
						AC[3] = RAM[SP]; /* the full word goes into AC3 */
						--SP;
						SP &= 077777;
						for(count = 2; count >= 0; --count) /* pop AC0-3 off the stack */
							{
							AC[count] = RAM[SP];
							--SP;
							SP &= 077777;
							}
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
				break;
			case 6: /* Divide and Multiply instructions */
				switch(control)
					{
					case 1: /* DIV */
						if (AC[0] >= AC[2])
							C = 1;
						else
							{
							temp = (AC[0] << 16) | AC[1];
							AC[1] = temp / AC[2]; /* quotient */
							AC[0] = temp % AC[2]; /* remainder */
							C = 0;
							}
						break;
					case 3: /* MUL */
						temp = AC[1] * AC[2];
						temp += AC[0];
						AC[1] = temp & 0xffff;
						AC[0] = (temp >> 16) & 0xffff;
						break;
					default: /* Undocumented - perform generic I/O operation */
						generic_io(accum, opcode, control, device);
						break;
					}
			default: /* Undocumented - perform generic I/O operation */
				generic_io(accum, opcode, control, device);
				break;
			}
		}
	else
		{
		generic_io(accum, opcode, control, device);
		
		switch(device)
			{
			case 010: /* TTI device */
				/* insert code here */
				break;
			case 011: /* TTO device */
				if (opcode == 2) /* check to see if it's a DOA */
					{
					dev_A[device] = dev_A[device] & 0177;
					putchar(dev_A[device]);
					fflush(stdout); // buffering doesn't really help us, I think
					dev_busy[device] = 0;
					dev_done[device] = 1;
					}
				break;
			default:
				break;
			} 
		}
	return 0;
	}
	
int generic_io(int acc, int op, int ctrl, int dev)		
	{
	switch(op)
		{
		case 0:	/* NIO */
			ctrl_func(ctrl, dev);
			break;
		case 1: /* DIA */
			AC[acc] = dev_A[dev];
			ctrl_func(ctrl, dev);
			break;
		case 2: /* DOA */
			dev_A[dev] = AC[acc];
			ctrl_func(ctrl, dev);
			break;
		case 3: /* DIB */
			AC[acc] = dev_B[dev];
			ctrl_func(ctrl, dev);
			break;
		case 4: /* DOB */
			dev_B[dev] = AC[acc];
			ctrl_func(ctrl, dev);
			break;
		case 5: /* DIC */
			AC[acc] = dev_C[dev];
			ctrl_func(ctrl, dev);
			break;
		case 6: /* DOC */
			dev_C[dev] = AC[acc];
			ctrl_func(ctrl, dev);
			break;
		case 7: /* SKP */
			switch(ctrl)
				{
				case 0: /* BN */
					if (dev_busy[dev])
						PC++;
					break;
				case 1: /* BZ */
					if (!dev_busy[dev])
						PC++;
					break;
				case 2: /*DN */
					if (dev_done[dev])
						PC++;
					break;
				case 3: /* DZ */
					if (!dev_done[dev])
						PC++;
					break;
				}
			break;
		}

	
	return 0;
	}

int ctrl_func(int cont, int devnum)
	{
	switch(cont)
		{
		case 0:
			break;
		case 1: /* Start */
			dev_busy[devnum]=1;
			dev_done[devnum]=0;
			break;
		case 2:/*Clear */
			dev_busy[devnum]=0;
			dev_done[devnum]=0;
			break;
		case 3: /* Pulse */
			dev_pulse[devnum]=1;
			break;
		}
	return 0;
	}