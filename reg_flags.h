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

External variables for registers and flags

*/

#include <stdio.h>

extern unsigned short RAM[32768];/* Memory */
extern unsigned int AC[4];	/* Accumulators */
extern unsigned int C;		/* Carry Flag */
extern unsigned int SR;		/* Switch Register */
extern unsigned int PC;		/* Program Counter */
extern unsigned int SP;		/* Stack Pointer */
extern unsigned int FP;		/* Frame Pointer */

extern unsigned char dev_busy[64];	/* Device BUSY flags */
extern unsigned char dev_done[64];	/* Device DONE flags */
extern unsigned char dev_pulse[64];	/* Device pulse status */

extern unsigned short dev_A[64];	/* Device register A */
extern unsigned short dev_B[64];	/* Device register B */
extern unsigned short dev_C[64];	/* Device register C */

extern unsigned char run; /* 0=halt, 1=run */
extern unsigned char step; /* 0=halt, 1=step */
extern unsigned char halt; /* indicates the execution of a halt instruction */