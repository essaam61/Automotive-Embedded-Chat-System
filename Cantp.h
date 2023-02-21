#ifndef CANTP_H_
#define CANTP_H_

#include "can.h"

#define NO_OF_SEGMENTED_FRAMES DATA_LENGTH/8

extern void CanTp_Init(void);
extern void CanTp_Transmit(uint8_t* data);

#endif /* CANTP_H_ */
