/*
=============================================================================================
  Test SRAM read/write operations
  Chip used: 8Kb 6264 SRAM from unknown FAMI-COM cartridge
=============================================================================================
*/

/*useful*/
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef char CHAR;
typedef bool BOOL;

//shift register pins
const static UINT8 SER = 2;  //assigned to gpio 2
const static UINT8 SRCLK = 3;  //gpio 3
const static UINT8 SRCLR = 4;  //unused - tied to +5V
const static UINT8 RCLK = 5; //gpio 5
const static UINT8 OE = 6; //unused - tied to GND
//SRAM pins
const static UINT8 WE = 6;
const static UINT8 CE2 = 7; //chip enable (active high)
const static UINT8 CE1; //unused - tied to GND (active low)

/*Data pins*/
const static UINT8 D0 = 10;  //D0 on the ROM chip is connected to gpio 10
const static UINT8 D7 = 17;  //D7 -> gpio 17

#define WE_WRITE 0  //writing to SRAM
#define WE_READ 1 //reading from SRAM

static UINT8 g_WE;  //current WE state

//=============================================================================================
//DEBUG MACROS
//=============================================================================================
//#define DEBUG

#ifdef DEBUG
#define LOG(s) Serial.println(s)
#endif
#ifndef DEBUG
#define LOG(s)
#endif
//=============================================================================================

//Cannot continue execution. Loop indefinitely.
void fatalError(char *pErrorStr)
{
  const static char prefix[] = "[ERROR]: ";
  const static UINT32 max_msg_len = 0X100;
  const static char msg[max_msg_len] = {0x00};
  if(!pErrorStr)
  {
    Serial.println("[ERROR]: null pointer passed to fatalError()");
  }
  else
  {
    strncat(msg, prefix, strlen(prefix));
    strncat(msg, pErrorStr, strlen(pErrorStr));
    Serial.println(msg);
  }
  while(1);
}

inline UINT8 getMode()
{
  return g_WE;
}

//set data pins to either read/write
//pass INPUT/OUTPUT macros
void setDataDir(int io)
{
  for(int i = D0;i <= D7;i++)
  {
    pinMode(i, ((io & 1) ? OUTPUT : INPUT));
  }
}

//set READ/WRITE mode on SRAM
void setMode(UINT8 op)
{
  if(op == WE_WRITE)  //0
  {
    LOG("Setting mode to WRITE");
    g_WE = op;  
    setDataDir(OUTPUT);
    digitalWrite(WE, LOW);
  }
  else if(op == WE_READ)  //1
  {
    LOG("Setting mode to READ");
    g_WE = op;  
    setDataDir(INPUT);
    digitalWrite(WE, HIGH);
  }
  else
  {
    fatalError("invalid operation");
  }
}

//toggle the CE (active high) pin
void enableChip(BOOL val)
{
  digitalWrite(CE2, (val ? HIGH : LOW));
}

//toggle shift register latch
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

  pinMode(WE, OUTPUT);  //write enable (active low)
  pinMode(CE2, OUTPUT); //chip enable (active high)
}

//place the 16bit address onto the address pins
void setAddress(UINT16 addr)
{
  UINT8 high = (addr & 0xFF00) >> 8;
  UINT8 low = (addr & 0x00FF);

  enableLatch(false);
  shiftOut(SER, SRCLK, MSBFIRST, high);
  shiftOut(SER, SRCLK, MSBFIRST, low);
  enableLatch(true);
}

//Read data from the 8 data pins in MSB first
UINT8 readDataPins()
{
  if(getMode() != WE_READ) fatalError("attempting to read in WRITE mode");
  UINT8 data = 0x00;
  for(int i = D7;i >= D0;i--)  //read data one bit/pin at a time
  {
    data <<= 1;
    data |= (digitalRead(i) & 1);    
  }
  return data;
}

void writeDataPins(UINT8 data)
{
  static CHAR msg[0x10] = {0x00};
  snprintf(msg, 0x10, "writing $%02X", data);
  LOG(msg);
  if(getMode() != WE_WRITE) fatalError("attempting to write in READ mode");
  UINT8 test = 0x00;
  for(int i = D0, j = 0;i <= D7, j < 8;i++, j++)
  {
    digitalWrite(i, ((data >> j) & 1) ? HIGH : LOW);
    test |= (data & (1 << j));
  }
  if(test != data)  //making sure correct values are being output on the pins
  {
    Serial.println("[ERROR]: test and data mismatch");
  }
}

void clearDataPins()
{  
  for(int i = D0;i <= D7;i++)
  {
    digitalWrite(i, LOW);
  }
}

//read from address
UINT8 readDataAt(UINT16 addr)
{
  enableChip(true);  
  setMode(WE_READ);
  setAddress(addr);    
  UINT8 data = readDataPins();
  enableChip(false);
  return data;
}

void writeDataAt(UINT16 addr, UINT8 data)
{
  setAddress(addr);
  enableChip(true);
  setMode(WE_WRITE);  //wake up the chip after setting address for precaution
  clearDataPins();
  writeDataPins(data);
  enableChip(false);  
}

void dumpRange(UINT16 from, UINT16 to, UINT8 **pOut)
{  
  static char str[0x100];  //output string
  UINT8 data;
  static UINT8 buffer[8];  //hold the last 8 bytes read
  UINT8 buf_index;  //index into buffer (0-7)
  UINT32 out_index = 0;

  snprintf(str, sizeof(str), "Dumping from $%04X-$%04X", from, to);
  Serial.println(str);

  for(UINT16 addr = from;addr < to;addr++)
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
    pOut[out_index] = data;
    out_index++;
  }
}

void loop() 
{  
  Serial.println("Writing values");
  UINT8 n = 0;
  for(UINT16 i = 0;i < 0x30;i++)
  {
    Serial.println(n, HEX);
    writeDataAt(i, n++);    
    delay(50);    
  }
  UINT8 buffer[0x30] = {0x00};
  memset(buffer, 0x00, 0x30);
  dumpRange(0x0000, 0x0030, (UINT8 **)&buffer);  

  while(1);
}
