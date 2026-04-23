#include "EEPROM.h"
#include "..\Driver\Flash.h"
#include <string.h>

/* Reserve last Flash page for app data (32KB ROM -> last 512-byte page). */
#define EEPROM_PAGE_ADDR      0x00007E00u
#define EEPROM_MAGIC_ALARM    0x414C524Du  /* "ALRM" */

static uint8_t s_page_buf[FLASH_PAGE_SIZE];

typedef struct
{
    uint32_t magic;
    uint8_t hour;
    uint8_t minute;
    uint8_t reserved0;
    uint8_t reserved1;
    uint32_t checksum;
} AlarmRecord_t;

static void EEPROM_FlashErasePage(uint32_t page_addr)
{
    __FLASH_CLEAR_ERROR_STATUS;
    SN_FLASH->CTRL = FLASH_PER;
    SN_FLASH->ADDR = page_addr;
    __FLASH_START_OPERATION;
}

static void EEPROM_FlashProgramPage(uint32_t page_addr, const uint8_t *data, uint32_t size)
{
    uint32_t i;
    uint32_t adr = page_addr;
    uint32_t bytes = (size > FLASH_PAGE_SIZE) ? FLASH_PAGE_SIZE : size;

    __FLASH_CLEAR_ERROR_STATUS;
    SN_FLASH->CTRL = FLASH_PG;
    SN_FLASH->ADDR = adr;

    for (i = 0; i < bytes; i += 4u)
    {
        uint32_t word_data =
            ((uint32_t)data[i]) |
            ((uint32_t)data[i + 1u] << 8) |
            ((uint32_t)data[i + 2u] << 16) |
            ((uint32_t)data[i + 3u] << 24);

        SN_FLASH->DATA = word_data;

        if ((((adr >> 2) % 2u) == 1u) || ((i + 4u) >= bytes))
        {
            SN_FLASH->CTRL |= FLASH_START;
            __FLASH_WAIT_FOR_DONE;
        }

        adr += 4u;
        if (((adr >> 2) % 2u) == 0u)
        {
            SN_FLASH->CTRL = FLASH_PG;
            SN_FLASH->ADDR = adr;
        }
    }
}

static uint32_t AlarmChecksum(uint8_t hour, uint8_t minute)
{
    return (EEPROM_MAGIC_ALARM ^ ((uint32_t)hour << 8) ^ (uint32_t)minute ^ 0x5A5AA5A5u);
}

void EEPROM_AlarmLoad(uint8_t *hour, uint8_t *minute)
{
    const AlarmRecord_t *rec = (const AlarmRecord_t *)EEPROM_PAGE_ADDR;

    if ((hour == 0) || (minute == 0))
        return;

    if ((rec->magic != EEPROM_MAGIC_ALARM) ||
        (rec->hour >= 24u) ||
        (rec->minute >= 60u) ||
        (rec->checksum != AlarmChecksum(rec->hour, rec->minute)))
    {
        return;
    }

    *hour = rec->hour;
    *minute = rec->minute;
}

void EEPROM_AlarmSave(uint8_t hour, uint8_t minute)
{
    AlarmRecord_t rec;
    const uint8_t *flash_page = (const uint8_t *)EEPROM_PAGE_ADDR;

    if ((hour >= 24u) || (minute >= 60u))
        return;

    /* Preserve the whole last page, especially the reserved word at 0x7FFC. */
    memcpy(s_page_buf, flash_page, sizeof(s_page_buf));

    rec.magic = EEPROM_MAGIC_ALARM;
    rec.hour = hour;
    rec.minute = minute;
    rec.reserved0 = 0xFF;
    rec.reserved1 = 0xFF;
    rec.checksum = AlarmChecksum(hour, minute);

    memcpy(s_page_buf, &rec, sizeof(rec));

    EEPROM_FlashErasePage(EEPROM_PAGE_ADDR);
    EEPROM_FlashProgramPage(EEPROM_PAGE_ADDR, s_page_buf, FLASH_PAGE_SIZE);
}
