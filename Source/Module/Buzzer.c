#include "Buzzer.h"
#include "SN32F400.h"
#include "..\Driver\CT16.h"
#include "..\Driver\CT16B0.h"

#define BUZZER_HCLK_FREQ          12000000UL
#define BUZZER_PWM0_SEL_POS       0u
#define BUZZER_PWM0_SEL_MASK      (0x3u << BUZZER_PWM0_SEL_POS)
#define BUZZER_PWM0_SEL_P30       (0x1u << BUZZER_PWM0_SEL_POS)
#define BUZZER_TONE_HZ            2800u

static uint8_t s_buzzer_enabled = 0;

void Buzzer_Init(void)
{
    __CT16B0_ENABLE;
    __CT16B0_CLKSEL_HCLK;

    /* Route CT16B0 PWM0 output to P3.0 */
    SN_PFPA->CT16B0 = (SN_PFPA->CT16B0 & ~BUZZER_PWM0_SEL_MASK) | BUZZER_PWM0_SEL_P30;

    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->CNTCTRL = mskCT16_CTM_TIMER;
    SN_CT16B0->MCTRL   = mskCT16_MR9RST_EN;
    SN_CT16B0->PWMCTRL = (mskCT16_PWM0EN_EN | mskCT16_PWM0MODE_1 | mskCT16_PWM0IOEN_EN);
    SN_CT16B0->EM      = mskCT16_EM0_LOW;

    s_buzzer_enabled = 1;

    Buzzer_Off();
}

void Buzzer_On(void)
{
    uint32_t period;

    if (!s_buzzer_enabled)
        return;

    period = BUZZER_HCLK_FREQ / BUZZER_TONE_HZ;
    if (period < 2u)
        period = 2u;

    SN_CT16B0->MR9 = (uint16_t)period;
    SN_CT16B0->MR0 = (uint16_t)(period / 2u);

    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->TMRCTRL = 1;
}

void Buzzer_Off(void)
{
    if (!s_buzzer_enabled)
        return;
    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->MR0 = 0;
}
