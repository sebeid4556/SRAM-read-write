# SRAM-read-write
Circuitry & code to perform read/write operations on SRAM chip<br/>

Chip Used: 8k x 8bit 6264 SRAM chip salvaged from FAMICOM game cartridge (labeled with masking tape that reads "SRAM")<br/>

# Pics
![alt text](pics/board.jpeg?raw=true)
![alt text](pics/board-top.jpeg?raw=true)

# Pin Descriptions:
GPIO  2 -> [SER] (Serial data line)<br/>
      3 -> [SRCLK] (Serial clock)<br/>
      5 -> [RCLK] (Latch)<br/>
  10-17 -> Data pins<br/>

Pins 2, 3, 5 are OUTPUT, and 10-17 are IN/OUT on the Arduino.<br/>

# What it does:
It programs the chip with values ranging from 0x00 to 0x30. Then it reads back the data that was written to the chip and outputs it to the serial monitor. <br/>

# Technical Explanation:
1. Write to chip<br/>
  - set address pins through the use of two cascaded 8-bit shift-registers<br/>
  - enable the chip and set to write mode (CE high, /WE low)<br/>
  - set data pins to desired value<br/>
  - disable chip<br/>
  - (repeat)<br/>
2. Read from chip<br/>
  - enable chip and set to read mode (CE high, /WE high)<br/>
  - set address pins to desired location<br/>
  - read data pins<br/>
  - (repeat)<br/>

# Demonstration
The LEDs are connected to the data pins in parallel so that values written are visible in real time (with software delay)
![alt text](pics/demo.gif?raw=true)

# Output
![alt text](pics/output.png?raw=true)