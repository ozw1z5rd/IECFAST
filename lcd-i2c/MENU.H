/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007-2009  Ingo Korb <ingo@akana.de>

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

   
   menu.h: Generic menu code

*/

#ifndef MENU_H
#define MENU_H

extern char *menulines[CONFIG_MAX_MENU_ENTRIES];

void    menu_init(void);
uint8_t menu_display(uint8_t init, uint8_t startentry);
void    menu_resetlines(void);
uint8_t menu_addline(char *line);

#endif
