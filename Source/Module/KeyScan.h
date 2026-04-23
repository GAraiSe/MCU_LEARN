#ifndef KEYSCAN_H
#define KEYSCAN_H

#include <stdint.h>

#define KEY_PUSH_FLAG   0x8000
#define KEY_SW3         0x0001
#define KEY_SW6         0x0002
#define KEY_SW10        0x0004
#define KEY_SW16        0x0008

uint16_t KeyScan(void);
void     KeyScan_Update(void);
uint8_t  SW3_Pressed(void);
uint8_t  SW6_Pressed(void);
uint8_t  SW10_Pressed(void);
uint8_t  SW16_Pressed(void);

#endif
