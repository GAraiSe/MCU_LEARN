/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:  SONiX
* DATE:     2023/11
* AUTHOR:   SA1
* IC:       SN32F400
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "FSM.h"
#include "Segment.h"
#include "Buzzer.h"
#include "EEPROM.h"
#include <SN32F400.h>

/*_____ D E F I N I T I O N S ______________________________________________*/
#define TIMEOUT_30S_MAX     30u     // seconds until auto-return to NORMAL
#define BUZZER_PIP_MS       250u
#define BUZZER_ALARM_MS     5000u
#define BUZZER_ALARM_PHASE_MS 500u

/*_____ V A R I A B L E S __________________________________________________*/
static FSM_State_t  g_state        = STATE_NORMAL;
static FSM_Event_t  g_event        = EV_NONE;

/* Time */
static uint8_t  g_hour      = 12;
static uint8_t  g_minute    = 0;
static uint8_t  g_second    = 0;

/* Alarm */
static uint8_t  g_alarm_hour   = 0;
static uint8_t  g_alarm_minute = 0;
static uint8_t  g_alarm_active = 0;    // 1 = alarm is ringing

/* Timeout counter */
static uint8_t  g_timeout_cnt  = 0;    // counts seconds of inactivity

typedef enum
{
    BUZZER_MODE_IDLE = 0,
    BUZZER_MODE_PIP,
    BUZZER_MODE_ALARM,
} BuzzerMode_t;

static BuzzerMode_t g_buzzer_mode = BUZZER_MODE_IDLE;
static uint16_t g_buzzer_ms = 0;
static uint16_t g_buzzer_alarm_total_ms = 0;
static uint16_t g_buzzer_alarm_phase_ms = 0;
static uint8_t  g_buzzer_on = 0;
static uint8_t  g_alarm_save_pending = 0;
static uint8_t  g_alarm_saved_hour = 0;
static uint8_t  g_alarm_saved_minute = 0;

/*_____ P R I V A T E   F U N C T I O N S _________________________________*/

static void TimeIncrement(void)
{
    g_second++;
    if (g_second >= 60) { g_second = 0; g_minute++; }
    if (g_minute >= 60) { g_minute = 0; g_hour++;   }
    if (g_hour   >= 24) { g_hour   = 0;              }
}

static void Buzzer_StartPip(void)
{
    g_buzzer_mode = BUZZER_MODE_PIP;
    g_buzzer_ms = BUZZER_PIP_MS;
    Buzzer_On();
    g_buzzer_on = 1;
}

static void Buzzer_StartAlarmPattern(void)
{
    g_buzzer_mode = BUZZER_MODE_ALARM;
    g_buzzer_alarm_total_ms = BUZZER_ALARM_MS;
    g_buzzer_alarm_phase_ms = BUZZER_ALARM_PHASE_MS;
    Buzzer_On();
    g_buzzer_on = 1;
}

static void CheckAlarm(void)
{
    if ((g_hour == g_alarm_hour) && (g_minute == g_alarm_minute) && (g_second == 0))
    {
        g_alarm_active = 1;
        Buzzer_StartAlarmPattern();
    }
}

static void UpdateDisplay(void)
{
    switch (g_state)
    {
        case STATE_NORMAL:
        case STATE_SET_HOUR:
        case STATE_SET_MIN:
            Digital_DisplayTime(g_hour, g_minute);
            break;

        case STATE_SET_ALARM_HOUR:
        case STATE_SET_ALARM_MIN:
            Digital_DisplayTime(g_alarm_hour, g_alarm_minute);
            break;

        default:
            break;
    }
}

static void UpdateBlink(void)
{
    switch (g_state)
    {
        case STATE_NORMAL:
            Digital_SetBlink(0x00);     // no blink
            break;
        case STATE_SET_HOUR:
        case STATE_SET_ALARM_HOUR:
            Digital_SetBlink(0x03);     // blink digit 0 & 1 (hour)
            break;
        case STATE_SET_MIN:
        case STATE_SET_ALARM_MIN:
            Digital_SetBlink(0x0C);     // blink digit 2 & 3 (minute)
            break;
        default:
            Digital_SetBlink(0x00);
            break;
    }

    /* LED OFF on state transition - will sync on next EV_TICK_500MS */
    SN_GPIO3->BSET = (1u << 8);
}

