#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

int main(void)
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("CYW43 initialization failed!\n");
        return 1;
    }

    printf("Pico 2 W LED control started!\n");

    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0);

    gpio_init(14);
    gpio_set_dir(14, GPIO_IN);
    gpio_pull_up(14);

    while (true)
    {
        bool button_state = gpio_get(14);

        if (button_state == false)
        {
            gpio_put(15, 1);
            printf("Button pressed - LED ON\n");
        }
        else
        {
            gpio_put(15, 0);
            printf("Button released - LED OFF\n");
        }

        sleep_ms(200);
    }

    cyw43_arch_deinit();

    return 0;
}