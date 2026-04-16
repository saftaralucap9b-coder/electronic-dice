#include "dice.h"
#include "pitches.h"
#include "bsp/bsp.h"
#include "drivers/gpio/gpio.h"
#include "drivers/pwm/pwm.h"
#include "drivers/timer/timer0.h"

#include <stdlib.h>

typedef struct {
    gpio_port_t port;
    uint8_t pin;
} dice_pin_t;