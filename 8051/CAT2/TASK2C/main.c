#include <reg52.h>
#include "delay.h"

sbit USER_LED = P1^0;

#define LED_ON 0
#define LED_OFF 1
#define BLINK_HALF_MS 500

void main(void)
{
    USER_LED = LED_OFF;

    while (1)
    {
        USER_LED = LED_ON;
        delay_ms(BLINK_HALF_MS);

        USER_LED = LED_OFF;
        delay_ms(BLINK_HALF_MS);
    }
}
