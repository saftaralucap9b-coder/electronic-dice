#include "gpio.h"
#include "utils/utils.h"
//Configurează
void GPIO_Init(gpio_port_t port, uint8_t pin, gpio_dir_t dir) {
    switch (port) {
        case GPIO_PORTB:
            if (dir == GPIO_OUTPUT) {
                SET_BIT(DDRB, pin);
            } else {
                CLEAR_BIT(DDRB, pin);
            }
            break;

        case GPIO_PORTC:
            if (dir == GPIO_OUTPUT) {
                SET_BIT(DDRC, pin);
            } else {
                CLEAR_BIT(DDRC, pin);
            }
            break;

        case GPIO_PORTD:
            if (dir == GPIO_OUTPUT) {
                SET_BIT(DDRD, pin);
            } else {
                CLEAR_BIT(DDRD, pin);
            }
            break;
    }
}
//Scrie pe pin
void GPIO_Write(gpio_port_t port, uint8_t pin, gpio_state_t state) {
    switch (port) {
        case GPIO_PORTB:
            WRITE_BIT(PORTB, pin, state);
            break;

        case GPIO_PORTC:
            WRITE_BIT(PORTC, pin, state);
            break;

        case GPIO_PORTD:
            WRITE_BIT(PORTD, pin, state);
            break;
    }
}
//Citește starea pin
gpio_state_t GPIO_Read(gpio_port_t port, uint8_t pin) {
    switch (port) {
        case GPIO_PORTB:
            return CHECK_BIT(PINB, pin);

        case GPIO_PORTC:
            return CHECK_BIT(PINC, pin);

        case GPIO_PORTD:
            return CHECK_BIT(PIND, pin);
    }

    return GPIO_LOW;
}
//Inversează starea pinului
void GPIO_Toggle(gpio_port_t port, uint8_t pin) {
    switch (port) {
        case GPIO_PORTB:
            TOGGLE_BIT(PORTB, pin);
            break;

        case GPIO_PORTC:
            TOGGLE_BIT(PORTC, pin);
            break;

        case GPIO_PORTD:
            TOGGLE_BIT(PORTD, pin);
            break;
    }
}