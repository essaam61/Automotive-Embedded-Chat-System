#include <StateMachine.h>
#include <portf.h>
#include <uart0.h>
#include <can.h>

char state;
int k,x,e;

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
                  state = presenting;  // Set state variable to the new state
                break;
              }
              case(presenting): {
                  GPIOPinWrite(GPIO_PORTF_BASE, 0x0E,GPIO_PIN_1);
                  UART0_SendString("\n\rMESSAGE RECEIVED!\n\r");

                  //
                  //Data Decryption
                  for(e = 0; e < length ; e++)
                      canstringrecv[e] = canstringrecv[e] - encryption_key;  //the key for encryption is 3 that is subtracted to ASCII value
                  UART0_SendString("\n\rMessage is decrypted.\n\r");



                  SimpleDelay();
                  UART0_SendString("\n\rThe message received: ");
                  for (k=0 ; k<=Idx ;k++)
                      UART0_SendByte(canstringrecv[k]);

                  UART0_SendString("\n\rSee you next time\n\r");
                  state = idle; // Set state variable to the new state
                break;
              }
            default: {}
            }

}

void SimpleDelay(void)
{
    //
    // Delay cycles for 1 second
    //
    SysCtlDelay(16000000 / 2);
}
