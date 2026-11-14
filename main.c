#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include "pico/stdio_usb.h"

#include "hardware/uart.h"
#include "hardware/i2c.h"

volatile bool alarm_fired = false;
volatile bool repeating_timer_fired = false;
volatile bool heartbeat_timer_fired = false;
volatile bool status_timer_fired = false;
volatile bool button_irq_fired = false;
volatile bool button_released_irq_fired = false;

void button_interrupt_handler(uint gpio, uint32_t events)
{
    if (gpio == 14)
    {
        if (events & GPIO_IRQ_EDGE_FALL)
        {
            button_irq_fired = true;
        }

        if (events & GPIO_IRQ_EDGE_RISE)
        {
            button_released_irq_fired = true;
        }
    }
}

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

    static uint32_t heartbeat_count = 0;

    heartbeat_count++;

    heartbeat_timer_fired = true;

    if (heartbeat_count >= 5)
    {
        return false;
    }

    return true;
}

bool status_timer_callback(repeating_timer_t *rt)
{
    (void)rt;

    status_timer_fired = true;

    return true;
}

void scan_i2c_bus(void)
{
    printf("\nScanning I2C bus...\n");

    uint8_t probe_byte = 0x00;
    bool device_found = false;

    for (uint8_t address = 0x08; address <= 0x77; address++)
    {
        int result = i2c_write_timeout_us(
            i2c0,
            address,
            &probe_byte,
            1,
            false,
            1000
        );

        if (result == 1)
        {
            printf("I2C device found at address: 0x%02X\n", address);
            device_found = true;
        }
    }

    if (!device_found)
    {
        printf("No I2C devices found.\n");
    }

    printf("I2C scan complete.\n\n");
}

static const uint8_t simulated_sensor_registers[] =
{
    0x42,  // Register 0x00: Device ID
    0x01,  // Register 0x01: Configuration
    0xFF,  // Register 0x02: Temperature MSB
    0xFB,  // Register 0x03: Temperature LSB
    0x00,  // Register 0x04: Status
    0x12,  // Register 0x05: Measurement byte 1
    0x34,  // Register 0x06: Measurement byte 2
    0x56   // Register 0x07: Measurement byte 3
};

uint8_t simulated_sensor_read_register(uint8_t register_address)
{
    if (register_address >= sizeof(simulated_sensor_registers))
    {
        printf("Invalid register address: 0x%02X\n", register_address);
        return 0;
    }

    return simulated_sensor_registers[register_address];
}

uint32_t simulated_sensor_read_24bit_measurement(void)
{
    uint8_t byte1 = simulated_sensor_read_register(0x05);
    uint8_t byte2 = simulated_sensor_read_register(0x06);
    uint8_t byte3 = simulated_sensor_read_register(0x07);

    uint32_t measurement =
        ((uint32_t)byte1 << 16) |
        ((uint32_t)byte2 << 8)  |
        byte3;

    return measurement;
}

bool simulated_sensor_data_ready(void)
{
    uint8_t status = simulated_sensor_read_register(0x04);

    return (status & 0x01) != 0;
}

bool simulated_sensor_read_measurement(uint32_t *measurement)
{
    uint32_t wait_time_ms = 0;

    while (!simulated_sensor_data_ready() && wait_time_ms < 5000)
    {
        printf("Sensor data is not ready. Waiting...\n");

        sleep_ms(1000);

        wait_time_ms += 1000;
    }

    if (!simulated_sensor_data_ready())
    {
        printf("Sensor timeout! No measurement received.\n");
        return false;
    }

    *measurement = simulated_sensor_read_24bit_measurement();

    return true;
}

uint16_t simulated_sensor_read_temperature(void)
{
    uint8_t msb = simulated_sensor_read_register(0x02);
    uint8_t lsb = simulated_sensor_read_register(0x03);

    uint16_t raw_temperature = ((uint16_t)msb << 8) | lsb;

    return raw_temperature;
}

int16_t simulated_sensor_read_signed_temperature(void)
{
    uint8_t msb = simulated_sensor_read_register(0x02);
    uint8_t lsb = simulated_sensor_read_register(0x03);

    uint16_t raw_value = ((uint16_t)msb << 8) | lsb;

    return (int16_t)raw_value;
}

float simulated_sensor_convert_temperature(int16_t raw_value)
{
    return (float)raw_value / 10.0f;
}

int main(void)
{
    stdio_init_all();

    while (!stdio_usb_connected())
    {
        sleep_ms(10);
    }

    uart_init(uart0, 9600);
    i2c_init(i2c0, 100000);

    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    gpio_set_function(4, GPIO_FUNC_I2C);
    gpio_set_function(5, GPIO_FUNC_I2C);

    gpio_pull_up(4);
    gpio_pull_up(5);

    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);

    uart_puts(uart0, "Hello UART!\n");

    printf("UART transmission attempted.\n");
    stdio_flush();

    if (cyw43_arch_init())
    {
        printf("CYW43 initialization failed!\n");
        return 1;
    }

    printf("USB CDC is working!\n");
    stdio_flush();

    printf("Pico 2 W LED control started!\n");

    uint8_t device_id = simulated_sensor_read_register(0x00);

    printf("Simulated sensor device ID: 0x%02X\n", device_id);

    uint16_t raw_temperature = simulated_sensor_read_temperature();

    if (simulated_sensor_data_ready())
    {
        printf("New measurement available!\n");
    }
    else
    {
        printf("No new measurement available.\n");
    }

    printf("Simulated raw temperature: %u\n",
           (unsigned int)raw_temperature);

    printf("Simulated raw temperature (hex): 0x%04X\n",
           (unsigned int)raw_temperature);

    int16_t signed_temperature = simulated_sensor_read_signed_temperature();

    printf("Simulated signed temperature: %d C\n",
           (int)signed_temperature);

    float temperature_celsius = simulated_sensor_convert_temperature(signed_temperature);

    printf("Simulated temperature: %.1f C\n", (double)temperature_celsius);

    uint32_t wait_time_ms = 0;

    uint32_t measurement;

    if (simulated_sensor_read_measurement(&measurement))
    {
        printf("New measurement available!\n");

        printf("Simulated 24-bit measurement: 0x%06X\n",
               (unsigned int)measurement);
    }

    scan_i2c_bus();

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

    gpio_set_irq_enabled_with_callback(
        14,
        GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE,
        true,
        &button_interrupt_handler
    );

    uint64_t timer_start_time = time_us_64();
    bool timer_cancelled = false;

    while (true)
    {
        if (uart_is_readable(uart0))
        {
            char received_char = uart_getc(uart0);

            printf("%c", received_char);
        }

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

        if (button_irq_fired)
        {
            button_irq_fired = false;

            gpio_put(15, 1);

            printf("Button pressed - LED ON\n");
        }

        if (button_released_irq_fired)
        {
            button_released_irq_fired = false;

            gpio_put(15, 0);

            printf("Button released - LED OFF\n");
        }
    }

    cyw43_arch_deinit();

    return 0;
}
