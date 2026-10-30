#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include <kern/conc/sleeplock.h>
#include <kern/proc/user_environment.h>
#include <kern/mem/memory_manager.h>
#include "../conc/kspinlock.h"
#include "../conc/ksemaphore.h"
#include <kern/cpu/cpu.h>

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [0] GIVEN FUNCTIONS
//==============================================
// [1] INITIALIZE KERNEL HEAP:
//==============================================
void kheap_init()
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		buddy_init();
		set_kheap_strategy(KHP_PLACE_CUSTOMFIT);
		kheapPageAllocStart = KERNEL_HEAP_START;
		kheapPageAllocBreak = KERNEL_HEAP_START;
	}
	//==================================================================================
	//==================================================================================
}

#define NUM_OF_HEAP_PAGES ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE)
#define MAX_FRAMES        (1 << 16)
#define PAGE_FREE      0
#define PAGE_BUDDY     1
#define PAGE_ALLOCATED 2

static uint8  page_type[NUM_OF_HEAP_PAGES];
static unsigned alloc_size[NUM_OF_HEAP_PAGES];
static unsigned int free_areas_start[NUM_OF_HEAP_PAGES / 2 + 2];
static unsigned int free_areas_size[NUM_OF_HEAP_PAGES / 2 + 2];
static unsigned int free_areas_count = 0;
static unsigned frame_va[MAX_FRAMES];
static int map_heap_page(uint32 va)
{
	struct FrameInfo *ptr_frame_info = NULL;

	if (allocate_frame(&ptr_frame_info) != 0)
		return -1;

	if (map_frame(ptr_page_directory, ptr_frame_info, va, PERM_WRITEABLE) != 0)
	{
		free_frame(ptr_frame_info);
		return -1;
	}

	uint32 frame_number = PPN(to_physical_address(ptr_frame_info));
	if (frame_number < MAX_FRAMES)
		frame_va[frame_number] = va;

	return 0;
}


static void remove_free_area(uint32 i)
{
	for (; i < free_areas_count - 1; i++)
	{
		free_areas_start[i] = free_areas_start[i + 1];
		free_areas_size[i] = free_areas_size[i + 1];
	}
	free_areas_count--;
}


static void release_pages(unsigned va, unsigned num_of_pages)
{
	unsigned i, pos;

	for (i = 0; i < num_of_pages; i++)
	{
		unsigned page_va = va + i * PAGE_SIZE;
		unsigned *ptr_page_table = NULL;
		struct FrameInfo *ptr_frame_info = get_frame_info(ptr_page_directory, page_va, &ptr_page_table);

		if (ptr_frame_info != NULL)
		{
			unsigned frame_number = PPN(to_physical_address(ptr_frame_info));
			if (frame_number < MAX_FRAMES)
				frame_va[frame_number] = 0;
			unmap_frame(ptr_page_directory, page_va);

		}
		page_type[(page_va - KERNEL_HEAP_START) / PAGE_SIZE] = PAGE_FREE;
	}
	alloc_size[(va - KERNEL_HEAP_START) / PAGE_SIZE] = 0;

	pos = 0;
	while (pos < free_areas_count && free_areas_start[pos] < va)
		pos++;

	if (pos > 0 && free_areas_start[pos - 1] + free_areas_size[pos - 1] * PAGE_SIZE == va)
	{
		free_areas_size[pos - 1] += num_of_pages;
		pos--;
	}
	else
	{
		for (i = free_areas_count; i > pos; i--)
		{
			free_areas_start[i] = free_areas_start[i - 1];
			free_areas_size[i] = free_areas_size[i - 1];
		}
		free_areas_start[pos] = va;
		free_areas_size[pos] = num_of_pages;
		free_areas_count++;
	}

	if (pos + 1 < free_areas_count && free_areas_start[pos] + free_areas_size[pos] * PAGE_SIZE == free_areas_start[pos + 1])
	{
		free_areas_size[pos] += free_areas_size[pos + 1];
		remove_free_area(pos + 1);
	}

	if (free_areas_start[pos] + free_areas_size[pos] * PAGE_SIZE == kheapPageAllocBreak)
	{
		kheapPageAllocBreak = free_areas_start[pos];
		remove_free_area(pos);
	}
}

