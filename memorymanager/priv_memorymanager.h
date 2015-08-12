#ifndef _PRIV_MEMORY_MANAGER_H_
#define _PRIV_MEMORY_MANAGER_H_

#include "types.h"

#define KB4_DIVISOR 5
#define KB4_MAX_VAL 32
#define KB4_MAX_VAL_L 6
#define KB4_MAX_VAL_MASK (0xFFFFFFFF >> (32 - KB4_MAX_VAL_L))

uint32_t *KB4_Blocks_Bitmap, *KB4_Blocks_FreeBitCount;
uint64_t KB4_Blocks_Count, KB4_Blocks_FreeBitCount_EntryNum;

#define I_OFF(i) ((i % KB4_DIVISOR) * KB4_MAX_VAL_L)

#define SET_FREE_BITCOUNT(i, val) (KB4_Blocks_FreeBitCount[(i)/KB4_DIVISOR] = (KB4_Blocks_FreeBitCount[(i)/KB4_DIVISOR] & ~(KB4_MAX_VAL_MASK << I_OFF((i)))) | (((val) & KB4_MAX_VAL_MASK) << I_OFF((i))))
#define GET_FREE_BITCOUNT(i) ((KB4_Blocks_FreeBitCount[(i)/KB4_DIVISOR] >> I_OFF((i))) & KB4_MAX_VAL_MASK)

#define DEC_FREE_BITCOUNT(i) (SET_FREE_BITCOUNT(i, (GET_FREE_BITCOUNT(i) - 1)))
#define INC_FREE_BITCOUNT(i) (SET_FREE_BITCOUNT(i, (GET_FREE_BITCOUNT(i) + 1)))

uint8_t *MB2_Blocks_Bitmap;
uint32_t MB2_Blocks_Count;

uint64_t freePageCount;
uint64_t totalPageCount;
uint64_t lastNonFullPage;
uint64_t lastFourthEmptyPage, lastHalfEmptyPage, lastFourthFullPage, lastEmptyPage;

uint64_t MemMan_CalculateBitmapIndex(uint64_t addr, size_t blockSize);
void MemMan_MarkUsed(uint64_t addr, uint64_t size);
void MemMan_MarkKB4Used(uint64_t addr, uint64_t size);

#endif /* end of include guard: _PRIV_MEMORY_MANAGER_H_ */
