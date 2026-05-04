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
* Description : Initialize RTC with ELS (32.768kHz crystal) or fallback to ILRC
*****************************************************************************/
void RTC_Init(void)
{
    uint32_t timeout;
    uint8_t use_els = 0;

    // Enable AHB clock for RTC
    SN_SYS1->AHBCLKEN |= (1 << 23);    // RTCCLKEN

    // Pulse RTC reset so the peripheral starts from a known state.
    SN_SYS1->PRST |= (1 << 23);        // RTCRST assert
    SN_SYS1->PRST &= ~(1 << 23);       // RTCRST release

    // Try to enable ELS (External 32.768 kHz crystal)
    SN_SYS0->ANBCTRL |= (1 << 2);      // ELSEN = 1 (enable ELS)
    
    // Wait for ELS ready with timeout (~100ms)
    timeout = 100000;
    while (timeout--)
    {
        if (SN_SYS0->CSST & (1 << 1))  // Check ELS ready flag
        {
            use_els = 1;
            break;
        }
    }

    if (use_els)
    {
        // ELS is ready - use external 32.768 kHz crystal
        SN_RTC->CLKS = 1;              // RTC_CLKSEL_ELS
        SN_RTC->SECCNTV = 32767;       // 32768 - 1 (accurate)
        
        // Debug: Toggle P0.0 to indicate ELS is used
        SN_GPIO0->DATA ^= (1 << 0);
        SN_GPIO0->DATA ^= (1 << 0);
        SN_GPIO0->DATA ^= (1 << 0);    // 3 quick toggles = ELS
    }
    else
    {
        // ELS not available - fallback to ILRC
        SN_SYS0->ANBCTRL &= ~(1 << 2); // Disable ELS to save power
        SN_RTC->CLKS = 0;              // RTC_CLKSEL_ILRC
        // Calibrated for ILRC: 51s real = 60s MCU → ~38554 Hz
        SN_RTC->SECCNTV = 38553;       // Calibrated value
        
        // Debug: Toggle P0.0 to indicate ILRC is used
        SN_GPIO0->DATA ^= (1 << 0);    // 1 toggle = ILRC
    }

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

/*****************************************************************************
* Function    : RTC_GetClockSource
* Description : Return current RTC clock source (for debug)
* Return      : 0 = ILRC, 1 = ELS
*****************************************************************************/
uint8_t RTC_GetClockSource(void)
{
    return (SN_RTC->CLKS & 0x01);
}
