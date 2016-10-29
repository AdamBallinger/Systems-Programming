//    Hal.c
// 	  Hardware Abstraction Layer for i86 architecture
//
//	The Hardware Abstraction Layer (HAL) provides an abstract interface
//	to control the basic motherboard hardware devices. This is accomplished
//	by abstracting hardware dependencies behind this interface.
//

#include <Hal.h>
#include "cpu.h"
#include "idt.h"
#include "pic.h"
#include <console.h>

static bool _halInitialised = false;

// Initialize hardware devices
int  HAL_Initialise () 
{
	// Initialize motherboard controllers and system timer
	I86_CPU_Initialise();
	I86_PIC_Initialise(0x20,0x28);

	// Enable hardware interrupts
	HAL_EnableInterrupts();
	_halInitialised = true;
	return 0;
}

// Shutdown hardware devices
int  HAL_Shutdown () 
{
	_halInitialised = false;
	I86_CPU_Shutdown();
	return 0;
}

bool HAL_IsInitialised()
{
	return _halInitialised;
}

// Generate interrupt call

void  HAL_GenerateInterrupt (int n) 
{
	// This generates the appropriate interrupt by modifying the byte
	// after the 'int' instruction
	asm("movb (%0), %%al\n\t " 
		"movb  %%al, (genint + 1)\n\t" 
		"jmp  genint \n"
	"genint:\n\t"
		"int $0"
	: : "r" (n));
}

// Notify hal thatinterrupt is done
inline void  HAL_InterruptDone(unsigned int intno) 
{
	// Ensure it's a valid hardware irq
	if (intno > 16)
	{
		return;
	}
	//! test if we need to send end-of-interrupt to second pic
	if (intno >= 8)
	{
		I86_PIC_SendCommand(I86_PIC_OCW2_MASK_EOI, 1);
	}
	// Always send end-of-interrupt to primary pic
	I86_PIC_SendCommand(I86_PIC_OCW2_MASK_EOI, 0);
}

// Read byte from device using port mapped io
unsigned char HAL_InputByteFromPort (unsigned short portid) 
{
	unsigned char result = 0;
	asm volatile ("movw	%1, %%dx \n\t"
				  "in	%%dx, %%al \n\t"
				  "movb %%al, %0"
				  : "=m" (result)
				  : "m" (portid));
	return result;
}

// Write byte to device through port mapped io
void HAL_OutputByteToPort(unsigned short portid, unsigned char value) 
{
	asm volatile("movb	%1, %%al \n\t"
				 "movw	%0, %%dx \n\t"
				  "out	%%al, %%dx"
				 : : "m" (portid), "m" (value));
				 
}

//! Enable all hardware interrupts
void HAL_EnableInterrupts() 
{
	asm("sti");
}


//! Disable all hardware interrupts
void HAL_DisableInterrupts() 
{
	asm("cli");
}


// Sets new interrupt vector
void HAL_SetInterruptVector(int intno, void (*vect)() ) 
{
	// Install interrupt handler! This overwrites prev interrupt descriptor
	I86_IDT_InstallInterruptHandler(intno, 
									I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32,
									0x8, 
									vect);
}

// Return current interrupt vector
void (*HAL_GetInterruptVector(int intno))() 
{
	// Get the descriptor from the idt
	idt_descriptor* desc = I86_IDT_GetInterruptHandler (intno);
	if (!desc)
	{
		return 0;
	}
	// Get address of interrupt handler
	uint32_t addr = desc->baseLo | (desc->baseHi << 16);

	// Return interrupt handler
	I86_IRQ_HANDLER irq = (I86_IRQ_HANDLER)addr;
	return irq;
}

// Returns cpu vender
char* HAL_GetCPUVendor() 
{
	return I86_CPU_GetVendor();
}


