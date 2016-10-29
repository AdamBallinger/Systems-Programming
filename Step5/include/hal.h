#ifndef _HAL_H
#define _HAL_H
//	Hardware Abstraction Layer Interface
//
//	The Hardware Abstraction Layer (HAL) provides an abstract interface
//	to control the basic motherboard hardware devices. This is accomplished
//	by abstracting hardware dependencies behind this interface.

#include <stdint.h>

#define far
#define near

// Initialize hardware abstraction layer
int	 HAL_Initialise();

// Shutdown hardware abstraction layer
int	 HAL_Shutdown();

// Is HAL initialised?
bool HAL_IsInitialised();

// Generate interrupt
void HAL_GenerateInterrupt(int n);

// Notify hal interrupt is done
void HAL_InterruptDone(unsigned int intno);

// Read byte from device using port mapped io
unsigned char HAL_InputByteFromPort(unsigned short portid);

// Write byte to device through port mapped io
void  HAL_OutputByteToPort(unsigned short portid, unsigned char value);

// EnableInterrupts all hardware interrupts
void  HAL_EnableInterrupts();

// DisableInterrupts all hardware interrupts
void  HAL_DisableInterrupts();

// Set new interrupt vector
void  HAL_SetInterruptVector(int intno, void (*vect)());

// Return current interrupt vector
void (*HAL_GetInterruptVector(int intno))();

// Return CPU vender
char * HAL_GetCPUVendor();

#endif
