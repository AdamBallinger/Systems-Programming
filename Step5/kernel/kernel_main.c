#include <string.h>
#include <hal.h>
#include <console.h>
#include "exception.h"

// This is a dummy __main.  For some reason, gcc puts in a call to 
// __main from main, so we just include a dummy.
 
void __main() {}

void InitialiseInterrupts()
{
	HAL_EnableInterrupts();

	// Install our exception handlers
	HAL_SetInterruptVector (0, DivideByZeroFault);
	HAL_SetInterruptVector (1, SingleStepTrap);
	HAL_SetInterruptVector (2, NMITrap);
	HAL_SetInterruptVector (3, BreakpointTrap);
	HAL_SetInterruptVector (4, OverflowTrap);
	HAL_SetInterruptVector (5, BoundsCheckFault);
	HAL_SetInterruptVector (6, InvalidOpcodeFault);
	HAL_SetInterruptVector (7, NoDeviceFault);
	HAL_SetInterruptVector (8, DoubleFaultAbort);
	HAL_SetInterruptVector (10, InvalidTSSFault);
	HAL_SetInterruptVector (11, NoSegmentFault);
	HAL_SetInterruptVector (12, StackFault);
	HAL_SetInterruptVector (13, GeneralProtectionFault);
	HAL_SetInterruptVector (14, PageFault);
	HAL_SetInterruptVector (16, FPUFault);
	HAL_SetInterruptVector (17, AlignmentCheckFault);
	HAL_SetInterruptVector (18, MachineCheckAbort);
	HAL_SetInterruptVector (19, SimdFPUFault);
}

void Initialise()
{
	ConsoleClearScreen(0x1F);
	ConsoleWriteString("UODOS 32-bit Kernel Executing.\n");
	HAL_Initialise();
	ConsoleWriteString("HAL Initialised. Running on ");
	ConsoleWriteString(HAL_GetCPUVendor());
	ConsoleWriteString("\n");
	InitialiseInterrupts();
}


void main() 
{
	Initialise();
	//ConsoleWriteInt(1 / 0, 10);
}
