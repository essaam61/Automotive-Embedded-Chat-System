#include "can.h"
#include <uart0.h>
#include <statemachine.h>


//*****************************************************************************
//
// A flag to indicate that some transmission error occurred.
//
//*****************************************************************************
volatile bool g_bErrFlag=0;

//*****************************************************************************
//
// A flag for the interrupt handler to indicate that a message was received.
//
//*****************************************************************************
volatile bool g_bRXFlag=0;
volatile bool g_IRXFlag=0;

void CANIntHandler(void)
{
    uint32_t ui32Status;

    //
    // Read the CAN interrupt status to find the cause of the interrupt
    //
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    //
    // If the cause is a controller status interrupt, then get the status
    //
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.  If the
        // CAN peripheral is not connected to a CAN bus with other CAN devices
        // present, then errors will occur and will be indicated in the
        // controller status.
        //
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        //
        // Set a flag to indicate some errors may have occurred.
        //
        g_bErrFlag = 1;
    }

    //
    // Check if the cause is message object 1, which what we are using for
    // sending messages.
    //
    else if(ui32Status == IRX_Object)
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object 1, and the message TX is complete.  Clear the
        // message object interrupt.
        //
        CANIntClear(CAN0_BASE, IRX_Object);


        //
        // Set flag to indicate received message is pending.
        //
        g_IRXFlag = 1;
        //
        // Since the message was sent, clear any error flags.
        //
        g_bErrFlag = 0;
    }
    else if(ui32Status == MSGRX_Object)
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object 1, and the message TX is complete.  Clear the
        // message object interrupt.
        //
        CANIntClear(CAN0_BASE, MSGRX_Object);


        //
        // Set flag to indicate received message is pending.
        //
        g_bRXFlag = 1;
        //
        // Since the message was sent, clear any error flags.
        //
        g_bErrFlag = 0;
    }

    else if(ui32Status == MSGTX_Object )
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object TXOBJECT, and the message reception is complete.
        // Clear the message object interrupt.
        //
        CANIntClear(CAN0_BASE, MSGTX_Object);

        //
        // Since a message was transmitted, clear any error flags.
        // This is done because before the message is transmitted it triggers
        // a Status Interrupt for TX complete. by clearing the flag here we
        // prevent unnecessary error handling from happeneing
        //
        g_bErrFlag  = 0;
    }
    else if(ui32Status == ITX_Object )
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object TXOBJECT, and the message reception is complete.
        // Clear the message object interrupt.
        //
        CANIntClear(CAN0_BASE, ITX_Object);

        //
        // Since a message was transmitted, clear any error flags.
        // This is done because before the message is transmitted it triggers
        // a Status Interrupt for TX complete. by clearing the flag here we
        // prevent unnecessary error handling from happeneing
        //
        g_bErrFlag  = 0;
    }


    //
    // Otherwise, something unexpected caused the interrupt.  This should
    // never happen.
    //
    else
    {
        //
        // Spurious interrupt handling can go here.
        //
    }
}

////////////////////////////////////////////////////////////////////////////////////

tCANMsgObject sCANMessage;
uint8_t pui8MsgData[8];

tCANMsgObject IndexMessage;
uint8_t IndexData[8];


tCANMsgObject sCANMessageRX;
uint8_t pui8MsgDataRX[8];


void CAN_Init(void)
{

#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    uint32_t ui32SysClock;
#endif


    //
    // Set the clocking to run directly from the external crystal/oscillator.
    // TODO: The SYSCTL_XTAL_ value must be changed to match the value of the
    // crystal on your board.
    //
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_OSC)
                                       25000000);
