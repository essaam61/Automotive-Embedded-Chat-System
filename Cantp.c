#include <Cantp.h>

static void SimpleDelay(void);

static uint8_t TxData_Idx=0;
static uint8_t TxMessage_ByteNO;

void CanTp_Init(void)
{
    TxData_Idx = 0;
    TxMessage_ByteNO = 0;
}

void CanTp_Transmit(uint8_t* data)
{
    char j;

    for(i=0 ; i < NO_OF_SEGMENTED_FRAMES ; i++)
    {
        for (j=TxData_Idx ; j<=(TxData_Idx+7)  ; j++)
        {
            pui8MsgData[TxMessage_ByteNO]=data[j];
            TxMessage_ByteNO++;
        }

        CANMessageSet(CAN0_BASE, MSGTX_Object, &sCANMessage, MSG_OBJ_TYPE_TX);
        SimpleDelay();

        TxData_Idx+=8;
        TxMessage_ByteNO=0;

        if(TxData_Idx == DATA_LENGTH)
        {
            TxData_Idx = 0;
        }
    }

}

void SimpleDelay(void)
{
    //
    // Delay cycles for 1 second
    //
    SysCtlDelay(16000000 / 2);
}
