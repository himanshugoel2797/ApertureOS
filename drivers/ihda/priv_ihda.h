#ifndef _IHDA_PRIV_H_
#define _IHDA_PRIV_H_

#include "types.h"

typedef struct{
	uint32_t addr_lo;
	uint32_t addr_hi;
	uint16_t wp;
	uint16_t rp;
	uint8_t ctrl;
	uint8_t sts;
	uint8_t size;
}__attribute__((packed)) 
RB_Bufs;

typedef struct{
	uint16_t gcap;
	uint8_t vmin;
	uint8_t vmaj;
	uint16_t outpay;
	uint16_t inpay;
	uint32_t gctl;
	uint16_t wakeen;
	uint16_t statests;
	uint16_t gsts;
	uint32_t unkn3;
	uint16_t unkn4;
	uint16_t outstrmpay;
	uint16_t instrmpay;
	uint32_t unkn0;
	uint32_t intctl;
	uint32_t intsts;
	uint32_t unkn2;
	uint32_t unkn5;
	uint32_t wallclock_ctr;
	uint32_t unkn1;
	uint32_t ssync;
	uint32_t unkn6;
	RB_Bufs corb;
	uint8_t unkn7;
	RB_Bufs rirb;
}__attribute__((packed))
IHDA_MMIO;


#endif