#else
//    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
//                   SYSCTL_XTAL_16MHZ);
#endif

    // CAN0 is used with RX and TX pins on port B4 and B5.
    // GPIO port B needs to be enabled so these pins can be used.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the GPIO pin muxing to select CAN0 functions for these pins.
    // This step selects which alternate function is available for these pins.
    // This is necessary if your part supports GPIO pin function muxing.
    GPIOPinConfigure(GPIO_PB4_CAN0RX);
    GPIOPinConfigure(GPIO_PB5_CAN0TX);

    // Enable the alternate function on the GPIO pins.  The above step selects
    // which alternate function is available.  This step actually enables the
    // alternate function instead of GPIO for these pins.
    // TODO: change this to match the port/pin you are using
    //
    GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // The GPIO port and pins have been set up for CAN.  The CAN peripheral
    // must be enabled.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    //
    // Initialize the CAN controller
    //
    CANInit(CAN0_BASE);

    //
    // Set up the bit rate for the CAN bus.  This function sets up the CAN
    // bus timing for a nominal configuration.  You can achieve more control
    // over the CAN bus timing by using the function CANBitTimingSet() instead
    // of this one, if needed.
    // In this example, the CAN bus is set to 500 kHz.  In the function below,
    // the call to SysCtlClockGet() or ui32SysClock is used to determine the
    // clock rate that is used for clocking the CAN peripheral.  This can be
    // replaced with a  fixed value if you know the value of the system clock,
    // saving the extra function call.  For some parts, the CAN peripheral is
    // clocked by a fixed 8 MHz regardless of the system clock in which case
    // the call to SysCtlClockGet() or ui32SysClock should be replaced with
    // 8000000.  Consult the data sheet for more information about CAN
    // peripheral clocking.
    //
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    CANBitRateSet(CAN0_BASE, ui32SysClock, 500000);
#else
    CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);
#endif

    //
    // Enable interrupts on the CAN peripheral.  This example uses static
    // allocation of interrupt handlers which means the name of the handler
    // is in the vector table of startup code.  If you want to use dynamic
    // allocation of the vector table, then you must also call CANIntRegister()
    // here.
    //
    // CANIntRegister(CAN0_BASE, CANIntHandler); // if using dynamic vectors
    //
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    //
    // Enable the CAN interrupt on the processor (NVIC).
    //
    IntEnable(INT_CAN0);

    //
    // Enable the CAN for operation.
    //
    CANEnable(CAN0_BASE);

    //
    // Initialize the message object that will be used for sending CAN
    // messages.  The message will be 4 bytes that will contain an incrementing
    // value.  Initially it will be set to 0.
    //

    //
    // Initialize message object 1 to be able to send CAN message 1.  This
    // message object is not shared so it only needs to be initialized one
    // time, and can be used for repeatedly sending the same message ID.
    //
    //MSG OBJECT 1
    sCANMessage.ui32MsgID = 0x1001;
    sCANMessage.ui32MsgIDMask = 0;
    sCANMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
    sCANMessage.ui32MsgLen = sizeof(pui8MsgData);
    sCANMessage.pui8MsgData = pui8MsgData;
    //
    // Initialize message object 2 to be able to send CAN message 2.  This
    // message object is not shared so it only needs to be initialized one
    // time, and can be used for repeatedly sending the same message ID.
    //
    //MSG OBJECT 2
    IndexMessage.ui32MsgID = 0x2001;
    IndexMessage.ui32MsgIDMask = 0;
    IndexMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
    IndexMessage.ui32MsgLen = sizeof(IndexData);
    IndexMessage.pui8MsgData = IndexData;


    //
    // Initialize a message object to be used for receiving CAN messages with
    // any CAN ID.  In order to receive any CAN ID, the ID and mask must both
    // be set to 0, and the ID filter enabled.
    //
    //
    // Initialize a message object to receive CAN messages with ID 0x1001.
    // The expected ID must be set along with the mask to indicate that all
    // bits in the ID must match.
    //
    sCANMessageRX.ui32MsgID = 0x1001;
    sCANMessageRX.ui32MsgIDMask = 0xfffff;
    sCANMessageRX.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    sCANMessageRX.ui32MsgLen = 8;

    //
    // Now load the message object into the CAN peripheral message object 1.
    // Once loaded the CAN will receive any messages with this CAN ID into
    // this message object, and an interrupt will occur.
    //
    CANMessageSet(CAN0_BASE, MSGRX_Object, &sCANMessageRX, MSG_OBJ_TYPE_RX);

    //
    // Change the ID to 0x2001, and load into message object 2 which will be
    // used for receiving any CAN messages with this ID.  Since only the CAN
    // ID field changes, we don't need to reload all the other fields.
    //
    sCANMessageRX.ui32MsgID = 0x2001;
    CANMessageSet(CAN0_BASE, IRX_Object, &sCANMessageRX, MSG_OBJ_TYPE_RX);

}


