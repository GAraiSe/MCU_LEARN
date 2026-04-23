/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
*****************************************************************************/
#ifndef __SEGMENT_H
#define __SEGMENT_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdint.h>

/*_____ D E F I N I T I O N S ______________________________________________*/
#define SEGMENT_BLANK   0x00
#define SEGMENT_DASH    0x40   // '-'

/*_____ F U N C T I O N S __________________________________________________*/
void Digital_DisplayDEC(uint16_t dec);
void Digital_DisplayHEX(uint16_t hex);
void Digital_DisplayTime(uint8_t hour, uint8_t minute);
void Digital_SetBlink(uint8_t digit_mask);   // bit0=digit0, bit1=digit1, ...
void Digital_BlinkTick(void);                // toggle blink state, call every 500ms
void Digital_Scan(void);

#endif /* __SEGMENT_H */