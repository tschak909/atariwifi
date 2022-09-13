/* Basically a simplified copy of the ESP Arduino library in HardwareSerial.h/HardwareSerial.cpp
*/
#ifndef FNUART_H
#define FNUART_H

#include <driver/uart.h>

#include <string>


#define UART_DEBUG UART_NUM_0
#define UART_ADAMNET UART_NUM_2
#ifdef BUILD_RS232
#define UART_SIO UART_NUM_1
#else
#define UART_SIO UART_NUM_2
#endif

class UARTManager
{
private:
    uart_port_t _uart_num;
    QueueHandle_t _uart_q;
    bool _initialized; // is UART ready?

    size_t _print_number(unsigned long n, uint8_t base);

public:
    UARTManager(uart_port_t uart_num=0);

    void begin(int baud);
    void end();
    bool initialized() { return _initialized; }

    uint32_t get_baudrate();
    void set_baudrate(uint32_t baud);

    int available();
    int peek();
    void flush();
    void flush_input();

    int read();
    size_t readBytes(uint8_t *buffer, size_t length);
    size_t readBytes(char *buffer, size_t length) { return readBytes((uint8_t *)buffer, length); };

    size_t write(uint8_t);
    size_t write(const uint8_t *buffer, size_t size);
    size_t write(const char *s);

    size_t write(unsigned long n) { return write((uint8_t)n); };
    size_t write(long n) { return write((uint8_t)n); };
    size_t write(unsigned int n) { return write((uint8_t)n); };
    size_t write(int n) { return write((uint8_t)n); };

    size_t printf(const char *format, ...);

    //size_t println(const char *format, ...);
    size_t println(const char *str);
    size_t println() { return print("\n"); };
    size_t println(std::string str);
    size_t println(int num, int base = 10);

    //size_t print(const char *format, ...);
    size_t print(const char *str);
    size_t print(std::string str);
    size_t print(int n, int base = 10);
    size_t print(unsigned int n, int base = 10);
    size_t print(long n, int base = 10);
    size_t print(unsigned long n, int base = 10);
};

extern UARTManager fnUartDebug;
extern UARTManager fnUartSIO;

#endif //FNUART_H