int Idx;

void CAN_ReceiveByte (void) {
    if(g_bRXFlag)
        {
            state=reception;

            //
            // Reuse the same message object that was used earlier to configure
            // the CAN for receiving messages.  A buffer for storing the
            // received data must also be provided, so set the buffer pointer
            // within the message object.
            //
            sCANMessageRX.pui8MsgData = pui8MsgDataRX;

            //
            // Read the message from the CAN.  Message object number 1 is used
            // (which is not the same thing as CAN ID).  The interrupt clearing
            // flag is not set because this interrupt was already cleared in
            // the interrupt handler.
            //
            CANMessageGet(CAN0_BASE, MSGRX_Object, &sCANMessageRX, 0);             //Tiva2 receive from Tiva1
            CANReceiveByte_ErrorHandler();


            //
            // Clear the pending message flag so that the interrupt handler can
            // set it again when the next message arrives.
            //
            g_bRXFlag = 0;
        }
        else if(g_IRXFlag)
        {
                state=idle;
                sCANMessageRX.pui8MsgData = pui8MsgDataRX;
                CANMessageGet(CAN0_BASE, IRX_Object, &sCANMessageRX, 0);      //Tiva2 receive from Tiva1

                Idx=pui8MsgDataRX[0];

                //
                // Clear the pending message flag so that the interrupt handler can
                // set it again when the next message arrives.
                //
                g_IRXFlag = 0;
        }

}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

int counter;
bool sendflag=false;

void CANSendByte_ErrorHandler (void)
{
    int j,v;
    if(i>=8 && i <=15)
    {
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
       CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }
        sendflag=true;

    }
    else if(i>=16 && i <=23){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
       CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //
        sendflag=true;
    }
    else if(i>=24 && i <=31){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //
        sendflag=true;
    }
    else if(i>=32 && i <=39){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

        sendflag=true;
    }
    else if(i>=40 && i <=47){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

        sendflag=true;
    }
    else if(i>=48 && i <=55){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

        sendflag=true;
    }
    else if(i>=56 && i <=63){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           pui8MsgData[j]=stringrecv[j];      }
              //
             CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
              //

        sendflag=true;
    }

    else if(i>=64 && i <=71){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           pui8MsgData[j]=stringrecv[j];      }
              //
             CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            pui8MsgData[j]=stringrecv[j];      }
               //
              CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
               //

        sendflag=true;
    }
    else if(i>=79 && i <=86){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           pui8MsgData[j]=stringrecv[j];      }
              //
             CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            pui8MsgData[j]=stringrecv[j];      }
               //
              CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             pui8MsgData[j]=stringrecv[j];      }
                //
               CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                //

        sendflag=true;
    }
    else if(i>=87 && i <=94){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           pui8MsgData[j]=stringrecv[j];      }
              //
             CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            pui8MsgData[j]=stringrecv[j];      }
               //
              CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             pui8MsgData[j]=stringrecv[j];      }
                //
               CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                //

                counter=87;
                for (j=counter ; j<(counter+7)  ; j++) {
                              pui8MsgData[j]=stringrecv[j];      }
                 //
                CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                 //
        sendflag=true;
    }
    else if(i>=95 && i <=102){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     pui8MsgData[j]=stringrecv[j];      }
        //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      pui8MsgData[j]=stringrecv[j];      }
         //
        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       pui8MsgData[j]=stringrecv[j];      }
          //
         CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        pui8MsgData[j]=stringrecv[j];      }
           //
          CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         pui8MsgData[j]=stringrecv[j];      }
            //
           CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          pui8MsgData[j]=stringrecv[j];      }
             //
            CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           pui8MsgData[j]=stringrecv[j];      }
              //
             CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            pui8MsgData[j]=stringrecv[j];      }
               //
              CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             pui8MsgData[j]=stringrecv[j];      }
                //
               CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                //

                counter=87;
                for (j=counter ; j<(counter+7)  ; j++) {
                              pui8MsgData[j]=stringrecv[j];      }
                 //
                CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                 //
                 counter=95;
                 for (j=counter ; j<(counter+7)  ; j++) {
                               pui8MsgData[j]=stringrecv[j];      }
                  //
                 CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);             //from Tiva1 to Tiva2
                  //
        sendflag=true;
    }
    else
    {
    //
    }
}

