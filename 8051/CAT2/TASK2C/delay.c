#include "delay.h"

void delay_ms(unsigned int ms)
{
    unsigned int i;
    volatile unsigned char j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < DELAY_1MS_INNER; j++);
        for (j = 0; j < DELAY_1MS_INNER; j++);
    }
}

void delay_50us(unsigned char n)
{
    volatile unsigned char j;

    while (n--)
    {
        for (j = 0; j < 4; j++);
    }
}
