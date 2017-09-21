/* This arduino sketch was developed aiming to read, save, query edit, and delete 
 * data using a flash memory. it was implemented using arduino. This development
 * is a module of the fishtank embedded controller. 
 * 
 * Add Event
 * Delete Event
 * Check Event
 * Update Event
 * Show Events
 * 
 * Set Output
 * Clear Output
 * Get Version
 * Set Status
 * 
 * 
 * Links
 * Flash memory sample code and input/output voltage clamp
 * http://www.instructables.com/id/How-to-Design-with-Discrete-SPI-Flash-Memory/
 * 
 * Flash memory datasheet
 * https://cdn-shop.adafruit.com/datasheets/W25Q80BV.pdf
 * 
 * 
 * Special thanks to the designer of the template Peter J. Torelli
 * 
 * 
 * AddEvent(Day,Time, Output, state,Status)
 * -Load flash events to local memory
 * -Add new event into local memory
 * -Delete sector of flash
 * -Save local memory to flash
 * 
 * DeleteEvent(EventID)
 * -Load flash events to local memory
 * -copy last event to temporal event variable
 * -rewrite temporal variable on the event to delete
 * -Delete sector of flash
 * -Save local memory to flash
 * 
 * CheckEvent(Day,Time)
 * -Load flash events to local memory
 * -Verify day and time
 * -Update outputs/global variables
 * 
 * UpdateEvent(ID, Day,Time, Output, state,Status)
 * -Load flash events to local memory
 * -Using ID to locate event,Update event details
 * -Delete sector of flash
 * -Save local memory to flash
 * 
 * ShowEvents()
 * -Load flash events to local memory 
 * -Print events using serial
 * 
 * SetOutput(Output No)
 * -Update global variables
 * -Wait for main loop to update values
 * 
 * ClearOutput(OutputNo)
 * -Update global variables
 * -Wait for main loop to update values
 * 
 * GetVersion()
 * -Print version of the development
 * 
 * SetStatus(Run/Offline/Edition)
 * -Sets the status of the system
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

typedef struct 
  {
      int nID;
      boolean bEventStatus;
      int nMinutes;
      int nHour;
      int nDay;
      int nOutput;
      int nOutputState;
      int nRes;
  } Event_Type;

typedef struct 
  {
      int nOutput;
      boolean bState;
  } Channel_Type;


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

void print_page_ascii(byte *page_buffer) {
  char buf[10];
   Serial.println("Hello print page ascii");
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      //sprintf(buf, "%02x", page_buffer[i * 16 + j]);
      sprintf(buf, "%c", page_buffer[i * 16 + j]);
      Serial.print(buf);
    }
    Serial.println(i);  
  }
  //read_events(page_buffer);
  
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

void read_page(unsigned int page_number) {
  char buf[80];
  sprintf(buf, "command: read_page(%04xh)", page_number);
  Serial.println(buf);
  byte page_buffer[256];
  _read_page(page_number, page_buffer);
  print_page_bytes(page_buffer);
  Serial.println("Ready");
}

void read_page2(unsigned int page_number, byte *page_buffer) {
  char buf[80];
  sprintf(buf, "command: read_page(%04xh)", page_number);
  Serial.println(buf);
 
  _read_page(page_number, page_buffer);
  print_page_bytes(page_buffer);
  Serial.println("Ready");
}


void read_page_ascii(unsigned int page_number) {
  char buf[80];
  sprintf(buf, "command: read_page(%04xh)", page_number);
  Serial.println(buf);
  byte page_buffer[256];
  _read_page(page_number, page_buffer);  
  print_page_ascii(page_buffer);
  Serial.println("Ready");
  
}

void erase_sector(unsigned int sector){//256 sectors of 4k bytes, 1 sector = 16 pages
  Serial.print("erasing sector: "); Serial.println(sector);
  _erase_sector(sector);
  Serial.println("ready");
}


//***********************Working here************************************//
void WriteEventsToFlash(Event_Type Event[]){
  int nPage=0;
  //Create Page header
  char cBuffer[1632]={ 1 ,'E','V','E','N','T',' ','F','I','L','E','S','T','A','R','T'};
  
  
  //Run the events loop
  //        -Translate information of events to mem format
  //        -add chars to the array
  // if there is still more data loop
  //    -Save information in memory page by page
  //    -Clear Buffer
  //    -Increase page number
  //
  //Create file tail
  
}


