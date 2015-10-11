#include "ihda.h"

static uint32_t ihda_bar;
static uint8_t codec_addresses[16];
static uint8_t codec_count;

uint32_t
IHDA_Initialize(void)
{
	for(int i = 0; i <= pci_deviceCount; i++)
	{
		if(i == pci_deviceCount)return -1;
		if(pci_devices[i].classCode == 0x04)
		{
			//Found what is likely an IHDA controller
			uint32_t bar = pci_devices[i].bars[0];
			//Map the BAR if necessary

			if(bar < MEMIO_TOP_BASE)
			{
				ihda_bar = (uint32_t)virtMemMan_FindEmptyAddress(KB(4), MEM_KERNEL);
				
				if(ihda_bar == NULL)
					return -1;

				physMemMan_MarkUsed(bar, KB(4));

				if(virtMemMan_Map(ihda_bar, 
				   bar, 
				   KB(4), 
				   MEM_TYPE_UC, 
				   MEM_WRITE | MEM_READ, 
				   MEM_KERNEL) < 0)
					return -1;
			}else{
				ihda_bar = VIRTUALIZE_HIGHER_MEM_OFFSET(bar);
			}

			COM_WriteStr("IHDA controller found!");
			break;
		}
	}

	IHDA_Reset();
	IHDA_DetectCodecs();
	IHDA_SetupCORB();
	IHDA_SetupRIRB();

	return 0;
}

void 
IHDA_Write(uint32_t offset, 
           uint32_t val)
{
	*(uint32_t*)(ihda_bar + offset) = val;
}

uint32_t 
IHDA_Read(uint32_t offset)
{
	return *(uint32_t*)(ihda_bar + offset);
}

void
IHDA_Reset(void)
{
	IHDA_Write(0x08, 1);	//Write 1 to the CRST bit, to start the IHDA controller

	while((IHDA_Read(0x08) & 1) != 1);	//Wait until the controller is out of reset
	for(int i = 0; i < 10000; i++);		//Wait a little more so the codecs can assert status change signals for codec detection
}

void
IHDA_DetectCodecs(void)
{
	uint16_t codecs = (uint16_t)IHDA_Read(0x0E);
	codec_count = 0;
	for(int i = 0; i < 16; i++)
	{
		if((codecs >> i) & 1)codec_addresses[codec_count++] = i;
	}
}

void
IHDA_SetupCORB(void)
{
	uint32_t corbsize = IHDA_Read(0x4E);	//Read the CORBSIZE register
	
	//Determine the largest supported size
	uint8_t supported_sizes = corbsize >> 4;
	uint8_t entry_count = 0;
	uint8_t entry_size = 4;	//One entry is 4 bytes

	if((supported_sizes >> 2) & 1)entry_count = 256;
	else if((supported_sizes >> 1) & 1)entry_count = 16;
	else entry_count = 2;

	//Stop the CORB DMA engine
	while(IHDA_Read(0x4C) & 2)IHDA_Write(0x4C, 0);

	//Set the size of the CORB buffer
	corbsize |= (entry_count / 16) & 3;	//Take the first 2 bits of the result of dividing by 16 to get the code
	IHDA_Write(0x4E, corbsize);

	//Allocate the needed size in DMA buffers and make sure it's KB aligned
	uint32_t base_addr = bootstrap_malloc(entry_count * entry_size + KB(1));
	base_addr += KB(1);
	base_addr -= (base_addr % KB(1));

	//Set the base address of the buffer
	IHDA_Write(0x40, base_addr);
	IHDA_Write(0x44, 0);

	//Reset the write pointer
	IHDA_Write(0x48, 0);

	//Reset the read pointer
	while((IHDA_Read(0x4A) >> 15) & 1 == 0)IHDA_Write(0x4A, 1 << 15);	//Wait till the controller has completed
	IHDA_Write(0x4A, 0);					//Reset the bit
	while((IHDA_Read(0x4A) >> 15) & 1);		//Read back and make sure the bit has been set to 0

	while(IHDA_Read(0x4C) & 2 != 0)IHDA_Write(0x4C, 2);	//Start the CORB DMA engine
}

void
IHDA_SetupRIRB(void)
{
	uint32_t corbsize = IHDA_Read(0x5E);	//Read the RIRBSIZE register
	
	//Determine the largest supported size
	uint8_t supported_sizes = corbsize >> 4;
	uint8_t entry_count = 0;
	uint8_t entry_size = 4;	//One entry is 4 bytes

	if((supported_sizes >> 2) & 1)entry_count = 256;
	else if((supported_sizes >> 1) & 1)entry_count = 16;
	else entry_count = 2;

	//Stop the RIRB DMA engine
	while(IHDA_Read(0x5C) & 2)IHDA_Write(0x5C, 0);

	//Set the size of the RIRB buffer
	corbsize |= (entry_count / 16) & 3;	//Take the first 2 bits of the result of dividing by 16 to get the code
	IHDA_Write(0x5E, corbsize);

	//Allocate the needed size in DMA buffers and make sure it's KB aligned
	uint32_t base_addr = bootstrap_malloc(entry_count * entry_size + KB(1));
	base_addr += KB(1);
	base_addr -= (base_addr % KB(1));

	//Set the base address of the buffer
	IHDA_Write(0x50, base_addr);
	IHDA_Write(0x54, 0);

	//Reset the write pointer
	IHDA_Write(0x58, 0);

	//Reset the read pointer
	while((IHDA_Read(0x5A) >> 15) & 1 == 0)IHDA_Write(0x5A, 1 << 15);	//Wait till the controller has completed
	IHDA_Write(0x5A, 0);					//Reset the bit
	while((IHDA_Read(0x5A) >> 15) & 1);		//Read back and make sure the bit has been set to 0

	while(IHDA_Read(0x5C) & 2 != 0)IHDA_Write(0x5C, 2);	//Start the RIRB DMA engine
}