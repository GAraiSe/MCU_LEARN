#ifndef __EEPROM_H
#define __EEPROM_H

#include <stdint.h>

void EEPROM_AlarmLoad(uint8_t *hour, uint8_t *minute);
void EEPROM_AlarmSave(uint8_t hour, uint8_t minute);

#endif /* __EEPROM_H */
