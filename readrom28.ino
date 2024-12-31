/*
=============================================================================================
  Dumps the contents of 28-pin MASK ROM
  Chip used: Super Mario Bros (NES) CHR-ROM
=============================================================================================
*/

/*useful*/
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;

//shift register pins
UINT8 SER = 2;  //assigned to gpio 2
UINT8 SRCLK = 3;  //gpio 3
UINT8 SRCLR = 4;  //unused - tied to +5V
UINT8 RCLK = 5; //gpio 5
UINT8 OE = 6; //unused - tied to GND

/*Data pins*/
UINT8 D0 = 10;  //D0 on the ROM chip is connected to gpio 10
UINT8 D7 = 17;  //D7 -> gpio 17

void enableLatch(bool value)
{
  digitalWrite(RCLK, (UINT8)(value ? HIGH : LOW));
}

void setup() {
  Serial.begin(9600);

  while(!Serial); //wait for serial ready

  Serial.println("\nSETTING UP PINS");

  /*
  SER = serial data line
  SRCLK = serial clock
  RCLK = store register
  */
  pinMode(SER, OUTPUT);
  pinMode(SRCLK, OUTPUT);
  pinMode(RCLK, OUTPUT);
}

/*
NOTE:
- output formatted string using snprintf
  - read a chunk at a time because snprintf is performance-heavy
*/

//place the 16bit address onto the address pins
void writeAddress(UINT16 addr)
{
  UINT8 high = (addr & 0xFF00) >> 8;
  UINT8 low = (addr & 0x00FF);

  enableLatch(false);
  shiftOut(SER, SRCLK, MSBFIRST, high);
  shiftOut(SER, SRCLK, MSBFIRST, low);
  enableLatch(true);
}

//Read data from the 8 data pins bit by bit
UINT8 readData()
{
  UINT8 data = 0x00;
  for(int i = D7;i >= D0;i--)  //read data one bit/pin at a time
  {
    data <<= 1;
    data |= (digitalRead(i) & 1);    
  }
  return data;
}

//read from address
UINT8 readDataAt(UINT16 addr)
{
  writeAddress(addr);
  return readData();
}

void loop() 
{
  char str[0x100];  //output string
  UINT8 data;
  UINT8 buffer[8];  //hold the last 8 bytes read
  UINT8 buf_index;  //index into buffer (0-7)

  UINT16 ADDR_START = 0x0000; //read from here
  UINT16 ADDR_END = 0x2000; //to here

  for(UINT16 addr = ADDR_START;addr < ADDR_END;addr++)
  {
    buf_index = addr % sizeof(buffer);
    if(buf_index == 0 && addr != 0x0000)  //show contents and clear buffer every 8 iterations
    {
      snprintf(str, sizeof(str), "$%04X | %02X %02X %02X %02X   %02X %02X %02X %02X", 
        (addr - 8), 
        buffer[0], buffer[1], buffer[2], buffer[3], 
        buffer[4], buffer[5], buffer[6], buffer[7]);
      memset(buffer, 0x00, sizeof(buffer)); //redundancy
      Serial.println(str);
    }
    data = readDataAt(addr);  //read
    buffer[buf_index] = data;        
  }  
  
  while(1);
}