/*_____ P U B L I C   F U N C T I O N S ___________________________________*/

/*****************************************************************************
* Function    : FSM_Init
*****************************************************************************/
void FSM_Init(void)
{
    g_state     = STATE_NORMAL;
    g_event     = EV_NONE;
    g_hour      = 0;
    g_minute    = 0;
    g_second    = 0;
    g_timeout_cnt = 0;
    g_alarm_hour = 0;
    g_alarm_minute = 0;
    g_alarm_active = 0;
    g_buzzer_mode = BUZZER_MODE_IDLE;
    g_buzzer_ms = 0;
    g_buzzer_alarm_total_ms = 0;
    g_buzzer_alarm_phase_ms = 0;
    g_buzzer_on = 0;
    g_alarm_save_pending = 0;
    Buzzer_Off();
    EEPROM_AlarmLoad(&g_alarm_hour, &g_alarm_minute);
    g_alarm_saved_hour = g_alarm_hour;
    g_alarm_saved_minute = g_alarm_minute;

    UpdateDisplay();
    UpdateBlink();
}

void FSM_Tick1ms(void)
{
    if (g_buzzer_mode == BUZZER_MODE_PIP)
    {
        if (g_buzzer_ms > 0u)
            g_buzzer_ms--;

        if (g_buzzer_ms == 0u)
        {
            g_buzzer_mode = BUZZER_MODE_IDLE;
            Buzzer_Off();
            g_buzzer_on = 0;
        }
    }
    else if (g_buzzer_mode == BUZZER_MODE_ALARM)
    {
        if (g_buzzer_alarm_total_ms > 0u)
            g_buzzer_alarm_total_ms--;
        if (g_buzzer_alarm_phase_ms > 0u)
            g_buzzer_alarm_phase_ms--;

        if (g_buzzer_alarm_total_ms == 0u)
        {
            g_buzzer_mode = BUZZER_MODE_IDLE;
            g_alarm_active = 0;
            Buzzer_Off();
            g_buzzer_on = 0;
            return;
        }

        if (g_buzzer_alarm_phase_ms == 0u)
        {
            g_buzzer_alarm_phase_ms = BUZZER_ALARM_PHASE_MS;
            g_buzzer_on ^= 1u;
            if (g_buzzer_on) Buzzer_On(); else Buzzer_Off();
        }
    }
}

void FSM_Task(void)
{
    if (g_alarm_save_pending)
    {
        EEPROM_AlarmSave(g_alarm_hour, g_alarm_minute);
        g_alarm_saved_hour = g_alarm_hour;
        g_alarm_saved_minute = g_alarm_minute;
        g_alarm_save_pending = 0;
    }
}

/*****************************************************************************
* Function    : FSM_SendEvent
* Description : Push an event (called from ISR or main loop)
*****************************************************************************/
void FSM_SendEvent(FSM_Event_t ev)
{
    g_event = ev;   // simple single-slot queue; extend to FIFO if needed
}

/*****************************************************************************
* Function    : FSM_GetState
*****************************************************************************/
FSM_State_t FSM_GetState(void)
{
    return g_state;
}

