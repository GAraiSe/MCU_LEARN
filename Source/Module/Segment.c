/******************** (C) COPYRIGHT 2021 SONiX ******************************/

#include "Segment.h"
#include "..\Driver\GPIO.h"

/* SEGMENT DEFINE */
#define SEG_A   0x01
#define SEG_B   0x02
#define SEG_C   0x04
#define SEG_D   0x08
#define SEG_E   0x10
#define SEG_F   0x20
#define SEG_G   0x40
#define SEG_H   0x80

const uint8_t SEGMENT_TABLE[] = {
    (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),           
    (SEG_B|SEG_C),                                   
    (SEG_A|SEG_B|SEG_D|SEG_E|SEG_G),                
    (SEG_A|SEG_B|SEG_C|SEG_D|SEG_G),                
    (SEG_B|SEG_C|SEG_F|SEG_G),                      
    (SEG_A|SEG_C|SEG_D|SEG_F|SEG_G),                
    (SEG_A|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G),          
    (SEG_A|SEG_B|SEG_C),                            
    (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G),    
    (SEG_A|SEG_B|SEG_C|SEG_D|SEG_F|SEG_G),          
    (SEG_A|SEG_B|SEG_C|SEG_E|SEG_F|SEG_G),          
    (SEG_C|SEG_D|SEG_E|SEG_F|SEG_G),                
    (SEG_A|SEG_D|SEG_E|SEG_F),                      
    (SEG_B|SEG_C|SEG_D|SEG_E|SEG_G),                
    (SEG_A|SEG_D|SEG_E|SEG_F|SEG_G),                
    (SEG_A|SEG_E|SEG_F|SEG_G),                      
};

/* VARIABLES */
static uint8_t com_scan = 0;
static uint8_t segment_buff[4];

static uint8_t blink_mask = 0;
static uint8_t blink_state = 0;

/* ========================= DISPLAY FUNCTIONS ========================= */

void Digital_DisplayDEC(uint16_t dec)
{
    if (dec > 9999)
    {
        for (int i = 0; i < 4; i++)
            segment_buff[i] = SEG_G;
    }
    else
    {
        segment_buff[3] = SEGMENT_TABLE[dec % 10]; dec /= 10;
        segment_buff[2] = SEGMENT_TABLE[dec % 10]; dec /= 10;
        segment_buff[1] = SEGMENT_TABLE[dec % 10]; dec /= 10;
        segment_buff[0] = SEGMENT_TABLE[dec % 10];
    }
}

void Digital_DisplayHEX(uint16_t hex)
{
    segment_buff[3] = SEGMENT_TABLE[hex & 0xF]; hex >>= 4;
    segment_buff[2] = SEGMENT_TABLE[hex & 0xF]; hex >>= 4;
    segment_buff[1] = SEGMENT_TABLE[hex & 0xF]; hex >>= 4;
    segment_buff[0] = SEGMENT_TABLE[hex & 0xF];
}

void Digital_DisplayTime(uint8_t hour, uint8_t minute)
{
    segment_buff[0] = SEGMENT_TABLE[hour / 10];
    segment_buff[1] = SEGMENT_TABLE[hour % 10];
    segment_buff[2] = SEGMENT_TABLE[minute / 10];
    segment_buff[3] = SEGMENT_TABLE[minute % 10];
}

/* BLINK CONTROL */
void Digital_SetBlink(uint8_t digit_mask)
{
    if (blink_mask != digit_mask)
    {
        /* Restart blink phase whenever we enter/switch edit fields:
           start visible, then hide after the next 500 ms tick. */
        blink_state = 0;
    }

    blink_mask = digit_mask;
}

void Digital_BlinkTick(void)
{
    blink_state ^= 1;
}

uint8_t Digital_GetBlinkState(void)
{
    return blink_state;
}

/* ========================= CORE SCAN ========================= */

static inline void COM_AllOff(void)
{
    SN_GPIO1->BCLR = (0xF << 9);
}

static inline void SEG_AllOff(void)
{
    SN_GPIO0->BCLR = 0x7F;   // A-G
}

static inline void COM_On(uint8_t idx)
{
    switch (idx)
    {
        case 0: SN_GPIO1->BSET = (1 << 9);  break;
        case 1: SN_GPIO1->BSET = (1 << 10); break;
        case 2: SN_GPIO1->BSET = (1 << 11); break;
        case 3: SN_GPIO1->BSET = (1 << 12); break;
    }
}

/* ========================= SCAN ISR ========================= */
/* CALL EVERY ~1–2ms */

void Digital_Scan(void)
{
    /* OFF ALL */
    COM_AllOff();
    SEG_AllOff();

    /* NEXT DIGIT */
    com_scan++;
    if (com_scan >= 4) com_scan = 0;

    /* BLINK CONTROL */
    if ((blink_mask >> com_scan) & 0x01)
    {
        if (blink_state)
            return;   // digit hidden: do not drive COM or SEG
    }

    /* OUTPUT SEGMENT */
    SN_GPIO0->BSET = segment_buff[com_scan];

    /* ENABLE COM */
    COM_On(com_scan);

    __NOP();
    __NOP();
}