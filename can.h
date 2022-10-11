#ifndef CAN_H_
#define CAN_H_

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "uart0.h"

#define MSGTX_Object 1
#define ITX_Object 2

#define MSGRX_Object 3
#define IRX_Object 4



extern tCANMsgObject sCANMessage;
extern uint8_t pui8MsgData[8];
extern tCANMsgObject IndexMessage;
extern uint8_t IndexData[8];

extern tCANMsgObject sCANMessageRX;
extern uint8_t pui8MsgDataRX[8];

extern int Idx;
extern bool sendflag;
extern char canstringrecv[length];


extern void CAN_Init(void);
extern void CAN_ReceiveByte (void);
extern void CANSendByte_ErrorHandler (void);
extern void CANReceiveByte_ErrorHandler (void);


#endif
