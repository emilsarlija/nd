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
    
Address calculation routine

*/

#include <stdio.h>

#include "reg_flags.h"

unsigned int addr_calc(unsigned int instruction)
	{
	
	unsigned int ind_addr(unsigned int addr, unsigned char ind);	
	
	unsigned int indirect, index, address;
	unsigned char disp;
	
	indirect = (instruction >>10) & 1;
	index = (instruction >> 8) & 3;
	disp = instruction & 0xff;
	
	if (index == 0)	/* Page Zero addressing */
		address = disp;	
	
	else if (index == 1) /* Relative addressing */
		address = (PC + (signed char)disp) & 077777;
	
	else	/* Indexed addressing */
		address = (AC[index] + (signed char)disp) & 077777;
		
	if (indirect == 1) /* Indirection? */
		address = ind_addr(address, indirect);
		
	return address;
	}
	
unsigned int ind_addr(unsigned int addr, unsigned char ind)
	{	
	while ((addr > 077777) || (ind == 1)) /* Is the MSB set for further indirection? (Beginning of the indirection loop) */
		{
		addr = addr & 077777;
		
		if (RAM[addr] > 077777) /* Check MSB for indirection before auto inc/dec */
			ind = 1;
		else
			ind = 0;
		
		if ((addr > 017) && (addr < 030)) /* Is the memory referenced an auto-increment location? */
			RAM[addr]++;
		else if ((addr > 027) && (addr < 040)) /* Is the memory referenced an auto-decrement location? */
			RAM[addr]--;
			
		addr = RAM[addr]; /* The contents of the refernced memory location becomes the new address */
		}
	return addr;
	}