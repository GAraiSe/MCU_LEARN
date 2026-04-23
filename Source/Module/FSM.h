/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*****************************************************************************/
#ifndef __FSM_H
#define __FSM_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdint.h>

/*_____ D E F I N I T I O N S ______________________________________________*/

/* ------- STATES ------- */
typedef enum
{
    STATE_NORMAL = 0,
    STATE_SET_HOUR,
    STATE_SET_MIN,
    STATE_SET_ALARM_HOUR,
    STATE_SET_ALARM_MIN,
} FSM_State_t;

/* ------- EVENTS ------- */
typedef enum
{
    EV_NONE = 0,
    EV_SW3,           // SETUP button
    EV_SW16,          // ALARM button
    EV_SW6,           // + button
    EV_SW10,          // - button
    EV_TIMEOUT_30S,   // 30s no activity -> back to NORMAL
    EV_TICK_1S,       // 1 second tick from RTC
    EV_TICK_500MS,    // 500ms tick for blink
} FSM_Event_t;

/*_____ F U N C T I O N S __________________________________________________*/
void FSM_Init(void);
void FSM_Tick1ms(void);
void FSM_SendEvent(FSM_Event_t ev);
void FSM_Run(void);                     // call in main loop

/* Getters for display / alarm use */
FSM_State_t FSM_GetState(void);

#endif /* __FSM_H */
