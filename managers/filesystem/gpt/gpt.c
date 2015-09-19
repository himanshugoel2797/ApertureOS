#include "gpt.h"
#include "priv_gpt.h"
#include "../mbr/priv_mbr.h"

uint64_t baseLBA = 0;
ReadFunc rd;
WriteFunc wr;

int32_t
GPT_Setup(ReadFunc read, 
          WriteFunc write,
          uint32_t *sector_size)
{
	uint8_t *top = bootstrap_malloc(MB(1) + KB(4));

	//Align address to page boundary
	//uint8_t *top = (tmp_top + KB(4));
	//top -= ((uint32_t)top % KB(4));

	if(read(0, MB(1), (uint16_t*)top) == 0){
		//kfree(tmp_top);
		return -1;
	}

	//Check the MBR first and determine if a GPT is present
	MBR_HEADER *mbr = (MBR_HEADER*)top;

	if(mbr->boot_sig[0] != MBR_SIG_0 && mbr->boot_sig[1] != MBR_SIG_1){
		//kfree(tmp_top);
		return -2;
	}

	if(mbr->partitions[0].partition_type != 0xEE){
		//kfree(tmp_top);
		return -3;
	}

	//This disk is GPT, probe for the GPT table

	uint64_t* magic_search = (uint64_t*)top;
	int i;

	for(i = 0; i < MB(1)/sizeof(uint64_t); i++)
	{
		if(magic_search[i] == GPT_MAGIC_VAL)
		{
			//Found the GPT table!
			if(sector_size != NULL)
				*sector_size = (i * sizeof(uint64_t));	//This is at LBA1, use this to determine how large a sector is
			break;
		}
	}

	if(i == MB(1)/sizeof(uint64_t))return -4;

	GPT_Header *hdr = (GPT_Header*)&magic_search[i];

	GPT_Entry *part_table = NULL;

	uint32_t req_size = hdr->partition_entry_size * hdr->partition_entry_count;
	uint32_t e_size = hdr->partition_entry_size;
	uint32_t e_count = hdr->partition_entry_count;

	if(req_size > MB(1))
		return -5;	//Don't support crazy partition tables!

	if( (hdr->lba_partition_entries * *sector_size) + req_size >= MB(1) )
	{
		if(read(hdr->lba_partition_entries, MB(1), (uint16_t*)top) == 0){
			//kfree(tmp_top);
			return -6;
		}

		part_table = (GPT_Entry*)top;
	}else{
		part_table = (GPT_Entry*)(top + hdr->lba_partition_entries * *sector_size);
	}

	for(int i = 0; i < e_count; i++)
	{
		if(*(uint32_t*)part_table->partition_type_guid == 0x0FC63DAF)
		{
			baseLBA = part_table->start_lba;
			rd = read;
			wr = write;
			int rval = Filesystem_RegisterDescriptor("/", GPT_0_Read, GPT_0_Write, BOOT_FS);
			return rval;
			break;
		}
		part_table = (GPT_Entry*)((uint32_t)part_table + e_size);
	}
	while(1);
	return 0;
}

bool
GPT_0_Write(uint64_t start,
             uint32_t count,
             uint16_t *buf)
{
	return wr(start + baseLBA, count, buf); 
}

bool
GPT_0_Read(uint64_t start,
            uint32_t count,
            uint16_t *buf)
{
    return rd(start + baseLBA, count, buf);
}