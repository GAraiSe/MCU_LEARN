/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "RTC.h"
#include "FSM.h"
#include "SN32F400.h"

/*_____ D E F I N I T I O N S ______________________________________________*/
// SN32F400 RTC register bits
#define RTC_CTRL_RTCEN      (1 << 0)
#define RTC_IE_SECIEN       (1 << 0)
#define RTC_IC_SECIC        (1 << 0)

/*_____ F U N C T I O N S __________________________________________________*/

/*****************************************************************************
* Function    : RTC_Init
* Description : Initialize RTC with ILRC as clock source, 1-second interrupt
*****************************************************************************/
void RTC_Init(void)
{
    // Enable AHB clock for RTC
    SN_SYS1->AHBCLKEN |= (1 << 23);    // RTCCLKEN

    // Pulse RTC reset so the peripheral starts from a known state.
    SN_SYS1->PRST |= (1 << 23);        // RTCRST assert
    SN_SYS1->PRST &= ~(1 << 23);       // RTCRST release

    // Select ILRC as RTC clock source (default)
    SN_RTC->CLKS = 0x0;

    // Set reload value for 1 second (ILRC ~32KHz -> 32768-1)
    SN_RTC->SECCNTV = 32767;

    // Clear any stale second interrupt flag left from a previous run/reset.
    SN_RTC->IC = RTC_IC_SECIC;

    // Enable RTC second interrupt
    SN_RTC->IE = RTC_IE_SECIEN;

    // Enable RTC
    SN_RTC->CTRL = RTC_CTRL_RTCEN;

    // Enable RTC interrupt in NVIC
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
}

/*****************************************************************************
* Function    : RTC_IRQHandler
* Description : Called every 1 second by RTC interrupt
*****************************************************************************/
void RTC_IRQHandler(void)
{
    // Clear interrupt flag
    SN_RTC->IC = RTC_IC_SECIC;

    // Push 1-second tick event into FSM
    FSM_SendEvent(EV_TICK_1S);
}
