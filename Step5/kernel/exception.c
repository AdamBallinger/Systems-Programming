//	System exception handlers. These are registered during system
//	initialization and called automatically when they are encountered

#include "exception.h"
#include <hal.h>
#include <console.h>

// For now, all of these interrupt handlers just disable hardware interrupts
// and calls kernal_panic(). This displays an error and halts the system

void KernelPanic (const char* message);

// Divide by zero
void DivideByZeroFault() 
{
	KernelPanic("Divide by 0");
}

// Single Step
void SingleStepTrap() 
{
	KernelPanic("Single step");
}

// Non-Maskable Interrupt
void NMITrap() 
{
	KernelPanic("NMI trap");
}

// Breakpoint hit
void BreakpointTrap() 
{
	KernelPanic("Breakpoint trap");
}

// Overflow
void OverflowTrap() 
{
	KernelPanic("Overflow trap");
}

// Bounds check
void BoundsCheckFault() 
{
	KernelPanic("Bounds check fault");
}

// Invalid opcode / instruction
void InvalidOpcodeFault() 
{
	KernelPanic("Invalid opcode");
}

// Device not available
void NoDeviceFault() 
{
	KernelPanic("Device not found");
}

// Double fault
void DoubleFaultAbort(unsigned int err) 
{
	KernelPanic("Double fault");
}

// Invalid Task State Segment (TSS)
void InvalidTSSFault(unsigned int err) 
{
	KernelPanic("Invalid TSS");
}

// Segment not present
void NoSegmentFault(unsigned int err) 
{
	KernelPanic("Invalid segment");
}

// Stack fault
void StackFault(unsigned int err) 
{
	KernelPanic("Stack fault");
}

// General protection fault
void GeneralProtectionFault(unsigned int err) 
{
	KernelPanic("General Protection Fault");
}

// Page fault
void PageFault(unsigned int err) 
{
	KernelPanic("Page Fault");
}

// Floating Point Unit (FPU) error
void FPUFault() 
{
	KernelPanic("FPU Fault");
}

// Alignment check
void AlignmentCheckFault(unsigned int err) 
{
	KernelPanic("Alignment Check");
}

// machine check
void MachineCheckAbort() 
{
	KernelPanic("Machine Check");
}

// Floating Point Unit (FPU) Single Instruction Multiple Data (SIMD) error
void SimdFPUFault() 
{
	KernelPanic("FPU SIMD fault");
}

// Something has gone very wrong. We cannot continue.
void KernelPanic(const char* message) 
{
	HAL_DisableInterrupts();

	ConsoleClearScreen(0x1f);
	ConsoleWriteString((char *)message);
	for (;;);
}

