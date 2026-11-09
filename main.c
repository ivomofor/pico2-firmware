#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"

volatile bool alarm_fired = false;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;

    alarm_fired = true;

    return 0;
}

int main(void)
{
    stdio_init_all();

    if (cyw43_arch_init())
    {
        printf("CYW43 initialization failed!\n");
        return 1;
    }

    printf("Pico 2 W LED control started!\n");
    add_alarm_in_ms(2000, alarm_callback, NULL, false);

    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0);

    gpio_init(14);
    gpio_set_dir(14, GPIO_IN);
    gpio_pull_up(14);

    uint64_t previous_time = time_us_64();

    while (true)
    {
        if (alarm_fired)
        {
            alarm_fired = false;

            printf("Hardware timer alarm fired!\n");
        }

        uint64_t current_time = time_us_64();

        if (current_time - previous_time >= 200000)
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

            previous_time = current_time;
        }
    }

    cyw43_arch_deinit();

    return 0;
}