// Custom Fit: exact fit, then worst fit, then move the break.
static void *allocate_pages(uint32 num_of_pages, uint8 type)
{
	uint32 i, va;
	int index = -1;

	for (i = 0; i < free_areas_count; i++)
	{
		if (free_areas_size[i] == num_of_pages)
		{
			index = i;
			break;
		}
		if (free_areas_size[i] > num_of_pages && (index == -1 || free_areas_size[i] > free_areas_size[index]))
			index = i;
	}

	if (index != -1)
	{
		va = free_areas_start[index];
		if (free_areas_size[index] == num_of_pages)
			remove_free_area(index);
		else
		{
			free_areas_start[index] += num_of_pages * PAGE_SIZE;
			free_areas_size[index] -= num_of_pages;
		}
	}
	else
	{
		uint32 size = num_of_pages * PAGE_SIZE;
		if (kheapPageAllocBreak + size < kheapPageAllocBreak || kheapPageAllocBreak + size > KERNEL_HEAP_MAX)
			return NULL;
		va = kheapPageAllocBreak;
		kheapPageAllocBreak += size;
	}

	// allocate frames and map them
	for (i = 0; i < num_of_pages; i++)
	{
		if (map_heap_page(va + i * PAGE_SIZE) != 0)
		{
			// no free frames: undo everything and fail
			release_pages(va, num_of_pages);
			return NULL;
		}
		page_type[(va + i * PAGE_SIZE - KERNEL_HEAP_START) / PAGE_SIZE] = type;
	}

	alloc_size[(va - KERNEL_HEAP_START) / PAGE_SIZE] = num_of_pages;
	return (void *)va;
}




/************************************************************************************/

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
/**************************************************/
/* BLOCK ALLOCATOR */
/**************************************************/
void* create_buddy_space()
{
	//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [4] create_buddy_space
	//Your code is here
	//Comment the following line
	panic("create_buddy_space() not implemented");
}

void  delete_buddy_space(void *va)
{
	//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [5] delete_buddy_space
	//Your code is here
	//Comment the following line
	panic("delete_buddy_space() not implemented");
}

/**************************************************/
/* PAGE ALLOCATOR */
/**************************************************/
void* kmalloc(unsigned int size)
{
unsigned num_of_pages;

		if (size == 0)
			return NULL;

		if (size + META_DATA_SIZE <= DYN_ALLOC_MAX_BLOCK_SIZE)
			return alloc_block(size);

		num_of_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
		return allocate_pages(num_of_pages, PAGE_ALLOCATED);
}


void kfree(void* virtual_address)
{
	   unsigned va = (uint32)virtual_address;
	   unsigned index;

		if (virtual_address == NULL)
			return;

		if (va < kheapPageAllocStart || va >= kheapPageAllocBreak)
			panic("kfree: invalid kernel heap address");

		index = (va - KERNEL_HEAP_START) / PAGE_SIZE;

		if (page_type[index] == PAGE_BUDDY)
		{
			free_block(virtual_address);
		}
		else if (page_type[index] == PAGE_ALLOCATED)
		{
			if (va % PAGE_SIZE != 0 || alloc_size[index] == 0)
				panic("kfree: address is not the start of an allocation");

			release_pages(va, alloc_size[index]);
		}
}


unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'26.GM#2] KERNEL HEAP - [8] kheap_physical_address
	//Your code is here
	//Comment the following line
	panic("kheap_physical_address() is not implemented yet...!!");

	/*EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED */
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	unsigned frame_number = PPN(physical_address);
		numOfKheapVACalls++;

		if (frame_number >= MAX_FRAMES || frame_va[frame_number] == 0)
			return 0;

		return frame_va[frame_number] + (physical_address & 0xFFF);
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().
void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'26.BONUS#1] KERNEL REALLOC - krealloc
	//Your code is here
	//Comment the following line
	panic("krealloc() is not implemented yet...!!");
}
