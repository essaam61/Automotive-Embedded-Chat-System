#ifndef UART0_H_
#define UART0_H_

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"


#define UART0_BAUDRATE   (115200)

#define DATA_LENGTH 200
#define encryption_key 3

extern bool Tflag;
extern uint8_t g_UART0_received;
extern uint8_t RxBuffer[DATA_LENGTH];
extern uint8_t i;

extern void UART0_Init(void);
extern void UART0_SendByte(char byte);
extern void UART0_ReceiveByte(void);
extern void UART0_SendString (char *pt);
extern int UART0_InChar(void);
extern void UART0_EncryptMessage (void);


#endif
