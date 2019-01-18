#include <stdint.h>

#define LED (*(volatile uint32_t*)0x02000000)

#define reg_uart_clkdiv (*(volatile uint32_t*)0x02000004)
#define reg_uart_data (*(volatile uint32_t*)0x02000008)

//#define F_CPU 25000000 // Hz
#define F_CPU 12000000 // Hz
#define BAUD_RATE 115200 // bit/s

int get_available()
{
  return reg_uart_data & 0x100;
}

char getchar()
{
  return reg_uart_data;
}

void putchar(char c)
{
    if (c == '\n')
      putchar('\r');
    reg_uart_data = c;
}

void print(const char *p)
{
    while (*p)
        putchar(*(p++));
}

void delay() {
    for (volatile int i = 0; i < 250000; i++)
        ;
}

int main() {
    char *get = "\n";
    reg_uart_clkdiv = F_CPU/BAUD_RATE; // sets baudrate (115200 @ 25 MHz)
//    reg_uart_clkdiv = 103; // sets baudrate (115200 @ 25 MHz)
    while (1) {
        LED = 0xFF;
//        print("hello world\n");
//        delay();
//        if(get_available())
//        {
//          get[0] = getchar();
//          print(get);
//        }
        LED = 0x00;
//        delay();
    }
}
