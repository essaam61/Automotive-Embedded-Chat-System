#include <portf.h>
#include <uart0.h>
#include <statemachine.h>
bool fifoflag;
void GPIOF_Handler (void)
{
    int PB2;
    PB2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);     //Read Switch 2
    // Check when SW2(PF0) is clicked
    if ((PB2 & GPIO_PIN_0) == 0)
    {
        SimpleDelay();
        UART0_SendString("\n\rbutton 2 is pressed\n\r");
        fifoflag=true;
    }

    // Clear the asserted interrupts.
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);      // Clear pending interrupts for PF4

}


void PortF_Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;   //UNLOCKING PORTF PINS SO I CAN USE PUSH BUTTONS
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTF_BASE ,GPIO_PIN_0|GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); // PULL UP RESISTANCE ON BOTH PUSH BUTTONS

    GPIOIntDisable(GPIO_PORTF_BASE,GPIO_PIN_0);        // Disable interrupt for PF4 (in case it was enabled)
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0);      // Clear pending interrupts for PF4
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    IntEnable(INT_GPIOF);
}

