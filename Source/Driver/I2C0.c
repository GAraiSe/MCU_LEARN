/******************** (C) COPYRIGHT 2020 SONiX *******************************
* COMPANY:		SONiX
* DATE:			2023/11
* AUTHOR:		SA1
* IC:			SN32F400
* DESCRIPTION:	I2C0 related functions.
*****************************************************************************/

#include <SN32F400.h>
#include "I2C.h"

#define I2C_TIMEOUT_VAL     100000u

/* Wait for I2C status change flag (bit 15), returns 0 on timeout */
static uint8_t i2c_wait(void)
{
    uint32_t t = I2C_TIMEOUT_VAL;
    while (((SN_I2C0->STAT & (1u << 15)) == 0u) && (--t));
    return (t > 0u) ? 1u : 0u;
}

void I2C0_IRQHandler(void)
{
    SN_I2C0->STAT = 1u << 15;
}

void I2C0_Init(void)
{
    SN_SYS1->AHBCLKEN_b.I2C0CLKEN = 1;
    SN_I2C0->SCLHT = I2C0_SCLHT;
    SN_I2C0->SCLLT = I2C0_SCLLT;
    SN_I2C0->CTRL_b.I2CEN = I2C_I2CEN_EN;
}

void I2C0_Start(void)
{
    SN_I2C0->STAT = 1u << 15;
    SN_I2C0->CTRL_b.STA = 1;
    i2c_wait();
}

void I2C0_Stop(void)
{
    SN_I2C0->STAT = 1u << 15;
    SN_I2C0->CTRL_b.STO = 1;
    i2c_wait();
}

uint8_t I2C_write_byte(uint8_t dat)
{
    SN_I2C0->STAT = 1u << 15;
    SN_I2C0->TXDATA = dat;
    if (!i2c_wait())
        return I2C_NACK_FALG;
    return (SN_I2C0->STAT & (1u << 1)) ? I2C_ACK_FALG : I2C_NACK_FALG;
}

uint8_t I2C_read_byte(uint8_t ack)
{
    SN_I2C0->STAT = 1u << 15;
    if (ack == I2C_ACK_FALG)
        SN_I2C0->CTRL_b.ACK = 1;
    else
        SN_I2C0->CTRL_b.NACK = 1;
    i2c_wait();
    return (uint8_t)SN_I2C0->RXDATA;
}