/*****************************************************************************
* Function    : FSM_Run
* Description : Process pending event - call from main loop
*****************************************************************************/
void FSM_Run(void)
{
    FSM_Event_t ev;
    FSM_State_t prev_state;

    if (g_event == EV_NONE)
        return;

    /* Atomically fetch & clear */
    ev      = g_event;
    g_event = EV_NONE;
    prev_state = g_state;

    /* ---- Tick 500ms: blink toggle (any state) ---- */
    if (ev == EV_TICK_500MS)
    {
        Digital_BlinkTick();

        /* LED D6 (P3.8): synced with blink_state - same source as segment, active-low */
        if (g_state == STATE_SET_ALARM_HOUR || g_state == STATE_SET_ALARM_MIN)
        {
            if (Digital_GetBlinkState())
                SN_GPIO3->BSET = (1u << 8);   // OFF (active-low = HIGH)
            else
                SN_GPIO3->BCLR = (1u << 8);   // ON  (active-low = LOW)
        }
        else
            SN_GPIO3->BSET = (1u << 8);       // OFF

        return;
    }

    /* ---- Tick 1s: time advance + timeout counter ---- */
    if (ev == EV_TICK_1S)
    {
        TimeIncrement();
        CheckAlarm();

        if (g_state != STATE_NORMAL)
        {
            g_timeout_cnt++;
            if (g_timeout_cnt >= TIMEOUT_30S_MAX)
            {
                g_timeout_cnt = 0;
                FSM_SendEvent(EV_TIMEOUT_30S);
                return;
            }
        }

        if (g_state == STATE_NORMAL)
            UpdateDisplay();    // refresh clock every second

        return;
    }

    /* ---- Reset timeout on any button press ---- */
    if (ev == EV_SW3 || ev == EV_SW16 || ev == EV_SW6 || ev == EV_SW10)
    {
        g_timeout_cnt = 0;
        Buzzer_StartPip();
    }

    /* ===== STATE MACHINE ===== */
    switch (g_state)
    {
        /* ----- NORMAL ----- */
        case STATE_NORMAL:
            if (ev == EV_SW3)
            {
                g_state = STATE_SET_HOUR;
            }
            else if (ev == EV_SW16)
            {
                g_state = STATE_SET_ALARM_HOUR;
            }
            break;

        /* ----- SET_HOUR ----- */
        case STATE_SET_HOUR:
            if (ev == EV_SW3)
            {
                g_state = STATE_SET_MIN;
            }
            else if (ev == EV_SW6)
            {
                g_hour = (g_hour + 1) % 24;
            }
            else if (ev == EV_SW10)
            {
                g_hour = (g_hour == 0) ? 23 : g_hour - 1;
            }
            else if (ev == EV_TIMEOUT_30S)
            {
                g_state = STATE_NORMAL;
            }
            break;

        /* ----- SET_MIN ----- */
        case STATE_SET_MIN:
            if (ev == EV_SW3)
            {
                g_state = STATE_NORMAL;
            }
            else if (ev == EV_SW6)
            {
                g_minute = (g_minute + 1) % 60;
            }
            else if (ev == EV_SW10)
            {
                g_minute = (g_minute == 0) ? 59 : g_minute - 1;
            }
            else if (ev == EV_TIMEOUT_30S)
            {
                g_state = STATE_NORMAL;
            }
            break;

        /* ----- SET_ALARM_HOUR ----- */
        case STATE_SET_ALARM_HOUR:
            if (ev == EV_SW16)
            {
                g_state = STATE_SET_ALARM_MIN;
            }
            else if (ev == EV_SW6)
            {
                g_alarm_hour = (g_alarm_hour + 1) % 24;
            }
            else if (ev == EV_SW10)
            {
                g_alarm_hour = (g_alarm_hour == 0) ? 23 : g_alarm_hour - 1;
            }
            else if (ev == EV_TIMEOUT_30S)
            {
                g_state = STATE_NORMAL;
            }
            break;

        /* ----- SET_ALARM_MIN ----- */
        case STATE_SET_ALARM_MIN:
            if (ev == EV_SW16)
            {
                g_state = STATE_NORMAL;
            }
            else if (ev == EV_SW6)
            {
                g_alarm_minute = (g_alarm_minute + 1) % 60;
            }
            else if (ev == EV_SW10)
            {
                g_alarm_minute = (g_alarm_minute == 0) ? 59 : g_alarm_minute - 1;
            }
            else if (ev == EV_TIMEOUT_30S)
            {
                g_state = STATE_NORMAL;
            }
            break;

        default:
            g_state = STATE_NORMAL;
            break;
    }

    UpdateDisplay();
    UpdateBlink();

    /* LED D6: ensure OFF when leaving alarm-set states */
    if (g_state != STATE_SET_ALARM_HOUR && g_state != STATE_SET_ALARM_MIN)
        SN_GPIO3->BSET = (1u << 8);   // OFF (active-low = HIGH)

    if ((prev_state != STATE_NORMAL) && (g_state == STATE_NORMAL))
    {
        Buzzer_StartPip();
    }

    if (((prev_state == STATE_SET_ALARM_HOUR) || (prev_state == STATE_SET_ALARM_MIN)) &&
        (g_state == STATE_NORMAL) &&
        ((g_alarm_hour != g_alarm_saved_hour) || (g_alarm_minute != g_alarm_saved_minute)))
    {
        g_alarm_save_pending = 1;
    }
}
