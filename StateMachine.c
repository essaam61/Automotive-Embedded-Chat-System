#include <StateMachine.h>
#include <portf.h>
#include <uart0.h>
#include <can.h>

char state;
static char k,e;

void State_Machine(void)
{
            switch(state) {
              case(idle): {
                  GPIOPinWrite(GPIO_PORTF_BASE, 0x0E,GPIO_PIN_2);  // Turn on led 2 (BLUE)
                break;
              }
              case(data_collection): {
                  GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, GPIO_PIN_3); // Turn on led 3 (GREEN)
                break;
              }
              case(transmission): {
                  GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, GPIO_PIN_3); // Turn on led 3 (GREEN)
                break;
              }
              case(reception): {
                 GPIOPinWrite(GPIO_PORTF_BASE, 0x0E, GPIO_PIN_1); // Turn on led 1 (RED)
                break;
              }
              case(presenting): {
                  GPIOPinWrite(GPIO_PORTF_BASE, 0x0E,GPIO_PIN_1);
                  UART0_SendString("\n\rMESSAGE RECEIVED!\n\r");

                  
                  /* Data Decryption */
                  for(e = 0; e < DATA_LENGTH ; e++)
                      canstringrecv[e] = canstringrecv[e] - encryption_key;  //the key for encryption is 3 that is subtracted to ASCII value
                  UART0_SendString("\n\rMessage is decrypted.\n\r");


                  UART0_SendString("\n\rThe message received: ");
                  for (k=0 ; k<=Idx ;k++)
                  {
                      UART0_SendByte(canstringrecv[k]);
                  }

                  state = idle; // Set state variable to the new state
                break;
              }
            default: {}
            }

}
