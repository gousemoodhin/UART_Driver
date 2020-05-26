# UART_Driver
Universal Asynchronous Receiver/Transmitter (UART).

Protocol structure
------------------
1 start bit, 8 data bits, no parity, and 1 stop bit

Important terms
---------------
- Bandwidth: The amount of data or useful information transmitted per second.
- Frame: It is the smallest complete unit of serial transmission(1 start bit + 8 bit data + 1 stop bit).
- Baud rate: Total number of bits transmitted per second.
- Bit time = 1 / Baud rate

- we see that 10 bits are sent for every byte of usual data
  bandwidth = baud rate / 10
  
- This channel is classified as "Full Duplex", because transmission can occur in both directions
  simultaneously.
  
- Each UART will have a baud rate control register, which we use to select the transmission rate.
  Each device is capable of creating its own serial clock with a transmission frequency approximately
  equal to the serial clock in the computer with which it is communicating.

- busy-wait synchronization to write I/O programs that send and receive data using the UART

Transmitter (asynchronous mode)
-------------------------------
- It has
  10-bit shift register
  16-element FIFO
  Transmit data register (UART0_DR_R)

- software -> UART0_DR_R -> FIFO -> shift register

- Software will first check to make sure the transmit FIFO is not full (it will wait if TXFF is 1)
  and then write to the transmit data register.
  
- The transmit data register is write only, which means the software can write to it
  (to start a new transmission) but cannot read from it.

- When a new byte is written to UART0_DR_R , it is put into the transmit FIFO. Byte by byte, the
  UART gets data from the FIFO and loads them into the 10-bit transmit shift register. The 10-bit
  shift register includes a start bit, 8 data bits, and 1 stop bit. Then, the frame is shifted out
  one bit at a time at a rate specified by the baud rate register. If there are already data in the
  FIFO or in theshift register when the UART0_DR_R is written, the new frame will wait until the
  previous frames have been transmitted, before it too is transmitted. The software can actually
  write 16bytes to the UART0_DR_R

Receiver (asynchronous mode)
----------------------------

- shift register -> FIFO -> UART0_DR_R -> software

- UART0_DR_R , is read only, which means write operations to this address have no effect on this
  register (recall write operations activate the transmitter). The receiver obviously cannot start a
  transmission, but it recognizes a new frame by its start bit. The bits are shifted in using the
  same order as the transmitter shifted them out.

- The Receive FIFO empty flag, RXFE, is clear when new input data are in the receive FIFO. When the
  software reads from UART0_DR_R , data are removed from the FIFO. When the FIFO becomes empty, the
  RXFE flag will be set, meaning there are no more input data.

