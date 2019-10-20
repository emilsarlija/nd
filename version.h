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
*/

/* version.h kindly "stolen" from Toby Thain */

/* version history

02 JUN 2007 -	First executable & compilable version, only supports
Ver 0.01	"bootloader" paper tape format, no I/O

05 JUN 2007 -	Added a bunch of I/O instructions, CPU instructions,
Ver 0.02	and TTO simulation

05 JUN 2007 -	The simulator now dumps memory contents to screen as it loads
Ver 0.021	the program, plus bugfixes & cleanup - added by Toby Thain
		(Many thanks, great idea! - Emil)
		
06 JUN 2007 -	Added examine memory and disassemble commands. Also added
Ver 0.022	disassembly to the memory dumping while loading

07 JUN 2007 -	Fixed a bug in SKP<t> CPU. Added MUL/DIV and stack instructions.
Ver 0.03	Minor code clean-up as well. Really minor

*/
#define VERSION "0.03"