void update_sector(unsigned int sector,word page, byte offset, byte databyte ){//256 sectors of 4k bytes, 1 sector = 16 pages
  
  //Creates a buffer to save all the data
  byte nDataBuffer[4096];
 
  

  //Saving back up of the sector in the buffer

  //Updating the new values in the buffer

  //Writing 

  
  
  Serial.print("erasing sector: "); Serial.println(sector);
  _erase_sector(sector);
  Serial.println("ready");
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

void write_array(char Array[], int n){
  int i=0;
  Serial.println ("Starting to write array");
  //Find
  while(Array[i]!=4 ){   
    Serial.print (Array[i]);
    write_byte(1,i,Array[i]);
    i++;
 }
 Serial.println ("Finishing array");  
}


//Working here!!!!
void read_array(int Array[], int n){
  int i=0;
  Serial.println ("Hello");
  //Find
  while(Array[i]!=0){   
    Serial.println (Array[i]);
    write_byte(1,i,Array[i]);
    i++;
 }
}

void LoadEventsToMemory(int nIniPage,Event_Type *Event ){

   byte page_buffer[256];
   

  //Validation of parameters

  //Reads page from flash memory
  read_page2(1, page_buffer);

  //Reads events from the memory buffer and loads them into 
  //into the event structure array
  read_events(page_buffer, Event );
  
  }



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void read_events(byte *pbuffer, Event_Type *Event){
  //char buff[100];
  //Event_Type Event[20];
  int nIdEvent=0;
  int i=0;
  Serial.println("Read Events");

  //travels the array and fills an structur
  while( (i < 300) && (pbuffer[i] != 4)){
      //Serial.print(page_buffer[i]); 
      i=i+1;
      
      if(pbuffer[i]==2){
          i=i+1;
          //Event ID          
          Event[nIdEvent].nID=ConvertToInt(pbuffer[i],pbuffer[i+1],pbuffer[i+2]);
          Serial.print("Event Number in integer: ");
          Serial.println(Event[nIdEvent].nID);
          i=i+3;
          //Event Status
          Event[nIdEvent].bEventStatus=ConvertToInt('0','0',pbuffer[i]);
          Serial.print("Event Statud: ");
          Serial.println(Event[nIdEvent].bEventStatus);
          
          i=i+1;
          //Event Time Hour
           Event[nIdEvent].nHour=ConvertToInt(pbuffer[i+1],pbuffer[i],'0');
          Serial.print("Hour: ");
          Serial.println(Event[nIdEvent].nHour);
           i=i+2;
           //Event Time minutes
           Event[nIdEvent].nMinutes=ConvertToInt('0',pbuffer[i],pbuffer[i+1]);
          Serial.print("Minutes: ");
          Serial.println( Event[nIdEvent].nMinutes);
           i=i+2;
           
          //Event Day
           Event[nIdEvent].nDay=ConvertToDay(pbuffer[i],pbuffer[i+1],pbuffer[i+2]);
            i=i+3;
            
          //Event Output
           Event[nIdEvent].nOutput=ConvertToInt('0','0',pbuffer[i]);
           i=i+1;
           
          //Event OutputState
           Event[nIdEvent].nOutputState=ConvertToInt('0','0',pbuffer[i]);
           i=i+1;
           
          //Event Reserved
           Event[nIdEvent].nRes=ConvertToInt('0','0',pbuffer[i]);
           i=i+1;
           
           if(pbuffer[i]==3){
             Serial.println("EOE"); 
             nIdEvent++;
           }
       }
       
       else if(pbuffer[i]==4){
         Serial.println("EOF"); 
      }
       
       Serial.println(i);
   }
   Serial.print("Total Events: ");   
   Serial.print(nIdEvent);
   Serial.println();
 
   //Print all the array of structure
  for(short nCounter=0; nCounter<nIdEvent;nCounter++){
    Serial.print("Position: ");
    Serial.print(nCounter);
    Serial.print(' ');
    Serial.print(Event[nCounter].nID);
    Serial.print(' ');
    Serial.print(Event[nCounter].bEventStatus);
    Serial.print(' ');
     Serial.print(Event[nCounter].nHour);
    Serial.print(' ');
     Serial.print(Event[nCounter].nMinutes);
    Serial.print(' ');
     Serial.print(Event[nCounter].nDay);
     Serial.print(' ');
     Serial.print(Event[nCounter].nOutput);
     Serial.print(' ');
     Serial.print( Event[nCounter].nOutputState);
     Serial.println(' ');
   
  }
   

}

int ConvertToInt(char cBit2,  char cBit1, char cBit0  ){
  //Calculates the integer from a 3 digit string
  //There is no error handling aiming to reduce size of the program
  //MSB= cBit2, LSB cBit0
  int nResult=0;
  nResult=  ((cBit2-48)*100) + ((cBit1- 48)*10) + (cBit0- 48);
  return nResult;
  }
  
 int ConvertToDay(char cBit2, char cBit1, char cBit0){
 //Create the string 
 int nResult=0;
 short nAsciiSum;
 
 nAsciiSum= cBit2+cBit1+cBit0;
 
 
switch (nAsciiSum) {
    case 234:
      //Monday-> M(77)+O(79)+N(78)=234
      Serial.println("Monday"); 
      return 1;
      break;
      
    case 238:
      //Tuesday-> //T(84)+U(85)+E(69)=238
      Serial.println("Monday"); 
      return 2;
      break;
      
    case 224:
      //Wednesday-> //W(87)+E(69)+D(68)=224
      Serial.println("Monday"); 
      return 3;
      break;
    
    case 241:
      //Thurday-> //T(84)+H(72)+U(85)=241
      Serial.println("Monday"); 
      return 4;
      break;
    
    case 225:
      //Friday-> //F(70)+R(82)+I(73)=225
      Serial.println("Monday"); 
      return 5;
      break;
    
    case 232:
      //Saturday-> //S(83)+A(65)+T(84)=232
      Serial.println("Monday"); 
      return 6;
      break;
     
    case 246:
      Serial.println("Monday"); 
      return 7;
      //Sunday-> //S(83)+U(85)+N(78)=246
      
      break;  
      
    default: 
      // if nothing else matches, do the default
      // Return 0;
      return 0;
      Serial.println("No match in th eday"); 
    break;
  }
 
 
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
    }
    else {
      g_command += c;
    }    
  }
}

