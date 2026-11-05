#include <stdio.h>
#include "pico/stdlib.h"
int main(void)
{
    stdio_init_all();
    while (true)
    {
        printf("Hello from Raspberry Pi Pico 2 W!\n");
        sleep_ms(1000);
    }

    return 0;
}