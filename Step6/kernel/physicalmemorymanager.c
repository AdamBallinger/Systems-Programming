#include "physicalmemorymanager.h"

// Store address of the bitmap.
uint32_t* pmm_mem_map = 0;
uint32_t pmm_mem_map_size = 0;

// Store the maximum blocks available.
uint32_t pmm_max_blocks = 0;
// Store the used block count.
uint32_t pmm_used_blocks = 0;

//bootInfo: Passed from kernel containing information from boot loader.
//bitmap: The memory address where the bitmap should be located.
// Returns the size of the bitmap.
uint32_t PMM_Initialise(BootInfo* bootInfo, uint32_t bitmap)
{
	pmm_mem_map = bitmap;
	
	for(size_t i = 0; i < bootInfo->MemoryRegions; i++)
	{
		// Check if start of region other than the first starts at 0 (Meaning no more memory)
		if(i > 0 && bootInfo->MemoryRegions[i].StartOfRegionLow <= 0) break;
		
		// Add to total block count the amount of blocks for this region of memory.
		pmm_max_blocks += bootInfo->MemoryRegions[i].SizeOfRegionLow / PMM_GetBlockSize();
	}
	
	pmm_used_blocks = pmm_max_blocks;
	pmm_mem_map_size = pmm_max_blocks;

	memset(pmm_mem_map, 1, pmm_mem_map_size);
	
	for(size_t i = 0; i < bootInfo->MemoryRegions; i++)
	{
		if(i > 0 && bootInfo->MemoryRegions[i].StartOfRegionLow <= 0) break;
		
		// For every region that is marked as available, free each block.
		if(bootInfo->MemoryRegions[i].Type == MEMORY_REGION_AVAILABLE)
		{
			PMM_FreeBlocks(bootInfo->MemoryRegions[i].StartOfRegionLow, 
			bootInfo->MemoryRegions[i].SizeOfRegionLow / PMM_GetBlockSize());
			
			// Decrement used blocks manually as FreeBlocks only decrements if the bit is actually set in the bitmap.
			pmm_used_blocks -= bootInfo->MemoryRegions[i].SizeOfRegionLow / PMM_GetBlockSize();
		}
	}
	
	return pmm_mem_map_size;
}

void PMM_SetBit(uint32_t bit)
{
	pmm_mem_map[bit / BITS] |= (1 << (bit % BITS));
}

int PMM_ClearBit(uint32_t bit)
{
	// Test if the bit is actually set.
	if(PMM_TestBit(bit))
	{
		pmm_mem_map[bit / BITS] &= ~(1 << (bit % BITS));
		return 1;
	}
	
	// Bit wasn't already set
	return 0;
}

bool PMM_TestBit(uint32_t bit)
{
	return pmm_mem_map[bit / BITS] & (1 << (bit % BITS));
}

// Get the first available block in the bitmap.
// Returns the position of the block in the bitmap.
uint32_t PMM_GetFirstFreeBlock()
{
	for(uint32_t i = 0; i < PMM_GetAvailableBlockCount() / BITS; i++)
	{
		if(pmm_mem_map[i] != 0xffffffff)
		{
			for(int j = 0; j < BITS; j++)
			{
				int bit = 1 << j;
				if(!(pmm_mem_map[i] & bit))
				{
					return i * 4 * 8 + j;
				}
			}
		}
	}
	
	return -1;
}

// Gets the first available block with enough following space equal to given size.
// Returns the position of the start block in the bitmap.
uint32_t PMM_GetFirstFreeBlocks(size_t size)
{
	// Can't allocate 0 blocks..
	if(size == 0)
		return -1;
		
	// If we try to allocate just 1 block use the other function.
	if(size == 1)
		return PMM_GetFirstFreeBlock();
		
	for(uint32_t i = 0; i < pmm_mem_map_size; i++)
	{
		if(pmm_mem_map[i] != 0xffffffff)
		{
			for(int j = 0; j < BITS; j++)
			{
				int bit = 1 << j;
				if(!(pmm_mem_map[i] & bit))
				{
					int startBit = i * BITS;
					startBit += bit;
					
					uint32_t free = 0;
					for(uint32_t count = 0; count <= size; count++)
					{
						// Test if the bit is set.
						if(! PMM_TestBit(startBit + count))
						{
							free++;
						}
						
						// Check if we have found enough empty bits to allocate too.
						if(free == size)
						{
							return i * 4 * 8 + j;
						}
					}
				}
			}
		}
	}
	
	return -1;
}

