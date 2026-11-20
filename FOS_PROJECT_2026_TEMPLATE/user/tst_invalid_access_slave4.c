/********************************************************** */
/* MAKE SURE PAGE_WS_MAX_SIZE = 15 */
/************************************************************/

#include <inc/lib.h>

void _main(void)
{
	//[4] Not in Page File, Not Stack & Not Heap
	uint32 kilo = 1024;
	{
		//uint32 size = 4*kilo;

		//Successful access to allowed space
		unsigned char *x = (unsigned char *)(0x00200000);

		int i=0;
		for(;i< 20;i++)
		{
			x[i]=-1;
		}

		inctst();

		//Access to NOT allowed space
		x = (unsigned char *)(0x00200000 - 20);

		i=0;
		for(;i< 20;i++)
		{
			x[i]=-1;
		}

	}

	inctst();
	panic("tst invalid access failed: Attempt to access page that's not exist in page file, neither stack or heap.\nThe env must be killed and shouldn't return here.");

	return;
}

