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
typedef enum {DICE_STATE_IDLE,
    DICE_STATE_BEEP_ON,
    DICE_STATE_BEEP_OFF,
    DICE_STATE_SHOW_RESULT
} dice_state_t;
static const dice_pin_t led_pins[6] = {{ D2 },{ D3 },{ D4 },{ D5 },{ D6 },{ D7 }};

static dice_state_t dice_state = DICE_STATE_IDLE;

static uint8_t dice_value = 0;
static uint8_t beep_index = 0;
static gpio_state_t last_button_state = GPIO_HIGH;

static uint32_t state_timestamp = 0;
static uint32_t last_button_event = 0;
static uint32_t entropy_counter = 0;

static void Dice_Clear(void);
static void Dice_Display(uint8_t value);
static void Dice_StartBeep(uint16_t frequency);
static void Dice_StopBeep(void);
static uint8_t Dice_IsButtonPressed(void);
static uint8_t Dice_GenerateRandomValue(void);