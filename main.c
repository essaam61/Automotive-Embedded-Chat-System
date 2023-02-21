#include <portf.h>
#include <uart0.h>
#include <Cantp.h>
#include <statemachine.h>


int main(void)
{
    PortF_Init();
    UART0_Init();
    CAN_Init();

    // Set the leds initially to blue
    GPIOPinWrite(GPIO_PORTF_BASE, 0x0E,GPIO_PIN_2);
    state = idle; // Declare state variable

    int PB1;
    Tflag=false;

    while (1)
    {
        PB1 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_4);         //Read Button 1

        // Check when SW1(PF4) is clicked
        if((PB1 & GPIO_PIN_4) == 0) {
            state = data_collection; // Set state variable to the new state
            State_Machine();

            UARTCharPut(UART0_BASE, '\n');
            UARTCharPut(UART0_BASE, '\r');
            UART0_SendString("\n\rWELCOME\n\r");
            UART0_SendString("Please input a message: ");

            UART0_ReceiveByte();      //from PC to Tiva
        }
        else if(Tflag)
        {
            state = transmission; // Set state variable to the new state
            State_Machine();

            

            IndexData[0] = i;
            CANMessageSet(CAN0_BASE, ITX_Object, &IndexMessage, MSG_OBJ_TYPE_TX);


            UART0_EncryptMessage();  //Enrypt the message before sending on the microcontrollers

            UART0_SendString("\n\n\rData is being transmitted..\n\r");
            CanTp_Transmit(RxBuffer);

            UART0_SendString("\n\rData is SENT\n\r");
            state = idle; // Set state variable to the new state
            Tflag=false;
        }
        else
        {
            CAN_Receive();                     //Tiva2 receive from Tiva1
            State_Machine();
        }


    }


}
