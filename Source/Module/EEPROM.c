#include "EEPROM.h"
#include "..\Driver\I2C.h"

/*
 * External EEPROM: AT24C02 (256 bytes) via I2C0
 * Device address: 0xA0 (A2=A1=A0=GND)
 * Alarm stored at byte address 0x00..0x03:
 *   [0] magic  = 0xAD
 *   [1] hour   (0-23)
 *   [2] minute (0-59)
 *   [3] checksum = magic ^ hour ^ minute
 */

#define EEPROM_MAGIC        0xADu
#define EEPROM_ADDR_ALARM   0x00u

/* Write acknowledge polling: AT24C02 needs up to 5ms after write */
#define EEPROM_WRITE_POLL_MAX   200u

static uint8_t checksum(uint8_t hour, uint8_t minute)
{
    return (uint8_t)(EEPROM_MAGIC ^ hour ^ minute);
}

/* ---- low-level byte read/write ---- */

static void eeprom_write_byte(uint8_t mem_addr, uint8_t data)
{
    I2C0_Start();
    I2C_write_byte(Device_ADDRESS);         /* SLA+W */
    I2C_write_byte(mem_addr);
    I2C_write_byte(data);
    I2C0_Stop();

    /* Poll ACK until write cycle completes */
    uint16_t poll = 0;
    do {
        I2C0_Start();
        poll++;
    } while (I2C_write_byte(Device_ADDRESS) != I2C_ACK_FALG && poll < EEPROM_WRITE_POLL_MAX);
    I2C0_Stop();
}

static uint8_t eeprom_read_byte(uint8_t mem_addr)
{
    uint8_t val;

    /* Dummy write to set address pointer */
    I2C0_Start();
    I2C_write_byte(Device_ADDRESS);         /* SLA+W */
    I2C_write_byte(mem_addr);

    /* Repeated START then read */
    I2C0_Start();
    I2C_write_byte(Device_ADDRESS | 0x01u); /* SLA+R */
    val = I2C_read_byte(I2C_NACK_FALG);     /* single byte, send NACK */
    I2C0_Stop();

    return val;
}

/* ---- public API ---- */

void EEPROM_AlarmLoad(uint8_t *hour, uint8_t *minute)
{
    uint8_t magic, h, m, cs;

    if ((hour == 0) || (minute == 0))
        return;

    magic = eeprom_read_byte(EEPROM_ADDR_ALARM + 0u);
    h     = eeprom_read_byte(EEPROM_ADDR_ALARM + 1u);
    m     = eeprom_read_byte(EEPROM_ADDR_ALARM + 2u);
    cs    = eeprom_read_byte(EEPROM_ADDR_ALARM + 3u);

    if ((magic != EEPROM_MAGIC) || (h >= 24u) || (m >= 60u) || (cs != checksum(h, m)))
        return;

    *hour   = h;
    *minute = m;
}

void EEPROM_AlarmSave(uint8_t hour, uint8_t minute)
{
    if ((hour >= 24u) || (minute >= 60u))
        return;

    eeprom_write_byte(EEPROM_ADDR_ALARM + 0u, EEPROM_MAGIC);
    eeprom_write_byte(EEPROM_ADDR_ALARM + 1u, hour);
    eeprom_write_byte(EEPROM_ADDR_ALARM + 2u, minute);
    eeprom_write_byte(EEPROM_ADDR_ALARM + 3u, checksum(hour, minute));
}
