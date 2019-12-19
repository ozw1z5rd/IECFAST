/* sd2iec - SD/MMC to Commodore serial bus interface/controller
   Copyright (C) 2007-2009  Ingo Korb <ingo@akana.de>

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


   display.c: Remote display interface

*/

#include <avr/pgmspace.h>
#include <string.h>
#include "config.h"
#include "buffers.h"
#include "eeprom.h"
#include "fatops.h"
#include "i2c.h"
#include "iec.h"
#include "parser.h"
#include "ustring.h"
#include "utils.h"
#include "display.h"

#include "uart.h"

/* All menu entries are converted from PETSCII on the display side */
static const PROGMEM char systemmenu[] = {
  "\xc3""HANGE DIRECTORY\0"
  "\xc3""HANGE ADDRESS\0"
  "\xd3""TORE SETTINGS\0"
  "\xc3""ANCEL\0"
};
#define SYSMENU_CHDIR  0
#define SYSMENU_CHADDR 1
#define SYSMENU_STORE  2
#define SYSMENU_CANCEL 3

static enum { MENU_NONE = 0, MENU_SYSTEM, MENU_CHDIR, MENU_CHADDR } menustate;

static uint8_t displaybuffer[CONFIG_DISPLAY_BUFFER_SIZE];

uint8_t display_found;

void display_send_prefixed(uint8_t cmd, uint8_t prefixbyte, uint8_t len, uint8_t *buffer) {
  displaybuffer[0] = prefixbyte;
  memcpy(displaybuffer+1, buffer, min(sizeof(displaybuffer)-1, len));
  i2c_write_registers(DISPLAY_I2C_ADDR, cmd, min(len+1,sizeof(displaybuffer)), displaybuffer);
}

static void menu_chdir(void) {
  struct cbmdirent dent;
  dh_t    dh;
  path_t  path;
  int8_t  res;

  menustate = MENU_CHDIR;
  display_menu_reset();
  ustrcpy_P(displaybuffer,PSTR("\xc3""ANCEL"));
  display_menu_add(displaybuffer);

  displaybuffer[0] = '_';
  displaybuffer[1] = 0;
  display_menu_add(displaybuffer);

  path.part = current_part;
  path.fat  = partition[current_part].current_dir;

  if (opendir(&dh, &path)) {
    menustate = MENU_NONE;
    return;
  }

  while (1) {
    res = readdir(&dh, &dent);

    if (res > 0)
      return;
    if (res < 0)
      break;

    /* Skip hidden files and only add image files on FAT */
    if (!(dent.typeflags & FLAG_HIDDEN)) {
      if ((dent.typeflags & TYPE_MASK) == TYPE_DIR) {
        display_menu_add(dent.name);
      } else if (partition[current_part].fop == &fatops &&
                 dent.realname[0] == 0 &&
                 check_imageext(dent.name) != IMG_UNKNOWN) {
        display_menu_add(dent.name);
      }
    }
  }
  display_menu_show(0);
}

static void menu_chaddr(void) {
  uint8_t i;

  menustate = MENU_CHADDR;
  memset(displaybuffer,0,3);
  display_menu_reset();
  for (i=4;i<31;i++) {
    if (i < 10)
      displaybuffer[0] = ' ';
    else
      displaybuffer[0] = '0' + i/10;
    displaybuffer[1] = '0' + i%10;
    display_menu_add(displaybuffer);
  }
  display_menu_show(device_address-4);
}

void display_service(void) {
  if (menustate == MENU_NONE) {
    /* No menu active, upload+run system menu */
    uint8_t i;

    /* Dummy read to reset the interrupt line */
    i2c_read_register(DISPLAY_I2C_ADDR, DISPLAY_MENU_GETSELECTION);

    display_menu_reset();
    i = 0;
    while (pgm_read_byte(systemmenu+i)) {
      ustrcpy_P(displaybuffer, systemmenu+i);
      display_menu_add(displaybuffer);
      i += ustrlen(displaybuffer)+1;
    }
    display_menu_show(0);
    menustate = MENU_SYSTEM;

  } else if (menustate == MENU_SYSTEM) {
    /* Selection on the system menu */
    uint8_t sel = i2c_read_register(DISPLAY_I2C_ADDR, DISPLAY_MENU_GETSELECTION);

    switch (sel) {
    case SYSMENU_CHDIR:
      menu_chdir();
      break;

    case SYSMENU_CHADDR:
      menu_chaddr();
      break;

    case SYSMENU_STORE:
      menustate = MENU_NONE;
      write_configuration();
      break;

    case SYSMENU_CANCEL:
      menustate = MENU_NONE;
      return;
    }
  } else if (menustate == MENU_CHADDR) {
    /* New address selected */
    uint8_t sel = i2c_read_register(DISPLAY_I2C_ADDR, DISPLAY_MENU_GETSELECTION);

    device_address = sel+4;
    menustate = MENU_NONE;
    display_address(device_address);
  } else if (menustate == MENU_CHDIR) {
    /* New directory selected */
    uint8_t sel = i2c_read_register(DISPLAY_I2C_ADDR, DISPLAY_MENU_GETSELECTION);
    path_t path;

    menustate = MENU_NONE;
    if (sel == 0)
      /* Cancel */
      return;

    /* Can't use displaybuffer here because it's used by display_send_prefixed, used by fat_chdir */
    if (sel == 1) {
      /* Previous directory */
      entrybuf[0] = '_';
      entrybuf[1] = 0;
    } else {
      i2c_read_registers(DISPLAY_I2C_ADDR, DISPLAY_MENU_GETENTRY, sizeof(entrybuf), entrybuf);
    }

    path.part = current_part;
    path.fat  = partition[current_part].current_dir;
    chdir(&path, entrybuf);
  }
}

uint8_t display_init(uint8_t len, uint8_t *message) {
  display_found = !i2c_write_registers(DISPLAY_I2C_ADDR, DISPLAY_INIT, len, message);
  return display_found;
}
