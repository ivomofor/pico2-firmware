#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"

volatile bool alarm_fired = false;
volatile bool repeating_timer_fired = false;
volatile bool heartbeat_timer_fired = false;
volatile bool status_timer_fired = false;

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;

    alarm_fired = true;

    return 0;
}

bool repeating_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    repeating_timer_fired = true;

    return true;
}

bool heartbeat_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    heartbeat_timer_fired = true;

    return true;
}

bool status_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    status_timer_fired = true;

    return true;
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

    repeating_timer_t repeating_timer;
    repeating_timer_t heartbeat_timer;
    repeating_timer_t status_timer;

    if (!add_repeating_timer_ms(2000, repeating_timer_callback, NULL, &repeating_timer))
    {
        printf("Failed to start repeating timer!\n");
        return 1;
    }

    if (!add_repeating_timer_ms(1000, heartbeat_timer_callback, NULL, &heartbeat_timer))
    {
        printf("Failed to start heartbeat timer!\n");
        return 1;
    }

    if (!add_repeating_timer_ms(3000, status_timer_callback, NULL, &status_timer))
    {
        printf("Failed to start status timer!\n");
        return 1;
    }

    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0);

    gpio_init(14);
    gpio_set_dir(14, GPIO_IN);
    gpio_pull_up(14);

    uint64_t previous_time = time_us_64();
    uint64_t timer_start_time = time_us_64();
    bool timer_cancelled = false;

    while (true)
    {
        if (alarm_fired)
        {
            alarm_fired = false;

            printf("Hardware timer alarm fired!\n");
        }
        
        if (repeating_timer_fired)
        {
            repeating_timer_fired = false;

            printf("Repeating timer fired!\n");
        }

        if (heartbeat_timer_fired)
        {
            heartbeat_timer_fired = false;

            printf("Heartbeat timer fired!\n");
        }

        if (status_timer_fired)
        {
            status_timer_fired = false;

            printf("Status timer fired!\n");
        }

        uint64_t current_time = time_us_64();

        if (!timer_cancelled && current_time - timer_start_time >= 10000000)
        {
            if (cancel_repeating_timer(&repeating_timer))
            {
                printf("Repeating timer cancelled!\n");
            }

            timer_cancelled = true;
        }

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