char canstringrecv[length];

extern void CANReceiveByte_ErrorHandler (void) {
    int j,v;
    if(Idx>=8 && Idx <=15)
    {
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }


    }
    else if(Idx>=16 && Idx <=23){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

    }
    else if(Idx>=24 && Idx <=31){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

    }
    else if(Idx>=32 && Idx <=39){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //


    }
    else if(Idx>=40 && Idx <=47){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //


    }
    else if(Idx>=48 && Idx <=55){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //


    }
    else if(Idx>=56 && Idx <=63){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           canstringrecv[j]=pui8MsgDataRX[j];      }
              //

              //


    }

    else if(Idx>=64 && Idx <=71){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           canstringrecv[j]=pui8MsgDataRX[j];      }
              //

              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            canstringrecv[j]=pui8MsgDataRX[j];      }
               //

               //


    }
    else if(Idx>=79 && Idx <=86){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           canstringrecv[j]=pui8MsgDataRX[j];      }
              //

              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            canstringrecv[j]=pui8MsgDataRX[j];      }
               //

               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             canstringrecv[j]=pui8MsgDataRX[j];      }
                //

                //


    }
    else if(Idx>=87 && Idx <=94){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           canstringrecv[j]=pui8MsgDataRX[j];      }
              //

              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            canstringrecv[j]=pui8MsgDataRX[j];      }
               //

               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             canstringrecv[j]=pui8MsgDataRX[j];      }
                //

                //

                counter=87;
                for (j=counter ; j<(counter+7)  ; j++) {
                              canstringrecv[j]=pui8MsgDataRX[j];      }
                 //

                 //

    }
    else if(Idx>=95 && Idx <=102){
        counter=0;
        for(v=0 ; v < 2 ; v++) {
            for (j=counter ; j<(counter+7)  ; j++) {
                     canstringrecv[j]=pui8MsgDataRX[j];      }
        //

        //
        counter=8;
        }

        counter=16;
        for (j=counter ; j<(counter+7)  ; j++) {
                      canstringrecv[j]=pui8MsgDataRX[j];      }
         //

         //

         counter=24;
         for (j=counter ; j<(counter+7)  ; j++) {
                       canstringrecv[j]=pui8MsgDataRX[j];      }
          //

          //

          counter=32;
          for (j=counter ; j<(counter+7)  ; j++) {
                        canstringrecv[j]=pui8MsgDataRX[j];      }
           //

           //

           counter=40;
           for (j=counter ; j<(counter+7)  ; j++) {
                         canstringrecv[j]=pui8MsgDataRX[j];      }
            //

            //

            counter=48;
            for (j=counter ; j<(counter+7)  ; j++) {
                          canstringrecv[j]=pui8MsgDataRX[j];      }
             //

             //

             counter=56;
             for (j=counter ; j<(counter+7)  ; j++) {
                           canstringrecv[j]=pui8MsgDataRX[j];      }
              //

              //

              counter=64;
              for (j=counter ; j<(counter+7)  ; j++) {
                            canstringrecv[j]=pui8MsgDataRX[j];      }
               //

               //

               counter=79;
               for (j=counter ; j<(counter+7)  ; j++) {
                             canstringrecv[j]=pui8MsgDataRX[j];      }
                //

                //

                counter=87;
                for (j=counter ; j<(counter+7)  ; j++) {
                              canstringrecv[j]=pui8MsgDataRX[j];      }
                 //

                 //
                 counter=95;
                 for (j=counter ; j<(counter+7)  ; j++) {
                               canstringrecv[j]=pui8MsgDataRX[j];      }
                  //

                  //

    }



    else
    {
    //
    }

}
