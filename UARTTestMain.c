/* 
  Used to test the UART driver.
*/
#include <stdint.h>
#include "PLL.h"
#include "UART.h"

/* 
  @OutCRLF()
  - Output a CR,LF to UART to go to a new line
  - Input: none
  - Output: none
*/
void OutCRLF(void) {
  UART_OutChar(CR);
  UART_OutChar(LF);
}

int main(void) {
  char ch;
  uint32_t n;
  char string[20];  // global to assist in debugging.
  
  PLL_Init();   // 50 MHz.
  UART_Init();  // initialize UART.
  OutCRLF();

  for (ch = 'A'; ch <= 'Z'; ch = ch + 1) {  // print the uppercase alphabet.
    UART_OutChar(ch);
  }
  OutCRLF();
  UART_OutChar(' ');

  for (ch = 'a'; ch <= 'z'; ch = ch + 1){  // print the lowercase alphabet.
    UART_OutChar(ch);
  }
  OutCRLF();
  UART_OutChar('-');
  UART_OutChar('-');
  UART_OutChar('>');

  while (1) {
    UART_OutString("InString: ");
    UART_InString(string,19);
    UART_OutString(" OutString="); UART_OutString(string); OutCRLF();

    UART_OutString("InUDec: ");  n=UART_InUDec();
    UART_OutString(" OutUDec="); UART_OutUDec(n); OutCRLF();

    UART_OutString("InUHex: ");  n=UART_InUHex();
    UART_OutString(" OutUHex="); UART_OutUHex(n); OutCRLF();

  }
}
