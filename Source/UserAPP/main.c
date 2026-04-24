/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*____________________________________________________________________________
*  REVISION  Date        User  Description
*  1.0       2023/11/07  SA1   First version released
*  2.0       2024/xx/xx  SA1   Add FSM, RTC, 7-Seg display
*____________________________________________________________________________
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "SN32F400.h"
#include "..\Driver\GPIO.h"
#include "..\Module\KeyScan.h"
#include "..\Module\Buzzer.h"
#include "..\Module\Segment.h"
#include "..\Module\RTC.h"
#include "..\Module\FSM.h"

/*_____ D E F I N I T I O N S ______________________________________________*/
#define SCAN_PERIOD_MS      1u      // Digital_Scan call period (every 1ms)
#define BLINK_PERIOD_MS     500u    // 500ms blink toggle

/*_____ V A R I A B L E S __________________________________________________*/
static volatile uint32_t g_tick_ms      = 0;    // millisecond counter from SysTick
static volatile uint8_t  g_flag_scan    = 0;    // set every 2ms
static volatile uint8_t  g_flag_500ms   = 0;    // set every 500ms

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function    : SysTick_Handler
* Description : 1ms SysTick interrupt
*****************************************************************************/
void SysTick_Handler(void)
{
    g_tick_ms++;

    /* 2ms flag -> 7-seg scan */
    if ((g_tick_ms % SCAN_PERIOD_MS) == 0)
        g_flag_scan = 1;

    /* 500ms flag -> blink */
    if ((g_tick_ms % BLINK_PERIOD_MS) == 0)
        g_flag_500ms = 1;
}

/*****************************************************************************
* Function    : main
*****************************************************************************/
int main(void)
{
    uint32_t tick_last = 0;
    uint8_t sw3_pressed;
    uint8_t sw16_pressed;
    uint8_t sw6_pressed;
    uint8_t sw10_pressed;

    /* ---- Hardware init ---- */
    GPIO_Init();

    /* ---- SysTick: 1ms @ 48MHz IHRC ---- */
    SysTick_Config(48000);          // 48 000 000 / 48 000 = 1ms

    /* ---- Module init ---- */
    Buzzer_Init();
    FSM_Init();
    RTC_Init();
    KeyScan_Update();

    /* ---- Main loop ---- */
    while (1)
    {
        if (g_tick_ms != tick_last)
        {
            tick_last = g_tick_ms;
            FSM_Tick1ms();
        }

        /* 2ms: refresh 7-seg display */
        if (g_flag_scan)
        {
            g_flag_scan = 0;
            Digital_Scan();
        }

        /* 500ms: blink tick */
        if (g_flag_500ms)
        {
            g_flag_500ms = 0;
            FSM_SendEvent(EV_TICK_500MS);
        }

        KeyScan_Update();

        sw3_pressed = SW3_Pressed();
        sw16_pressed = SW16_Pressed();
        sw6_pressed = SW6_Pressed();
        sw10_pressed = SW10_Pressed();

        if (sw3_pressed)   FSM_SendEvent(EV_SW3);
        if (sw16_pressed)  FSM_SendEvent(EV_SW16);
        if (sw6_pressed)   FSM_SendEvent(EV_SW6);
        if (sw10_pressed)  FSM_SendEvent(EV_SW10);

        /* Run FSM */
        FSM_Run();
        FSM_Task();
    }
}
