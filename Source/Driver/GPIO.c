#include "GPIO.h"

void GPIO_Init(void)
{
    // -------------------------------------------------------
    // 7-SEG: SEG A~DP = P0.0 ~ P0.7 (output, init LOW = off)
    // -------------------------------------------------------
    SN_GPIO0->MODE |= 0xFF;          // P0.0~P0.7 = output
    SN_GPIO0->BCLR  = 0xFF;          // t?t t?t c? segment

    // -------------------------------------------------------
    // 7-SEG: COM0~COM3 = P1.9 ~ P1.12 (output, init LOW = t?t)
    // COM dùng BJT: base HIGH -> transistor ON -> COM kéo GND -> digit sáng
    // -------------------------------------------------------
    SN_GPIO1->MODE |= (0xF << 9);    // P1.9~P1.12 = output
    SN_GPIO1->BCLR  = (0xF << 9);    // t?t t?t c? COM

    // -------------------------------------------------------
    // KeyScan ROW: P1.4 ~ P1.7 (output, init HIGH)
    // -------------------------------------------------------
    SN_GPIO1->MODE |= (0xF << 4);
    SN_GPIO1->DATA |= (0xF << 4);

    // -------------------------------------------------------
    // KeyScan COL: P2.4 ~ P2.7 (input, pull-up)
    // -------------------------------------------------------
    SN_GPIO2->MODE &= ~(0xF << 4);
    SN_GPIO2->CFG  &= ~(0xFF << 8);  // CFG4~7 = 00 = pull-up
}
