/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*****************************************************************************/
#ifndef __RTC_H
#define __RTC_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdint.h>

/*_____ F U N C T I O N S __________________________________________________*/
void RTC_Init(void);
uint8_t RTC_GetClockSource(void);  // 0=ILRC, 1=ELS

#endif /* __RTC_H */
