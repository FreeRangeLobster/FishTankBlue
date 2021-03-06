#include <SPI.h>




//https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/
//String handling in Arduino
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


String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
int stringLenght=0;
char cTemp;

String sCommand;
String sFirstParameter;



int nParameter=0;




/* 
 * These global variables enable assembly of the user serial 
 * input command.
 */

/*******************************************LOW LEVEL ROUTINES**************************************
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
    delay(2);
  }
  digitalWrite(SS, HIGH);
  not_busy();
}


 
/*
 * See the timing diagram in section 9.2.21 of the
 * data sheet, "Page Program (02h)".
 */
void _write_page(word page_number, byte *page_buffer) {
  Serial.println("_write_Page");
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
    //Serial.println((char)page_buffer[i]);
    delay(10);
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

/*******************************************END OF LOW LEVEL ROUTINES**************************************/
//Mid level

void read_page(unsigned int page_number) {
  char buf[80];
  sprintf(buf, "command: read_page(%04xh)", page_number);
  Serial.println(buf);
  byte page_buffer[256];
  _read_page(page_number, page_buffer);
  print_page_bytes(page_buffer);
  Serial.println("Ready");
}

  
void read_page_ascii(unsigned int page_number) {
  char buf[256];
  sprintf(buf, "Command: read_page(%04xh)", page_number);
  Serial.println(buf);
  byte page_buffer[256];
  _read_page(page_number, page_buffer);
  print_page_ascii(page_buffer);
  Serial.println("Ready");
}

void print_page_ascii(byte *page_buffer) {
  Serial.println("Print Page AScii");
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      Serial.print((char)page_buffer[(i * 16) + j]);
      Serial.flush();
    }
    Serial.println(i);  
  }
}

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
//Mid Level Routines

void erase_sector(unsigned int sector){//256 sectors of 4k bytes, 1 sector = 16 pages
  Serial.print("erasing sector: "); Serial.println(sector);
  _erase_sector(sector);
  Serial.println("ready");
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

void write_bytes(word page, byte offset, byte Array[], int len) {
  char buf[80];
  byte page_data[256];
  _read_page(page, page_data);
  for(int i=0;i<=len;i++){
      page_data[offset+i]=Array[i];
      delay(10);
  }

  //For debugging purposes
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 16; ++j) {
      Serial.print((char)page_data[(i * 16) + j]);
    }
    Serial.println(i);  
  }  

  Serial.println("WriteBytes debugging"); 
  
  _write_page(page, page_data);
  Serial.println("Ready");
  
  
}

//Working here
//AddEvent H0010150TUE11T;
//AddEvent Heyllama;

void Write_Event(){
  word nPage=0;
  byte nOffset=0;
  byte page_buffer[256];
  char cFirstParameter[16];
  byte byFirstParameter[16];
  //char cEvent[16];
  Serial.println("Check point 1");
  delay(200);
  Serial.print("Parameter1 in String:  ");
  Serial.println(sFirstParameter);
  sFirstParameter.toCharArray(cFirstParameter,16);
  delay(200);
  Serial.println("Coping Across to char array");
  Serial.print("Array:  -");
  
  _read_page(0, page_buffer);
  Serial.println("Check point 2: Conversion");

  for(int i=0;i<=13;i++){
    byFirstParameter[i]=cFirstParameter[i];
    Serial.print(byFirstParameter[i]);
    }
  
  nOffset=0;
  nPage=0;
 Serial.println("Check point 2: End of Conversion");

  
  while(page_buffer[nOffset]=='H'){
      //Serial.println("Check point 3");
      //Serial.println("Loop");
      nOffset=nOffset+16;
      delay(10); 
      //Serial.print("-Position Available: ");
      delay(10);
      //Serial.println(nOffset);
      //if (nOffset>=255){
      //  nPage++;
      //  nOffset=0;
      //  _read_page(nPage, page_buffer);
      //  delay(10); 
      //  Serial.println("Page");  
      //}
      delay(100);       
    }
  
  Serial.println(nOffset);
  Serial.flush();
  Serial.println("Check point 3");
  delay(500);
  //byte Array[10]={'H','E','L','L','O'};
  write_bytes(0,nOffset,byFirstParameter,14);
  delay(1000);
  Serial.println("Check point 4");

  //Add Tail
  //'T'
  //write_byte(nPage,nOffset,'T');


  //read_page_ascii(0);
  //read_page_ascii(1); 
  
}

