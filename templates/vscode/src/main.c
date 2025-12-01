/*
 * MCF52235 Template Project - LED Blink Example
 *
 * This simple program blinks LEDs on the M52233DEMO board
 * to verify the toolchain and debugger are working correctly.
 */

#include "mcf52235.h"

/* Global variable for testing debugger */
volatile uint32_t blink_count = 0;

int main(void)
{
    /* Initialize LEDs */
    leds_init();

    /* Turn all LEDs off */
    LED1_OFF();
    LED2_OFF();
    LED3_OFF();
    LED4_OFF();


    /* Main loop - blink LEDs in sequence */
    while (1) {
        /* LED1 on */
        LED1_ON();
        delay(100000);
        LED1_OFF();

        /* LED2 on */
        LED2_ON();
        delay(100000);
        LED2_OFF();

        /* LED3 on */
        LED3_ON();
        delay(100000);
        LED3_OFF();

        /* LED4 on */
        LED4_ON();
        delay(100000);
        LED4_OFF();

        /* Increment counter (visible in debugger) */
        blink_count++;
    }

    return 0;
}