void UpdateChannel(Channel_Type &Channel){
  Channel.bState=1;
  if (Channel.nOutput==1){
  }

  else if (Channel.nOutput==2){}

  else if(Channel.nOutput==3){}

  else if((Channel.nOutput==4)){}

  else{}
  }

bool CheckForEvent(Event_Type *Now, Event_Type *Events, Channel_Type &Channel){
  //Now hold the information NOW, in other words current day, hour and minutes
  //Event holds the information regarding all the events in the memory
  //Channel has te information regarding what channel needs to be updated and to what state
   Channel.bState=1;
  return true;
  } 

void setup(void) {
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  Serial.begin(9600);
  Serial.println("");
  Serial.println("Ready"); 
  get_jedec_id();
  //read_page_ascii(1);

  //creates an array of structures
  Event_Type Event[20];
  Event_Type Now;
  Channel_Type Channel;
  LoadEventsToMemory(1,Event);
  Now.nMinutes=30;
  Now.nHour=10;
  Now.nDay=1;
  Channel.bState=0;
  
  if (CheckForEvent(&Now,Event,Channel)){
      Serial.println("Entered to the Check Event"); 
      if(  Channel.bState==1 ){
            Serial.println("Output=1");
        }
    }

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

     else if (g_command.startsWith("read_p_ascii")) {
      int pos = g_command.indexOf(" ");
      if (pos == -1) {
        Serial.println("Error: Command 'read_page' expects an int operand");
      } else {
        word page = (word)g_command.substring(pos).toInt();
        read_page_ascii(page);
      }
    }

  //JV added on 29/08/2017 to delete a 4kb sector
    else if (g_command.startsWith("erase_sector")) {
      int pos = g_command.indexOf(" ");
      if (pos == -1) {
        Serial.println("Error: Command 'read_page' expects an int operand");
      } else {
        word page = (word)g_command.substring(pos).toInt();
        //read_page(page);
        erase_sector(page);
      }
    }
    
    else if (g_command.startsWith("write_events")) {
     Serial.println("Writing Events");

  char MyArray[600]={ 1 ,'E','V','E','N','T',' ','F','I','L','E','S','T','A','R','T',
                        2 ,'0','0','0','1','1','0','3','0','M','O','N','1','1','x',3,
                        2 ,'0','0','1','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','2','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','3','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','4','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','5','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','6','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','7','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','8','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','0','9','1','1','0','3','5','T','U','E','1','0','x',3,
                        2 ,'0','1','0','1','1','0','3','5','T','U','E','1','0','x',3,
                       'E','V','E','N','T',' ','F','I','L','E',' ','E','N','D',' ',4,
                       'S','O','M','E','M','O','R','E','R','U','B','I','S','H','x','x'};

   
                       
     /* char MyArray[800]={ 1 ,'E','V','E','N','T',' ','F','I','L','E','S','T','A','R','T',
                          2 ,'0','0','0','1','0','1','3','0','M','O','N','1','1','x',3,
                          2 ,'0','0','1','1','0','2','3','5','T','U','E','1','0','x',3,
                          2 ,'0','0','2','1','0','3','4','0','W','E','D','2','1','x',3,
                          2 ,'0','0','3','1','0','4','4','5','T','H','U','2','0','x',3,
                          2 ,'0','0','4','1','0','5','5','5','F','R','I','3','1','x',3,
                          2 ,'0','0','5','1','0','6','6','0','S','A','T','3','0','x',3,
                          2 ,'0','0','6','1','0','7','0','5','S','U','N','4','1','x',3,
                          2 ,'0','0','7','1','0','8','1','0','M','O','N','4','0','x',3,
                          2 ,'0','0','8','1','0','9','1','5','T','U','E','1','1','x',3,
                          2 ,'0','0','9','1','1','0','2','0','W','E','D','1','0','x',3,
                          2 ,'0','1','1','1','1','1','2','5','T','H','U','2','1','x',3,
                          2 ,'0','1','2','1','1','2','3','0','F','R','I','2','0','x',3,
                          2 ,'0','1','3','1','1','3','3','5','S','A','T','3','1','x',3,
                          2 ,'0','1','4','1','1','4','4','0','S','U','N','3','0','x',3,
                          2 ,'0','1','5','1','1','5','4','5','M','O','N','4','1','x',3,
                          2 ,'0','1','6','1','1','6','5','0','T','U','E','4','0','x',3,
                          2 ,'0','1','7','1','1','7','5','5','W','E','D','1','1','x',3,
                          2 ,'0','1','8','1','1','8','1','0','T','H','U','1','0','x',3,
                          2 ,'0','1','9','1','1','9','1','5','F','R','I','2','1','x',3,
                          2 ,'0','2','0','1','2','0','2','0','S','A','T','2','0','x',3,
                          2 ,'0','2','1','1','2','1','2','5','S','U','N','3','1','x',3,
                          2 ,'0','2','2','1','2','2','3','0','M','O','N','3','0','x',3,
                          2 ,'0','2','3','1','2','3','3','5','T','U','E','4','1','x',3,
                          2 ,'0','2','4','1','0','0','4','0','W','E','D','4','0','x',3,
                          2 ,'0','2','5','1','0','1','4','5','F','R','I','1','1','x',3,
                          'E','V','E','N','T',' ','F','I','L','E',' ','E','N','D',' ',4,
                          'S','O','M','E','M','O','R','E','R','U','B','I','S','H','x','x'};
                   */
     write_array(MyArray,1);
     Serial.println("Done");
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