void Write_FullArray(){
  byte FullArray[256]={'H','0','0','0','2','3','5','0','M','O','N','1','1','T','X','X',
  'H','0','0','1','2','3','5','5','T','U','E','1','0','T','X','X',
  'H','0','0','2','0','0','0','0','W','E','D','1','1','T','X','X',
  'H','0','0','3','0','0','0','5','T','H','U','1','0','T','X','X',
  'H','0','0','4','0','0','1','0','F','R','I','1','1','T','X','X',
  'H','0','0','5','0','0','1','5','S','A','T','1','0','T','X','X',
  'H','0','0','6','0','0','2','0','S','U','N','1','1','T','X','X',
  'H','0','0','7','0','0','2','5','M','O','N','1','0','T','X','X',
  'H','0','0','8','0','0','3','0','T','U','E','1','1','T','X','X',
  'H','0','0','9','0','0','3','5','W','E','D','1','0','T','X','X',
  'H','0','1','0','0','0','4','0','T','H','U','1','1','T','X','X',
  'H','0','1','1','0','0','4','5','F','R','I','1','0','T','X','X',
  'H','0','1','2','0','0','5','0','S','A','T','1','1','T','X','X',
  'H','0','1','3','0','0','5','5','S','U','N','1','0','T','X','X',
  'H','0','1','4','0','1','0','0','M','O','N','1','1','T','X','X',
  'H','0','1','5','0','1','0','5','T','U','E','1','0','T','X','X'};
  _write_page(0, FullArray);
}


//.nID=ConvertToInt(pbuffer[i],pbuffer[i+1],pbuffer[i+2]
int ConvertToInt(char cBit2,  char cBit1, char cBit0  ){
  //Calculates the integer from a 3 digit string
  //There is no error handling aiming to reduce size of the program
  //MSB= cBit2, LSB cBit0
  int nResult=0;
  nResult=  ((cBit2-48)*100) + ((cBit1- 48)*10) + (cBit0- 48);
  return nResult;
 }



//if (IntToDay(Event[i].nDay,cNumbers)==true)
boolean IntToDay( int nDay,char *cDay){
  
  switch(nDay){
    case 1:
      cDay[0]='M';
      cDay[1]='O';
      cDay[2]='N';
      return true;
    break;

    case 2:
      cDay[0]='T';
      cDay[1]='U';
      cDay[2]='E';
      return true;
    break;

    case 3:
      cDay[0]='W';
      cDay[1]='E';
      cDay[2]='D';
      return true;
    break;

    case 4:
      cDay[0]='T';
      cDay[1]='H';
      cDay[2]='U';
      return true;
    break;

    case 5:
      cDay[0]='F';
      cDay[1]='R';
      cDay[2]='I';
      return true;
    break;

    case 6:
      cDay[0]='S';
      cDay[1]='A';
      cDay[2]='T';
      return true;
    break;

    case 7:
      cDay[0]='S';
      cDay[1]='U';
      cDay[2]='N';
      return true;
    break;

    default:
      return false;
    break;
    
    }
  
  }





//Current date= '0','0','1','5','S','A','T'
void CheckEvent(int nHour, int nMinutes, int Day){
  word nPage=0;
  byte nOffset=0;
  byte page_buffer[256];
  
  //Read Memory

  //Loop for checking
  //Convert minutes, days and hour
  //if nHour=nhour_Memory ,nDay=nDay_Memory ,  if nMinutes_Memory<nMinutes<nMinutes_Memory , 
    //Update Output
  //Endloop
  
/*  
 *    if 
 *    'H','0','0','5','0','0','1','5','S','A','T','1','0','T','X','X',
 *    
 *   -Get hour,minute,Day
 *   
 */
  
   _read_page(nPage, page_buffer);
  while(page_buffer[nOffset]=='H'){

    //  if (page_buffer[nOffset]==CurrentDate[0] &&
    //      page_buffer[nOffset]==CurrentDate[1]){
    //        //Update output
    //   }
  
          
      
      //Serial.println("Check point 3");
      //Serial.println("Loop");
      nOffset=nOffset+16;
      delay(10); 
      //Serial.print("-Position Available: ");
      delay(10);
      //Serial.println(nOffset);
      //if (nOffset>=255){
      //  nPage++;
      //  nOffset=0;
      //  _read_page(nPage, page_buffer);
      //  delay(10); 
      //  Serial.println("Page");  
      //}
      delay(100);       
    }
 

}


