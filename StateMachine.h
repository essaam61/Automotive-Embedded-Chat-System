#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <stdbool.h>
#include <stdint.h>

// Define states
# define idle 0
# define data_collection 1
# define transmission 2
# define reception 3
# define presenting 4

extern char state;

extern void State_Machine (void);
extern void SimpleDelay(void);

#endif