//base: Base address of the region of memory to mark as available.
//size: Size of the region of memory (in bytes).
void PMM_MarkRegionAsAvailable(uint32_t base, size_t size)
{
	uint32_t align = base / PMM_GetBlockSize();
	uint32_t blocks = size / PMM_GetBlockSize();
	
	// If the size is less than the size of a block then add 1 to make sure a block is used.
	if(blocks == 0) blocks++;
	
	for(; blocks > 0; blocks--)
	{
		PMM_ClearBit(align++);
		pmm_used_blocks--;
	}
}

//base: Base address of the region of memory to mark as unavailable.
//size: Size of the region of memory (in bytes).
void PMM_MarkRegionAsUnavailable(uint32_t base, size_t size)
{
	uint32_t align = base / PMM_GetBlockSize();
	uint32_t blocks = size / PMM_GetBlockSize();
	
	// If the size is less than the size of a block then add 1 to make sure a block is used.
	if(blocks == 0) blocks++;
	
	for(; blocks > 0; blocks--)
	{
		PMM_SetBit(align++);
		pmm_used_blocks++;
	}
}

// Returns a pointer to the block of memory that has been allocated.
void* PMM_AllocateBlock()
{
	// Check there are available (usable) blocks to allocate too.
	if(PMM_GetFreeBlockCount() <= 0)
	{
		// No blocks available for allocation.
		return 0;
	}
	
	// Get the first available free blocks bit in the bitmap. (First fit).
	uint32_t freeBlock = PMM_GetFirstFreeBlock();
	
	if(freeBlock == -1)
	{
		// No free blocks available (Out of memory).
		return 0;
	}
	
	// Mark the bit as used.
	PMM_SetBit(freeBlock);
	pmm_used_blocks++;
	
	// Convert from bitmap bit to physical address for the block.
	uint32_t address = freeBlock * PMM_GetBlockSize();
	
	return (void*)address;
}

//size: The number of blocks of memory to be allocated.
// Returns a pointer to the block of memory that has been allocated.
void* PMM_AllocateBlocks(size_t size)
{
	// Check there are enough free blocks before allocating.
	if(PMM_GetFreeBlockCount() < size)
	{
		// Not enough free blocks available to allocate.
		return 0;
	}
	
	// Get the first block bit in the bitmap to start allocation from.
	uint32_t startBlock = PMM_GetFirstFreeBlocks(size);
	
	if(startBlock == -1)
	{
		// Not enough free blocks available to allocate specified size.
		return 0;
	}
	
	// Set each bit to mark block as used.
	for(uint32_t i = 0; i < size; i++)
	{
		PMM_SetBit(startBlock + i);
	}
	
	// Get the address of the block we started allocating from.
	uint32_t startAddress = startBlock * PMM_GetBlockSize();
	
	// Add to the number of used blocks.
	pmm_used_blocks += size;
	
	// Return the address of the first block allocated.
	return (void*) startAddress;
}

//p: The memory address of a block of memory previously allocated.
void PMM_FreeBlock(void* p)
{
	if(p)
	{
		// Cast pointer to address of block
		uint32_t address = (uint32_t)p;
		
		// Get the bit in the bitmap for this block.
		uint32_t bit = address / PMM_GetBlockSize();
		
		// Check if the bit was already set.
		if(PMM_ClearBit(bit) == 1)
		{
			// Decrement used blocks only if the bit was already set.
			pmm_used_blocks--;
		}
	}	
}

// P: Address of blocks.
// Size: Number of blocks to free.
void PMM_FreeBlocks(void* p, size_t size)
{
	if(p)
	{
		// Cast pointer to physical address.
		uint32_t address = (uint32_t)p;
		
		// Get position in bitmap of block to free.
		uint32_t bit = address / PMM_GetBlockSize();
		
		for(uint32_t i = 0; i < size; i++)
		{
			// Only decrement used blocks count if the bit was actually originally set.
			if(PMM_ClearBit(bit) == 1)
			{
				pmm_used_blocks--;
			}
		}
	}
}

// Returns the total amount of memory available (In K).
size_t PMM_GetAvailableMemorySize()
{
	// Because block size is in bytes, divide by 1024 to turn value into KB.
	return PMM_GetAvailableBlockCount() * PMM_GetBlockSize() / 1024;
}

// Returns the total number of blocks available (Both used and unused).
uint32_t PMM_GetAvailableBlockCount()
{
	return pmm_max_blocks;
}

// Returns the number of used blocks.
uint32_t PMM_GetUsedBlockCount()
{
	return pmm_used_blocks;
}

// Returns the number of available unused blocks.
uint32_t PMM_GetFreeBlockCount()
{
	return pmm_max_blocks - pmm_used_blocks;
}

// Returns the size in bytes of each block.
uint32_t PMM_GetBlockSize()
{
	// Each block is 4096 bytes in size.
	return 4096;
}

// Returns the address of the bitmap.
uint32_t PMM_GetMemoryMap()
{
	return *pmm_mem_map;
}