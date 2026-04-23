#ifndef __BUZZER_H
#define __BUZZER_H

#include <stdint.h>

typedef enum
{
    BUZZER_PITCH_C4 = 0,
    BUZZER_PITCH_D4,
    BUZZER_PITCH_E4,
    BUZZER_PITCH_F4,
    BUZZER_PITCH_G4,
    BUZZER_PITCH_A4,
    BUZZER_PITCH_B4,
    BUZZER_PITCH_C5,
    BUZZER_PITCH_D5,
    BUZZER_PITCH_E5,
    BUZZER_PITCH_MAX
} BuzzerPitch_t;

void Buzzer_Init(void);
void Buzzer_SetPitch(BuzzerPitch_t pitch);
void Buzzer_Stop(void);

#endif /* __BUZZER_H */
