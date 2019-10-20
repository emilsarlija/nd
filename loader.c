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

Binary loader and human interface

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "version.h"
#include "reg_flags.h"

int main(int argc, char *argv[])
	{
	int emul(void);
	void print_reg(void);
	int disasm(int addr);	
	 
	FILE *fp;
	unsigned short address;
	int count, c, start_addr, end_addr, asc_addr, temp;

	printf("DG NOVA Debugger/Simulator v" VERSION "\nCopyright (C) 2006-7 Emil Sarlija\n");
	
	if (argc == 1) /* Display copyright and usage */
		printf("\nusage: nd [FILE]\n");
	else
		while(--argc > 0)
			{
			if ((fp = fopen(*++argv, "r")) == NULL) /* Check if the file actually exists */
				{
				printf("\nnd: can't open %s\n", *argv);
				return EXIT_FAILURE;
				}
			else
				{
				do /* fetch bytes until non-zero */
					count = getc(fp);
				while(!count);
				
				count = getc(fp);
				count = 0x10000 - ((count << 8) | getc(fp)); /* fetch count */
				
				if(feof(fp) || count < 2 || count > 192)
					{
					fputs("Are you sure that file is a bootstrap program? Aborting.\n", stderr);
					return EXIT_FAILURE;
					}
				
				for (address = 0100; --count; ++address) /* load words into RAM until the counter hits zero (i.e. no more to fetch) */
					{
					RAM[address] = getc(fp) << 8;
					RAM[address] |= getc(fp);
					printf("[%06o] %06o ", address, RAM[address]);
					disasm(address);
					}
				
				PC = 0100; /* load start address */
				
				printf("\nProgram successfully loaded\n\n");
				
				printf("Current register values are:");
				
				while (!halt)
					{
					print_reg();
					printf("(R)un or (S)ingle Step the program, (E)xamine or (D)isassemble memory -\nor (Q)uit: ");

					// ignore any whitespace
					do
						c = getchar();
					while(c != EOF && isspace(c));

					// make sure we don't get wedged on a ^D or funky input redirection
					if(c == EOF)
						break;

					switch(toupper(c)){
						case 'S': /* Single Step */
							step = 1;
							disasm(PC);
							emul();
							break;
	
						case 'R': /* Run */
							run = 1;
							emul();
							break;
	
						case 'E': /* Examine */
							printf("Start address: ");
							scanf("%o", &start_addr);
							start_addr &= 0x7fff; /* start address for the dumped words */
							asc_addr = start_addr; /* start address for the dumped ascii */
							printf("End address: "); /* end address */
							scanf("%o", &end_addr);
							end_addr &= 0x7fff;
														
							while (start_addr <= end_addr)
								{
								printf("[%06o] ", start_addr);
								
								for(count = 0; count < 4; ++count) /* print 4 words of memory */
									{
									printf("%06o ", RAM[start_addr]);
									++start_addr;
									}
								
								printf("  ");
								
								for(count = 0; count < 4; ++count) /* print 4 ascii pairs */
									{
									temp = RAM[asc_addr] & 0x7f; /* Get low byte */
									
									if (isprint(temp)) /* make sure it's a printable character */
										putchar(temp);
									else
										printf("."); /* print a dot if not */
									
									temp = (RAM[asc_addr] >> 8) & 0x7f; /* Get high byte */
									
									if (isprint(temp)) /* make sure it's a printable character */
										putchar(temp);
									else
										printf("."); /* print a dot if not */

									++asc_addr;
									}
								
								printf("\n");
								}
							break;
							
						case 'D': /* Disassemble */
							printf("Start address: ");
							scanf("%o", &start_addr);
							start_addr &= 0x7fff;
							printf("End address: ");
							scanf("%o", &end_addr);
							end_addr &= 0x7fff;
							
							for (; start_addr <= end_addr; ++start_addr)
								{
								printf("[%06o] %06o ", start_addr, RAM[start_addr]);
								disasm(start_addr);
								}							
							break;
						
						case 'Q': /* Quit */
							return EXIT_SUCCESS;
							
						default:
							break;
						}
					}
				print_reg();
			}
		}

	return EXIT_SUCCESS; 
	}
	
void print_reg(void)
	{
	printf("\nPC=%06o AC0=%06o AC1=%06o AC2=%06o AC3=%06o C=%u SP=%06o FP=%06o\n",
		   PC, AC[0], AC[1], AC[2], AC[3], C, SP, FP);
	}