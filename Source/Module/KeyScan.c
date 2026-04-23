#include "KeyScan.h"
#include "SN32F400.h"

static uint16_t key_last = 0;
static uint16_t key_press_edge = 0;

uint16_t KeyScan(void)
{
    uint16_t result = 0;
    uint8_t  col;

    // --- Qut ROW P1.4 ---
    SN_GPIO1->DATA |=  (0xF << 4);   // t?t c? ROW = HIGH
    SN_GPIO1->DATA &= ~(1 << 4);     // P1.4 = LOW
    for(volatile int i = 0; i < 10; i++);

    col = (SN_GPIO2->DATA >> 4) & 0xF;
    // SW3:  COL P2.4 = bit0 c?a col
    if (!(col & (1 << 0))) result |= KEY_SW3;
    // SW6:  COL P2.7 = bit3 c?a col
    if (!(col & (1 << 3))) result |= KEY_SW6;

    // --- Qut ROW P1.5 ---
    SN_GPIO1->DATA |=  (0xF << 4);   // t?t c? ROW = HIGH
    SN_GPIO1->DATA &= ~(1 << 5);     // P1.5 = LOW
    for(volatile int i = 0; i < 10; i++);

    col = (SN_GPIO2->DATA >> 4) & 0xF;
    // SW10: COL P2.7 = bit3 c?a col
    if (!(col & (1 << 3))) result |= KEY_SW10;

    // --- Qut ROW P1.7 ---
    SN_GPIO1->DATA |=  (0xF << 4);   // t?t c? ROW = HIGH
    SN_GPIO1->DATA &= ~(1 << 7);     // P1.7 = LOW
    for(volatile int i = 0; i < 10; i++);

    col = (SN_GPIO2->DATA >> 4) & 0xF;
    // SW16: COL P2.4 = bit0 c?a col
    if (!(col & (1 << 0))) result |= KEY_SW16;

    // Tr? ROW v? HIGH
    SN_GPIO1->DATA |= (0xF << 4);

    if (result != 0)
        result |= KEY_PUSH_FLAG;

    return result;
}

void KeyScan_Update(void)
{
    uint16_t key_now = KeyScan();
    uint16_t key_mask = (KEY_SW3 | KEY_SW6 | KEY_SW10 | KEY_SW16);

    /* Detect press edge (0 -> 1) on each key bit */
    key_press_edge = (uint16_t)((~key_last) & key_now & key_mask);
    key_last = key_now;
}

uint8_t SW3_Pressed(void)
{
    if (key_press_edge & KEY_SW3)
    {
        key_press_edge &= ~KEY_SW3;
        return 1;
    }
    return 0;
}

uint8_t SW6_Pressed(void)
{
    if (key_press_edge & KEY_SW6)
    {
        key_press_edge &= ~KEY_SW6;
        return 1;
    }
    return 0;
}

uint8_t SW10_Pressed(void)
{
    if (key_press_edge & KEY_SW10)
    {
        key_press_edge &= ~KEY_SW10;
        return 1;
    }
    return 0;
}

uint8_t SW16_Pressed(void)
{
    if (key_press_edge & KEY_SW16)
    {
        key_press_edge &= ~KEY_SW16;
        return 1;
    }
    return 0;
}