#include <string.h>
#include <hal.h>
#include "exception.h"
#include "physicalmemorymanager.h"
#include "bootinfo.h"

// Uncomment the following line once you have written the physical memory manager functions
// to ensure that the physical memory manager is initialised correctly.

#define PMM

BootInfo *	_bootInfo;

char* memoryTypes[] = 
{
	{"Available"},
	{"Reserved"},
	{"ACPI Reclaim"},
	{"ACPI NVS"}
};

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

#ifdef PMM
void InitialisePhysicalMemory()
{
	// Initialise the physical memory manager
	// We place the memory bit map in the next available block of memory after the kernel

	uint32_t memoryMapAddress = 0x100000 + _bootInfo->KernelSize;
	if (memoryMapAddress % PMM_GetBlockSize() != 0)
	{
		// Make sure that the memory map is going to be aligned on a block boundary
		memoryMapAddress = (memoryMapAddress / PMM_GetBlockSize() + 1) * PMM_GetBlockSize();
	}
	uint32_t sizeOfMemoryMap = PMM_Initialise(_bootInfo, memoryMapAddress);
	
	// We now need to mark various regions as unavailable
	
	// Mark first page as unavailable (so that a null pointer is invalid)
	PMM_MarkRegionAsUnavailable(0x00, PMM_GetBlockSize());

	// Mark memory used by kernel as unavailable
	PMM_MarkRegionAsUnavailable(0x100000, _bootInfo->KernelSize);

	// Mark memory used by memory map as unavailable
	PMM_MarkRegionAsUnavailable(PMM_GetMemoryMap(), sizeOfMemoryMap);

	// Reserve two blocks for the stack and make unavailable (the stack is set at 0x90000 in boot loader)
	uint32_t stackSize = PMM_GetBlockSize() * 2;
	PMM_MarkRegionAsUnavailable(_bootInfo->StackTop - stackSize, stackSize);
}
#endif

void Initialise()
{
	ConsoleClearScreen(0x1F);
	ConsoleWriteString("UODOS 32-bit Kernel. Kernel size is ");
	ConsoleWriteInt(_bootInfo->KernelSize, DECIMAL);
	ConsoleWriteString(" bytes\n");
	HAL_Initialise();
	InitialiseInterrupts();
#ifdef PMM	
	InitialisePhysicalMemory();
#endif
}

void PrintBlockUsage()
{
	ConsoleSetColour(0x1A);
	ConsoleWriteString("\nAvailable Blocks: ");
	ConsoleWriteInt(PMM_GetAvailableBlockCount(), DECIMAL);
	ConsoleWriteString("\nUsed Blocks: ");
	ConsoleWriteInt(PMM_GetUsedBlockCount(), DECIMAL);
	ConsoleWriteString("\nFree Blocks: ");
	ConsoleWriteInt(PMM_GetFreeBlockCount(), DECIMAL);
	ConsoleWriteString("\n");
	ConsoleSetColour(0x1F);
}

void PrintMemoryMap(BootInfo* bootInfo)
{
	ConsoleSetColour(0x1B);
	ConsoleWriteString("\nPhysical Memory Map: Address: 0x");
	ConsoleWriteInt(PMM_GetMemoryMap(), HEX);
	for(int i = 0; i < bootInfo->MemoryRegions; i++)
	{
		MemoryRegion region = bootInfo->MemoryRegions[i];
		// Out of memory check
		if(i > 0 && region.StartOfRegionLow <= 0) break;
		
		ConsoleWriteString("\nRegion: ");
		ConsoleWriteInt(i, 10);
		ConsoleWriteString(" Start: 0x");
		ConsoleWriteInt(region.StartOfRegionLow, HEX);
		ConsoleWriteString(" Length: 0x");
		ConsoleWriteInt(region.SizeOfRegionLow, HEX);
		ConsoleWriteString(" bytes Type: ");
		ConsoleWriteString(memoryTypes[region.Type - 1]);
	}
	
	ConsoleWriteString("\nTotal Available Memory: ");
	ConsoleWriteInt(PMM_GetAvailableMemorySize(), DECIMAL);
	ConsoleWriteString(" KB\n");
}

void Tests()
{
	uint32_t* block1 = PMM_AllocateBlock();
	uint32_t* blocks = PMM_AllocateBlocks(10);
	uint32_t* block2 = PMM_AllocateBlock();	
	PMM_FreeBlock(block1);	
	uint32_t* block3 = PMM_AllocateBlock();

	ConsoleWriteString("\n");	
	PrintBlockUsage();
}

void main(BootInfo * bootInfo) 
{
	_bootInfo = bootInfo;
	Initialise();
	
	PrintMemoryMap(bootInfo);
	PrintBlockUsage();
	Tests();
	
	while (true)
	{
		
	}
}
