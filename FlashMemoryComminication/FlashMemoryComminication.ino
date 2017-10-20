/*
    windbond_serial_debug.cpp 
    A simple program for the Arduino IDE to help familiarize you with
    using WinBond flash memory; can also be used to download the entire
    contents of a flash chip to a file via a serial port interface.
    
    Important bits of the code: the low-level flash functions (which 
    implement the timing diagrams in the datasheet), and a simple 
    serial-port command interface that allows you to talk to your 
    UNO with any generic terminal application (even the command line).
    
    Copyright 2014, Peter J. Torelli

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>   


Revisions:
    rev 2 - 21-SEP-2014.
    User 'fiaskow' pointed out that driving the WEL instruction after
    program and erase w/o waiting for the op to finish may be corrupting
    execution. Removed this code (also not needed b/c the WEL is already
    cleared after page write or chip erase).
*/

#include <SPI.h>
// SS:   pin 10
// MOSI: pin 11
// MISO: pin 12
// SCK:  pin 13

// WinBond flash commands
#define WB_WRITE_ENABLE       0x06
#define WB_WRITE_DISABLE      0x04
#define WB_CHIP_ERASE         0xc7
#define WB_READ_STATUS_REG_1  0x05
#define WB_READ_DATA          0x03
#define WB_PAGE_PROGRAM       0x02
#define WB_JEDEC_ID           0x9f
#define WB_SECTOR_ERASE       0x20

/* 
 * These global variables enable assembly of the user serial 
 * input command.
 */
boolean g_command_ready(false);
String g_command;

/*
 * print_page_bytes() is a simple helperf function that formats 256
 * bytes of data into an easier to read grid.
 */
void print_page_bytes(byte *page_buffer) {
  char buf[10];
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      sprintf(buf, "%02x", page_buffer[i * 16 + j]);
      Serial.print(buf);
    }
    Serial.println();
  }
}

/*
================================================================================
User Interface Routines
The functions below map to user commands. They wrap the low-level calls with 
print/debug statements for readability.
================================================================================
*/

/* 
 * The JEDEC ID is fairly generic, I use this function to verify the setup
 * is working properly.
 */
void get_jedec_id(void) {
  Serial.println("command: get_jedec_id");
  byte b1, b2, b3;
  _get_jedec_id(&b1, &b2, &b3);
  char buf[128];
  sprintf(buf, "Manufacturer ID: %02xh\nMemory Type: %02xh\nCapacity: %02xh",
    b1, b2, b3);
  Serial.println(buf);
  Serial.println("Ready");
} 

void chip_erase(void) {
  Serial.println("command: chip_erase");
  _chip_erase();
  Serial.println("Ready");
}

void erase_sector(unsigned int sector){//256 sectors of 4k bytes, 1 sector = 16 pages
  Serial.print("erasing sector: "); Serial.println(sector);
  _erase_sector(sector);
  Serial.println("ready");
}


void read_page(unsigned int page_number) {
  char buf[80];
  sprintf(buf, "command: read_page(%04xh)", page_number);
  Serial.println(buf);
  byte page_buffer[256];
  _read_page(page_number, page_buffer);
  print_page_bytes(page_buffer);
  Serial.println("Ready");
}

void read_all_pages(void) {
  Serial.println("command: read_all_pages");
  byte page_buffer[256];
  for (int i = 0; i < 4096; ++i) {
    _read_page(i, page_buffer);
    print_page_bytes(page_buffer);
  }
  Serial.println("Ready");
}

void write_byte(word page, byte offset, byte databyte) {
  char buf[80];
  sprintf(buf, "command: write_byte(%04xh, %04xh, %02xh)", page, offset, databyte);
  Serial.println(buf);
  byte page_data[256];
  _read_page(page, page_data);
  page_data[offset] = databyte;
  _write_page(page, page_data);
  Serial.println("Ready");
}

/*
================================================================================
Low-Level Device Routines
The functions below perform the lowest-level interactions with the flash device.
They match the timing diagrams of the datahsheet. They are called by wrapper 
functions which provide a little more feedback to the user. I made them stand-
alone functions so they can be re-used. Each function corresponds to a flash
instruction opcode.
================================================================================
*/

/*
 * See the timing diagram in section 9.2.35 of the
 * data sheet, "Read JEDEC ID (9Fh)".
 */
void _get_jedec_id(byte *b1, byte *b2, byte *b3) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(WB_JEDEC_ID);
  *b1 = SPI.transfer(0); // manufacturer id
  *b2 = SPI.transfer(0); // memory type
  *b3 = SPI.transfer(0); // capacity
  digitalWrite(SS, HIGH);
  not_busy();
}  

/*
 * See the timing diagram in section 9.2.26 of the
 * data sheet, "Chip Erase (C7h / 06h)". (Note:
 * either opcode works.)
 */
