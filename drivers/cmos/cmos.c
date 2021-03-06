#include "cmos.h"

#include "acpi_tables/acpi_tables.h"
#include "acpi_tables/fadt.h"

#include "utils/native.h"

#define CMOS_REG_SELECT_PORT 0x70
#define CMOS_REG_SET_PORT 0x71

int century_register = 0;

void CMOS_Initialize()
{
    FADT *fadt = ACPITables_FindTable(FADT_SIG, 0);
    if(fadt != NULL)
        {
            century_register = fadt->Century;
        }
}

void CMOS_SelectRegister(uint8_t nmi_disabled, uint8_t cmos_reg)
{
    outb(CMOS_REG_SELECT_PORT, (nmi_disabled << 7) | cmos_reg);
}

void CMOS_SetRegister(uint8_t regVal, uint8_t nmi_disabled, uint8_t cmos_reg)
{
    CMOS_SelectRegister(nmi_disabled, cmos_reg);
    outb(CMOS_REG_SET_PORT, regVal);
}

uint8_t CMOS_GetRegister(uint8_t nmi_disabled, uint8_t cmos_reg)
{
    CMOS_SelectRegister(nmi_disabled, cmos_reg);
    return inb(CMOS_REG_SET_PORT);
}

uint8_t CMOS_UpdateInProgress()
{
    return CMOS_GetRegister(1, 0x0A) & 0x80;
}

void CMOS_GetRTCTime(RTC_Time *rtc)
{
    uint8_t second, minute, hour, day, month;
    uint16_t year;
    unsigned char century = 0;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (CMOS_UpdateInProgress()) ;             // Make sure an update isn't in progress
    second = CMOS_GetRegister(1, 0x00);
    minute = CMOS_GetRegister(1, 0x02);
    hour = CMOS_GetRegister(1, 0x04);
    day = CMOS_GetRegister(1, 0x07);
    month = CMOS_GetRegister(1, 0x08);
    year = CMOS_GetRegister(1, 0x09);
    if(century_register != 0)
        {
            century = CMOS_GetRegister(1, century_register);
        }

    do
        {
            last_second = second;
            last_minute = minute;
            last_hour = hour;
            last_day = day;
            last_month = month;
            last_year = year;
            last_century = century;

            while (CMOS_UpdateInProgress()) ;      // Make sure an update isn't in progress
            second = CMOS_GetRegister(1, 0x00);
            minute = CMOS_GetRegister(1, 0x02);
            hour = CMOS_GetRegister(1, 0x04);
            day = CMOS_GetRegister(1, 0x07);
            month = CMOS_GetRegister(1, 0x08);
            year = CMOS_GetRegister(1, 0x09);
            if(century_register != 0)
                {
                    century = CMOS_GetRegister(1, century_register);
                }
        }
    while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
            (last_day != day) || (last_month != month) || (last_year != year) ||
            (last_century != century) );

    registerB = CMOS_GetRegister(1, 0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04))
        {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(century_register != 0)
                {
                    century = (century & 0x0F) + ((century / 16) * 10);
                }
        }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (hour & 0x80))
        {
            hour = ((hour & 0x7F) + 12) % 24;
        }

    // Calculate the full (4-digit) year

    if(century_register != 0)
        {
            year += century * 100;
        }
    else
        {
            year += (CURRENT_YEAR / 100) * 100;
            if(year < CURRENT_YEAR) year += 100;
        }

    rtc->seconds = second;
    rtc->minutes = minute;
    rtc->hours = hour;
    rtc->dayOfMonth = day;
    rtc->month = month;
    rtc->year = year;
    rtc->century = century;

}
