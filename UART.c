/* 
  - Simple device driver for the UART.
  - Runs on LM3S811, LM3S1968, LM3S8962, LM4F120, TM4C123
*/

#include <stdint.h>
#include "UART.h"
#include "tm4c123gh6pm.h"

#define UART_FR_TXFF      0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE      0x00000010  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8  0x00000060  // 8 bit word length
#define UART_LCRH_FEN     0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN   0x00000001  // UART Enable


/*
  @UART_Init()
  - Baud rate 115,200.
  - UART0_IBRD_R, UART0_FBRD_R registers specify the baud rate.
    Let Bus clock frequency 50 MHz(Baud16 clock is created from the system bus clock).
    Baud16 = Baud16 / 16 =  (Bus clock frequency) / (16 * divider);
  - bit word length, no parity bits, one stop bit, FIFOs enabled
*/
void UART_Init(void) {
  SYSCTL_RCGCUART_R |= 0x01;            // activate UART0
  SYSCTL_RCGCGPIO_R |= 0x01;            // activate port A

  while((SYSCTL_PRGPIO_R & 0x01) == 0) {}; //ready
  UART0_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART0_IBRD_R = 27;                    // IBRD = int(50,000,000 / (16 * 115,200)) = int(27.1267)
  UART0_FBRD_R = 8;                     // FBRD = int(0.1267 * 64 + 0.5) = 8
  
  // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART0_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN);
  UART0_CTL_R |= UART_CTL_UARTEN;       // enable UART
  GPIO_PORTA_AFSEL_R |= 0x03;           // enable alt funct on PA1-0
  GPIO_PORTA_DEN_R |= 0x03;             // enable digital I/O on PA1-0
                                        // configure PA1-0 as UART
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFFFFFF00)+0x00000011;
  GPIO_PORTA_AMSEL_R &= ~0x03;          // disable analog functionality on PA
}

/* 
  @UART_InChar().
  - Wait for new serial port input.
  - UART0_FR_R gives status of the two FIFOs.
  - Input: none
  - Output: ASCII code for key typed.
*/
char UART_InChar(void) {
  while ((UART0_FR_R & UART_FR_RXFE) != 0);
  return ((char)(UART0_DR_R & 0xFF));
}

/* 
  @UART_OutChar().
  - Output 8-bit to serial port.
  - Input: letter is an 8-bit ASCII character to be transferred
  - Output: none
*/
void UART_OutChar(char data) {
  while ((UART0_FR_R & UART_FR_TXFF) != 0);
  UART0_DR_R = data;
}


/*
  @UART_OutString().
  - It will output a string to the serial port.
  - Output String (NULL termination)
  - Input: pointer to a NULL-terminated string to be transferred
*/
void UART_OutString(char *pt) {
  while (*pt) {
    UART_OutChar(*pt);
    pt++;
  }
}

/*
  @UART_InUDec().
  - It accepts ASCII input in unsigned decimal format and converts to a 32-bit unsigned number.
    valid range is 0 to 4294967295 (2^32-1).
  - If you enter a number above 4294967295, it will return an incorrect value.
  - Backspace will remove last digit typed.
  - Output: 32-bit unsigned number.
*/
uint32_t UART_InUDec(void) {
  char character;
  uint32_t number = 0;
  uint32_t length = 0;
  character = UART_InChar();
  
  // accepts until <enter> is typed.
  while (character != CR) { 
    // The next line checks that the input is a digit, 0-9, If the character is not 0-9,
    // it is ignored and not echoed
    if ((character >= '0') && (character <= '9')) {
      number = 10 * number + (character - '0');   // this line overflows if above 4294967295
      length++;
      UART_OutChar(character);
    } else if ((character == BS) && length) {  // If the input is a backspace, 
      number /= 10;                            // then the return number is, changed and a
      length--;                                // backspace is outputted to th screen.
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

/*
  @UART_OutUDec)().
  - It uses recursion to convert decimal number of unspecified length as an ASCII string.
  - Output a 32-bit number in unsigned decimal format.
  - Input: 32-bit number to be transferred
  - Output: none
  - Variable format 1-10 digits with no space before or after
*/
void UART_OutUDec(uint32_t n){
  if( n >= 10) {
    UART_OutUDec(n / 10);
    n = n % 10;
  }
  UART_OutChar(n + '0'); /* n is between 0 and 9 */
}

/* @UART_InUHex().
  - Accepts ASCII input in unsigned hexadecimal (base 16) format
  - Output: 32-bit unsigned number
  - No '$' or '0x' need be entered, just the 1 to 8 hex digits
  - It will convert lower case a-f to uppercase A-F and converts to a 16 bit unsigned number value
    range is 0 to FFFFFFFF.
  - If you enter a number above FFFFFFFF, it will return an incorrect value
  - Backspace will remove last digit typed
*/ 
uint32_t UART_InUHex(void) {
  char character;
  uint32_t digit;
  uint32_t number = 0;
  uint32_t length = 0;
  
  character = UART_InChar();
  while (character != CR) {
    digit = 0x10; // assume bad
    if ((character >= '0') && (character <= '9')) {
      digit = character-'0';
    } else if((character>='A') && (character<='F')) {
      digit = (character-'A')+0xA;
    } else if((character>='a') && (character<='f')) {
      digit = (character-'a')+0xA;
    }
   // If the character is not 0-9 or A-F, it is ignored and not echoed
    if (digit <= 0xF) {
      number = number * 0x10 + digit;
      length++;
      UART_OutChar(character);
    } else if((character == BS) && length){  // Backspace outputted and return value changed
                                             // if a backspace is inputted
      number /= 0x10;
      length--;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  return number;
}

/*
  @UART_OutUHex().
  - uses recursion to convert the number of unspecified length as an ASCII string.
  - Output a 32-bit number in unsigned hexadecimal format.
  - Input: 32-bit number to be transferred.
  - Variable format 1 to 8 digits with no space before or after.
*/
void UART_OutUHex(uint32_t number){
  if( number >= 0x10) {
    UART_OutUHex(number / 0x10);
    UART_OutUHex(number % 0x10);
  } else {
    if(number < 0xA) {
      UART_OutChar(number + '0');
    } else {
      UART_OutChar((number - 0x0A) + 'A');
    }
  }
}

/*
  @UART_InString().
  - Accepts ASCII characters from the serial port and adds them to a string until <enter> is typed
    or until max length of the string is reached, It echoes each character as it is inputted. If a
    backspace is inputted, the string is modified and the backspace is echoed terminates the string
    with a null character, uses busy-waiting synchronization on RDRF.
  - Input: pointer to empty buffer, size of buffer
  - Output: Null terminated string
*/
void UART_InString(char *bufPt, uint16_t max) {
  int length=0;
  char character;
  character = UART_InChar();
  while (character != CR) {
    if (character == BS) {
      if (length) {
        bufPt--;
        length--;
        UART_OutChar(BS);
      }
    } else if (length < max) {
      *bufPt = character;
      bufPt++;
      length++;
      UART_OutChar(character);
    }
    character = UART_InChar();
  }
  *bufPt = 0;
}