void _chip_erase(void) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_WRITE_ENABLE);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_CHIP_ERASE);
  digitalWrite(SS, HIGH);
  /* See notes on rev 2 
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_WRITE_DISABLE);
  digitalWrite(SS, HIGH);
  */
  not_busy();
}


// Deletes 4KB sector
void _erase_sector(word sector){
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(WB_WRITE_ENABLE);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(WB_SECTOR_ERASE);
  sector=sector*16;//sectors are in multiples of 16 pages
  SPI.transfer((sector >> 8) & 0xFF);//send byte from front 8 bits  
  SPI.transfer((sector >> 0) & 0xFF);//send byte from bottom 8 bits
  SPI.transfer(0); // send byte 0x00
  digitalWrite(SS, HIGH); 
  not_busy();
}

/*
 * See the timing diagram in section 9.2.10 of the
 * data sheet, "Read Data (03h)".
 */
void _read_page(word page_number, byte *page_buffer) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  SPI.transfer(WB_READ_DATA);
  // Construct the 24-bit address from the 16-bit page
  // number and 0x00, since we will read 256 bytes (one
  // page).
  SPI.transfer((page_number >> 8) & 0xFF);
  SPI.transfer((page_number >> 0) & 0xFF);
  SPI.transfer(0);
  for (int i = 0; i < 256; ++i) {
    page_buffer[i] = SPI.transfer(0);
  }
  digitalWrite(SS, HIGH);
  not_busy();
}
 
/*
 * See the timing diagram in section 9.2.21 of the
 * data sheet, "Page Program (02h)".
 */
void _write_page(word page_number, byte *page_buffer) {
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_WRITE_ENABLE);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_PAGE_PROGRAM);
  SPI.transfer((page_number >>  8) & 0xFF);
  SPI.transfer((page_number >>  0) & 0xFF);
  SPI.transfer(0);
  for (int i = 0; i < 256; ++i) {
    SPI.transfer(page_buffer[i]);
  }
  digitalWrite(SS, HIGH);
  /* See notes on rev 2
  digitalWrite(SS, LOW);  
  SPI.transfer(WB_WRITE_DISABLE);
  digitalWrite(SS, HIGH);
  */
  not_busy();
}

/* 
 * not_busy() polls the status bit of the device until it
 * completes the current operation. Most operations finish
 * in a few hundred microseconds or less, but chip erase 
 * may take 500+ms. Finish all operations with this poll.
 *
 * See section 9.2.8 of the datasheet
 */
void not_busy(void) {
  digitalWrite(SS, HIGH);  
  digitalWrite(SS, LOW);
  SPI.transfer(WB_READ_STATUS_REG_1);       
  while (SPI.transfer(0) & 1) {}; 
  digitalWrite(SS, HIGH);  
}

/*
 * This handy built-in callback function alerts the UNO
 * whenever a serial event occurs. In our case, we check
 * for available input data and concatenate a command
 * string, setting a boolean used by the loop() routine
 * as a dispatch trigger.
 */
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = (char)Serial.read();
    if (c == ';') {
      g_command_ready = true;
      g_command.trim();
    }
    else {
      g_command += c;
    }    
  }
}

void setup(void) {
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Ready"); 
}

/*
 * loop() dispatches the commands compiled by the serialEvent
 * parser callback. Some commands take multiple arguments, so
 * I have to split up the strings with some messy manipulation.
 */
void loop(void) {
  if (g_command_ready) {
    if (g_command == "get_jedec_id") {
      get_jedec_id();
    }
    else if (g_command == "chip_erase") {
      chip_erase();
    }
    else if (g_command == "read_all_pages") {
      read_all_pages();
    }
    // A one-parameter command...
    else if (g_command.startsWith("read_page")) {
      int pos = g_command.indexOf(" ");
      if (pos == -1) {
        Serial.println("Error: Command 'read_page' expects an int operand");
      } else {
        word page = (word)g_command.substring(pos).toInt();
        read_page(page);
      }
    }
    // A three-parameter command..
    else if (g_command.startsWith("write_byte")) {
      word pageno;
      byte offset;
      byte data;
      
      String args[3];
      for (int i = 0; i < 3; ++i) {
        int pos = g_command.indexOf(" ");
        if (pos == -1) {
          Serial.println("Syntax error in write_byte");
          goto done;
        }
        args[i] = g_command.substring(pos + 1);
        g_command = args[i];
      }
      pageno = (word)args[0].toInt();
      offset = (byte)args[1].toInt();
      data = (byte)args[2].toInt();
      write_byte(pageno, offset, data);
    }
    else {
      Serial.print("Invalid command sent: ");
      Serial.println(g_command);
    }
done:
    g_command = "";
    g_command_ready = false;
  }
}


