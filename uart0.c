#include "tm4c123gh6pm.h"
#include <uart0.h>
#include <portf.h>
#include <can.h>

bool Tflag;
uint8_t g_UART0_received;
char stringrecv[length];
uint8_t i;
int x,e;



void UART0_Init(void)
{
    /* Set MCU clock. */
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                   SYSCTL_OSC_MAIN);

    /* Enable UART0 module & its port. */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /* Configure UART0 pins. */
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE,
                    GPIO_PIN_0|GPIO_PIN_1);
    
    /* Enable UART0 interrupt if needed. */
//    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    
    /* Configure UART0. */
    UARTConfigSetExpClk(UART0_BASE, 
                        SysCtlClockGet(),
                        UART0_BAUDRATE,
                        (UART_CONFIG_WLEN_8|
                         UART_CONFIG_PAR_NONE|
                         UART_CONFIG_STOP_ONE));

}

void UART0_SendByte(char byte)
{
    /* Send a byte. */
    UARTCharPut(UART0_BASE, byte);
}

void UART0_ReceiveByte(void)
{
        i=0;
        fifoflag=false;
        /* Return received byte. */
        while (i < length && fifoflag == false) {
                g_UART0_received = UART_InChar();

                stringrecv[i] = g_UART0_received;
                i++;
                UART0_SendByte(g_UART0_received);
        }

        Tflag=true;

}

void UART0_SendString(char *pt) {
    while(*pt){
        UARTCharPut(UART0_BASE, *pt);
        pt++;
    }
}

int UART_InChar(void) {
  while ((UART0_FR_R & 0x0010) != 0 && fifoflag == false) {}
    //wait until RXFE is 0
    return ((unsigned int)(UART0_DR_R&0xFF));

}

void Message_Cryption (void)
{
       for(e = 0; e < length ; e++)
         stringrecv[e] = stringrecv[e] + encryption_key; //the key for encryption is 3 that is added to ASCII value

       SimpleDelay();
       UART0_SendString("\n\rEncrypted Message: ");
       for(e = 0; e < length ; e++)
           UART0_SendByte(stringrecv[e]);

}
