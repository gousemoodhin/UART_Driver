// Standard ASCII symbols.
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20
#define DEL  0x7F

/* Function prototypes */
void UART_Init(void);
char UART_InChar(void);
void UART_OutChar(char data);
void UART_OutString(char *pt);
uint32_t UART_InUDec(void);
void UART_OutUDec(uint32_t n);
uint32_t UART_InUHex(void);
void UART_OutUHex(uint32_t number);
void UART_InString(char *bufPt, uint16_t max);
