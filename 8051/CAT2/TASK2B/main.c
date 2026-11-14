#include <reg52.h>

sbit USER_LED = P1^0;

#define LED_ON 0
#define LED_OFF 1

#define HALF_PERIOD_PASSES 221

void soft_delay(unsigned int passes)
{
    unsigned int i;
    volatile unsigned char j;

    for (i = 0; i < passes; i++)
    {
        for (j = 0; j < 230; j++)
        {
        }
    }
}

void main(void)
{
    USER_LED = LED_OFF;

    while (1)
    {
        USER_LED = LED_ON;
        soft_delay(HALF_PERIOD_PASSES);

        USER_LED = LED_OFF;
        soft_delay(HALF_PERIOD_PASSES);
    }
}
