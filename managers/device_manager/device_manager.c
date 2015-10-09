#include "device_manager.h"
#include "utils/common.h"
#include "globals.h"

#define DMA_BMP_SIZE (DMA_POOL_SIZE/(PAGE_SIZE * 64) + 1)

static AOS_Devices devices[MAX_ACTIVE_DEVICES];
static uint8_t free_device_index = 0;

static uint8_t dma_pool[DMA_POOL_SIZE];
static uint64_t dma_pool_bitmap[DMA_BMP_SIZE];

static uint64_t msi_vectors[4];

void
DeviceManager_Initialize(void)
{
    memset(devices, 0, sizeof(AOS_Devices) * MAX_ACTIVE_DEVICES);
    memset(dma_pool_bitmap, 0, sizeof(uint64_t) * DMA_BMP_SIZE);
    free_device_index = 0;
    memset(msi_vectors, 0, sizeof(uint64_t) * 4);

    //Mark initial vectors as being taken
    msi_vectors[0] |= ((2 << 40) - 1);	//Set first 40 bits
}

UID
DeviceManager_RegisterDevice(AOS_Devices *device)
{
    device->deviceID = new_uid();
    memcpy(&devices[free_device_index], device, sizeof(AOS_Devices));
    return device->deviceID;
}

uint32_t
DeviceManager_RequestMSIVector(uint8_t vector_count,
                               uint8_t *assigned_vector_count)
{
    //Find a continuous block of free MSI vectors
    int score = 0, prevScore = 0, scoreBase = 0, prevScoreBase = 0;
    for(int i = 0; i < 256; i++)
        {
            int index = i / 64;
            int off = i % 64;
            if (((msi_vectors[index] >> off) & 1) == 0)
                {
                    if(score == 0)scoreBase = i;
                    score++;

                    if(score >= vector_count)break;
                }
            else
                {
                    prevScore = score;
                    prevScoreBase = scoreBase;

                    scoreBase = 0;
                    score = 0;
                }
        }

    if(score == 0)
        {
            if(assigned_vector_count != NULL)*assigned_vector_count = prevScore;
            return prevScoreBase;
        }
    else
        {
            if(assigned_vector_count != NULL)*assigned_vector_count = score;
            return scoreBase;
        }
}

void
DeviceManager_TransitionPowerState(AOS_PowerStates pm_state)
{

}

void*
DeviceManager_RequestDMABlock(uint8_t pageCount)
{

}

void
DeviceManager_FreeDMABlock(void *dma_addr,
                           uint8_t pageCount)
{

}