void write_array(char Array[], int nPage){
  int i=0;
  int nIndexArray=0;
  int nIndexPages=nPage;
  int nIndexOffset=0;
 
  //Serial.println ("Starting to write array");
  //Find
  while(Array[nIndexArray]!=4 ){    
    //Serial.print (Array[nIndexArray]);
    write_byte(nIndexPages,nIndexOffset,Array[nIndexArray]);
    nIndexArray++;
    nIndexOffset++;
    
    if(nIndexOffset>=255){
      nIndexOffset=0;
      nIndexPages++;    
    }    
    
 }
 //Writes the end of file, need to valida
 //write_byte(nPage,nIndexOffset,4);
 Serial.println ("Array saved to flash");  
}


//
void setup() {
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setBitOrder(MSBFIRST);
  
  Serial.begin(9600);
  
  Serial.println("");
  Serial.println("Ready"); 
  get_jedec_id();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  sFirstParameter.reserve(50);

  //byte Array[10]={'N','E','W','L','I'};
  //write_bytes(0,60,Array,4);

}

void loop() {
  // print the string when a newline arrives:
   digitalWrite(LED_BUILTIN, HIGH);
  if (stringComplete) {
    Serial.println(sCommand);
    // clear the string:

    if(sCommand=="ShowEvents"){
      Serial.println("ShowEvents-OK");
      stringLenght=sCommand.length();
      Serial.println(stringLenght);
      read_page_ascii(0);
      read_page_ascii(1);
      }
    
    else if(sCommand=="AddEvent"){
      Serial.println("AddEvent-OK");
      delay(1000);
      Serial.println("Checking parameter :"); 
      delay(1000);     
      Write_Event();
      Serial.println("All good until here"); 
      }

    else if(sCommand=="EraseEvents"){
      Serial.println("EraseEvents-OK");
      stringLenght=sCommand.length();
      Serial.println(stringLenght);
      erase_sector(0);
      }

    else if (sCommand=="ReadPage") {
      read_page(0);
      read_page(1);
    } 

    else if (sCommand=="WriteTemplate") {
        Serial.println("Writing Events");
        //char MyArray[33]={ 'H','0','0','0','!','0','1','3','0','M','O','N','1','1','!','T',    
        //                    'H','0','1','0','!','1','2','3','5','T','U','E','1','0','!','T',4};
        //write_array(MyArray,0);
        Write_FullArray();
        Serial.println("Done");
    }

    else{
      Serial.println("Not Found");
      stringLenght=sCommand.length();
      Serial.println(stringLenght);
      //cTemp=sCommand.charAt(1);
      Serial.println("Character: ");
      Serial.println(cTemp,DEC);
      sCommand = "";
      sFirstParameter="";
      stringComplete = false;
      Serial.flush();
      }
    
    sCommand = "";
    sFirstParameter="";
    stringComplete = false;
    Serial.flush();
  }

  digitalWrite(LED_BUILTIN, LOW);
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    int nIndex=0;
   if (inChar == ';') {
      stringComplete = true;
      nParameter=0;
      sCommand.trim();
      sFirstParameter.trim();
      Serial.flush();
      
   }
   else {
      if (nParameter==0){
          sCommand += inChar;
        }
      
      if (nParameter==1){
          sFirstParameter += inChar;
          nIndex++;
       }
   }    
   
   if(inChar==' ') {
    nParameter++;
   }
   
   
  }   
}

