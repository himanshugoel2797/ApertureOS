#include "ihda.h"
#include "utils/common.h"

static uint32_t ihda_bar;
static uint8_t codec_addresses[16];
static uint8_t codec_count;
static uint32_t *corb_buf;
static uint64_t *rirb_buf; 
static uint32_t corb_entry_count, rirb_entry_count;

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

    		pci_setCommand(i, PCI_BUS_MASTER_CMD);
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

	while(!(IHDA_Read(0x08) & 1));	//Wait until the controller is out of reset
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
	COM_WriteStr("Detected Codecs: %b\r\n", codecs);
}

void
IHDA_SetupCORB(void)
{
	uint32_t corbsize = IHDA_Read(0x4E);	//Read the CORBSIZE register
	
	//Determine the largest supported size
	uint32_t supported_sizes = corbsize >> 4;
	uint32_t entry_count = 256;
	uint32_t entry_size = 4;	//One entry is 4 bytes

	if((supported_sizes >> 2) & 1)entry_count = 256;
	else if((supported_sizes >> 1) & 1)entry_count = 16;
	else entry_count = 2;

	//Stop the CORB DMA engine
	while(IHDA_Read(0x4C) & 2)IHDA_Write(0x4C, 0);

	//Set the size of the CORB buffer
	corbsize |= (entry_count / 16) & 3;	//Take the first 2 bits of the result of dividing by 16 to get the code
	IHDA_Write(0x4E, corbsize);

	//Allocate the needed size in DMA buffers and make sure it's KB aligned
	uint32_t base_addr = bootstrap_malloc(entry_count * entry_size + 128);
	base_addr += 128;
	base_addr -= (base_addr % 128);
	memset((void*)base_addr, 0, entry_size * entry_count);

	corb_buf = (uint32_t*)base_addr;
	corb_entry_count = entry_count;

	//Set the base address of the buffer
	IHDA_Write(0x40, base_addr);
	IHDA_Write(0x44, 0);

	//Reset the write pointer
	IHDA_Write(0x48, 0);

	//Reset the read pointer
	while((IHDA_Read(0x4A) >> 15) & 1 == 0)IHDA_Write(0x4A, IHDA_Read(0x4A) | (1 << 15));	//Wait till the controller has completed
	IHDA_Write(0x4A, 0);					//Reset the bit
	while((IHDA_Read(0x4A) >> 15) & 1);		//Read back and make sure the bit has been set to 0

	while(IHDA_Read(0x4C) & 2 != 2)IHDA_Write(0x4C, IHDA_Read(0x4C) | 2);	//Start the CORB DMA engine
}

void
IHDA_SetupRIRB(void)
{
	uint32_t corbsize = IHDA_Read(0x5E);	//Read the RIRBSIZE register
	
	//Determine the largest supported size
	uint32_t supported_sizes = corbsize >> 4;
	uint32_t entry_count = 0;
	uint32_t entry_size = 4;	//One entry is 4 bytes

	if((supported_sizes >> 2) & 1)entry_count = 256;
	else if((supported_sizes >> 1) & 1)entry_count = 16;
	else entry_count = 2;

	//Stop the RIRB DMA engine
	while(IHDA_Read(0x5C) & 2)IHDA_Write(0x5C, 0);

	//Set the size of the RIRB buffer
	corbsize |= (entry_count / 16) & 3;	//Take the first 2 bits of the result of dividing by 16 to get the code
	IHDA_Write(0x5E, corbsize);

	//Allocate the needed size in DMA buffers and make sure it's KB aligned
	uint32_t base_addr = bootstrap_malloc(entry_count * entry_size + 128);
	base_addr += 128;
	base_addr -= (base_addr % 128);
	memset((void*)base_addr, 0, entry_size * entry_count);

	rirb_buf = (uint64_t*)base_addr;
	rirb_entry_count = entry_count;

	//Set the base address of the buffer
	IHDA_Write(0x50, base_addr);
	IHDA_Write(0x54, 0);

	//Reset the read pointer
	IHDA_Write(0x5A, 0);

	//Reset the write pointer
	while((IHDA_Read(0x58) >> 15) & 1 == 0)IHDA_Write(0x58, IHDA_Read(0x58) | 1 << 15);	//Wait till the controller has completed
	IHDA_Write(0x58, 0);					//Reset the bit
	while((IHDA_Read(0x58) >> 15) & 1);		//Read back and make sure the bit has been set to 0

	while(IHDA_Read(0x5C) & 2 != 2)IHDA_Write(0x5C, IHDA_Read(0x5C) | 2);	//Start the RIRB DMA engine
}

void
IHDA_WriteVerb(uint32_t verb)
{
	while(1)
	{
		COM_WriteStr("Data: %b\r\n", IHDA_Read(0x4A));
		uint32_t corb_write_pos = IHDA_Read(0x48);
		uint32_t corb_read_pos = IHDA_Read(0x4A) & ~(1 << 15);

		if(corb_write_pos == corb_read_pos)	//Make sure that all commands so far have been sent
		{
			uint32_t write_pos = (corb_write_pos + 1) % corb_entry_count;

			corb_buf[write_pos] = verb;
			IHDA_Write(0x48, write_pos);
			while(IHDA_Read(0x4C) & 2 != 2)IHDA_Write(0x4C, IHDA_Read(0x4C) | 2);	//Start the CORB DMA engine
			return;
		}
	}
}

uint64_t
IHDA_ReadResponse(void)
{
	while(1)
	{
		while(IHDA_Read(0x5C) & 2 != 2)IHDA_Write(0x5C, IHDA_Read(0x5C) | 2);	//Start the CORB DMA engine
		uint32_t rirb_write_pos = IHDA_Read(0x58) & ~(1 << 15);
		uint32_t rirb_read_pos = IHDA_Read(0x5A);

		if(rirb_write_pos > rirb_read_pos)
		{
			uint32_t read_pos = (rirb_read_pos + 1) % rirb_entry_count;

			uint64_t resp = rirb_buf[rirb_read_pos];
			IHDA_Write(0x5A, read_pos);
			return resp;
		}
	}
}