# ROM-Dumper
Dump the contents of a 28-pin mask ROM using an Arduino UNO R3

#Pin Descriptions:
GPIO  2 -> [SER] (Serial data line)
      3 -> [SRCLK] (Serial clock)
      5 -> [RCLK] (Latch)
  10-17 -> Data pins

Pins 2, 3, 5 are OUTPUT and 10-17 are INPUT on the Arduino.

#How it works:
-The address to read from is placed onto the address pins of the ROM chip through shift registers
  -two 74HC595 8-bit shift registers cascaded to be able to output up to 16 bits of data
-Simply read the data pins and voila

This design allows for outputting more lines of data than there are pins available on the board through the use of shift registers.

![alt text](https://github.com/sebeid4556/ROM-Dumper/blob/main/pics/board.jpeg?raw=true)

Output:<br/>
![alt text](https://github.com/sebeid4556/ROM-Dumper/blob/main/pics/output.png?raw=true)