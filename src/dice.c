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
void Dice_Init(void) {
    // Inițializare LED-uri
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Init(led_pins[i].port, led_pins[i].pin, GPIO_OUTPUT);
        GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
    }

    // Inițializare buton cu pull-up intern
    GPIO_Init(D12, GPIO_INPUT);
    GPIO_Write(D12, GPIO_HIGH);
// Inițializare buzzer
    PWM_Init(D11, NOTE_C4);
    PWM_SetDutyCycle(D11, 0);
    PWM_Stop(D11);

    dice_state = DICE_STATE_IDLE;
    dice_value = 0;
    beep_index = 0;
    last_button_state = GPIO_HIGH;
    state_timestamp = 0;
    last_button_event = 0;
    entropy_counter = 0;
}
void Dice_Update(void) {
    uint32_t now = Millis();
    entropy_counter++;

    switch (dice_state) {
  case DICE_STATE_IDLE:
if (Dice_IsButtonPressed()) {
// debounce simplu
if ((now - last_button_event) >= 200) {
 last_button_event = now;
 dice_value = Dice_GenerateRandomValue();
 Dice_Display(dice_value);

 beep_index = 0;
Dice_StartBeep(NOTE_C4);
state_timestamp = now;
 dice_state = DICE_STATE_BEEP_ON;
 }
 }
  break;
    case DICE_STATE_BEEP_ON:
// 200 ms ON, similar cu tone(..., 200)
  if ((now - state_timestamp) >= 200) {
  Dice_StopBeep();
  state_timestamp = now;
dice_state = DICE_STATE_BEEP_OFF;}
break;
        case DICE_STATE_BEEP_OFF:
  // 100 ms pauză, astfel totalul devine ~300 ms per sunet
 if ((now - state_timestamp) >= 100) {
   beep_index++;

if (beep_index < dice_value) {
 uint16_t next_note = NOTE_C4 + (beep_index * 50);
Dice_StartBeep(next_note);
state_timestamp = now;
 dice_state = DICE_STATE_BEEP_ON;
 } else {
 state_timestamp = now;
 dice_state = DICE_STATE_SHOW_RESULT;
  }
 } break;
 case DICE_STATE_SHOW_RESULT:
 // 500 ms afișare rezultat, apoi stingere LED-uri
if ((now - state_timestamp) >= 500) {
 Dice_Clear();
dice_state = DICE_STATE_IDLE;
  }
 break;