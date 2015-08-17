#ifndef _DRIVERS_ALL_H_
#define _DRIVERS_ALL_H_

#include "acpi_tables/acpi_tables.h"  //ACPI Tables Parser
#include "apic/apic.h"                //Local APIC Driver
#include "cmos/cmos.h"                //CMOS Time Driver
#include "fpu/fpu.h"                  //VFPU Driver
#include "hpet/hpet.h"                //HPET Driver
#include "pic/pic.h"                  //PIC Driver
#include "pit/pit.h"                  //PIT Driver
#include "pci/pci.h"                  //PCI Driver
#include "ps2/ps2.h"                  //PS/2 Controller Driver
#include "serial/COM.h"               //Serial Port Driver

#endif /* end of include guard: _DRIVERS_ALL_H_ */
