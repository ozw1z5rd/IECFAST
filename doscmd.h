/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007-2009  Ingo Korb <ingo@akana.de>
   Fast Serial protocol and Burst Command Instruction Set:
   Copyright (C) 2011  Robert Willie <hydradix@yahoo.com>

   Inspiration and low-level SD/MMC access based on code from MMC2IEC
     by Lars Pontoppidan et al., see sdcard.c|h and config.h.

   FAT filesystem access based on code from ChaN and Jim Brain, see ff.c|h.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License only.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


   doscmd.h: Definitions for the command channel parser

*/

#ifndef DOSCMD_H
#define DOSCMD_H

extern uint8_t command_buffer[CONFIG_COMMAND_BUFFER_SIZE+2];
extern uint8_t command_length;
extern date_t date_match_start;
extern date_t date_match_end;

extern uint16_t datacrc;
extern uint8_t bcis_status;
extern uint8_t bcis_interleave;

void parse_doscommand(void);

#endif
