#ifndef _DEVICE_MANAGER_H_
#define _DEVICE_MANAGER_H_

#include "types.h"
#include "drivers.h"
#include "managers.h"

#define MAX_ACTIVE_DEVICES 32
#define DMA_POOL_SIZE MB(8)

typedef enum
{
	AOS_D0 = 0,		//Normal operation
	AOS_D1 = 1,		//Lv1 power saving
	AOS_D2 = 2,		//Lv2 power saving
	AOS_D3 = 3,		//Lv3 power saving
}AOS_PowerStates;

typedef enum
{
	AOS_CHIPSET_DEVICE = 1,
	AOS_NETWORK_DEVICE = 2,
	AOS_STORAGE_DEVICE = 3,
	AOS_AUDIO_DEVICE = 4,
	AOS_HUMAN_INTERFACE_DEVICE = 5
}AOS_DeviceTypes;

typedef struct
{
	char name[256];
	uint32_t(*Initialize)(void);
	uint32_t(*Disable)(void);
	uint32_t(*Enable)(void);
	uint32_t(*SetPowerState)(AOS_PowerStates pm_state);
	void(*SaveState)(void);
	void(*RestoreState)(void);
	UID deviceID;
	AOS_DeviceTypes deviceType;
}AOS_Devices;


void
DeviceManager_Initialize(void);

UID
DeviceManager_RegisterDevice(AOS_Devices *device);

uint32_t
DeviceManager_RequestMSIVector(uint8_t vector_count);

void
DeviceManager_TransitionPowerState(AOS_PowerStates pm_state);

void*
DeviceManager_RequestDMABlock(uint8_t pageCount);	//Number of pages of DMA space requested

void
DeviceManager_FreeDMABlock(void *dma_addr, 
                           uint8_t pageCount);

#endif