
#include "pitches.h"
#include "bsp/bsp.h"
#include "drivers/gpio/gpio.h"
#include "drivers/pwm/pwm.h"
#include "drivers/timer/timer0.h"
#include "drivers/dice/dice.h"

#include <stdlib.h>

typedef struct {
gpio_port_t port;
uint8_t pin;
} dice_pin_t;//structura pini
typedef enum {DICE_STATE_IDLE,
    DICE_STATE_BEEP_ON,
    DICE_STATE_BEEP_OFF,
    DICE_STATE_SHOW_RESULT
} dice_state_t;//enumeram starile
static const dice_pin_t led_pins[6] = {{ D2 },{ D3 },{ D4 },{ D5 },{ D6 },{ D7 }};

static dice_state_t dice_state = DICE_STATE_IDLE;//starea curenta -asteptam apasare buton

static uint8_t dice_value = 0;//valoare zar
static uint8_t beep_index = 0;//de cate ori face beep
static gpio_state_t last_button_state = GPIO_HIGH;//starea butonului se retine

static uint32_t state_timestamp = 0;//salvez momentul cat sta afisat rezultatul
static uint32_t last_button_event = 0;//ultima apasare de buton
static uint32_t entropy_counter = 0;//contor care creste mereu si ajuta la random

static void Dice_Clear(void);//stingem led
static void Dice_Display(uint8_t value);//afisam valoarea zar
static void Dice_StartBeep(uint16_t frequency);//pornim buzzer la o anume frecventa
static void Dice_StopBeep(void);
static uint8_t Dice_IsButtonPressed(void);
static uint8_t Dice_GenerateRandomValue(void);
void Dice_Init(void) {
    // Inițializare LED-uri
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Init(led_pins[i].port, led_pins[i].pin, GPIO_OUTPUT);//configuram led ca iesire
        GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
    }

    // Inițializare buton cu pull-up intern
    GPIO_Init(D12, GPIO_INPUT);//D12 intrare
    GPIO_Write(D12, GPIO_HIGH);
// Inițializare buzzer
    PWM_Init(D11, NOTE_C4);//D11 la frecvența NOTE_C4
    PWM_SetDutyCycle(D11, 0);//fara sunet
    PWM_Stop(D11);//oprit

    dice_state = DICE_STATE_IDLE;//asteptare
    dice_value = 0;//reset
    beep_index = 0;//reset contor beep
    last_button_state = GPIO_HIGH;//presupun ca nu e apasat
    state_timestamp = 0;
    last_button_event = 0;
    entropy_counter = 0;//reset contor random
}
void Dice_Update(void) {//functie pe care o apelez continuu in main
    uint32_t now = Millis();//timpul curent
    entropy_counter++;

    switch (dice_state) {
  case DICE_STATE_IDLE:
if (Dice_IsButtonPressed()) {
// debounce simplu
if ((now - last_button_event) >= 200) {//daca butonul e apasat verific daca au trecut 200ms de la u apasare
 last_button_event = now;//memorez momentul apasarii
 dice_value = Dice_GenerateRandomValue();//generez v zar
 Dice_Display(dice_value);

 beep_index = 0;
Dice_StartBeep(NOTE_C4);//primul beep la frecvența C4
state_timestamp = now;
 dice_state = DICE_STATE_BEEP_ON;
 }
 }
  break;
    case DICE_STATE_BEEP_ON:
    //vreau ca beep-ul să țină 200 ms
  if ((now - state_timestamp) >= 200) {//daca au trecut
  Dice_StopBeep();
  state_timestamp = now;
dice_state = DICE_STATE_BEEP_OFF;}
break;
        case DICE_STATE_BEEP_OFF://pauza aia dintre sunete
  // 100 ms pauză totalul devine 300 ms per sunet
 if ((now - state_timestamp) >= 100) {
   beep_index++;

if (beep_index < dice_value) {
 uint16_t next_note = NOTE_C4 + (beep_index * 50);//calculez frecventa urmatorului sunet
Dice_StartBeep(next_note);
state_timestamp = now;
 dice_state = DICE_STATE_BEEP_ON;
 } else {
 state_timestamp = now;
 dice_state = DICE_STATE_SHOW_RESULT;
  }
 } break;
 case DICE_STATE_SHOW_RESULT:
 // 500 ms afișare rezultat apoi sting LED-uri
if ((now - state_timestamp) >= 500) {
 Dice_Clear();
dice_state = DICE_STATE_IDLE;
  }
 break;
 default:
     dice_state = DICE_STATE_IDLE;//daca nu mai merge resetez si pun iar in starea sigura
      break;
    }
}
static void Dice_Clear(void) {
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
    }
}
static void Dice_Display(uint8_t value) {
    for (uint8_t i = 0; i < 6; i++) {
        if (i < value) {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_HIGH);
        } else {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
        }
    }
}
static void Dice_StartBeep(uint16_t frequency) {
    PWM_Init(D11, frequency);
    PWM_SetDutyCycle(D11, 128); // ~50% duty cycle
}
static void Dice_StopBeep(void) {
    PWM_SetDutyCycle(D11, 0);
    PWM_Stop(D11);
}
static uint8_t Dice_IsButtonPressed(void) {
    gpio_state_t current_state = GPIO_Read(D12);
    uint8_t pressed = 0;

    if (last_button_state == GPIO_HIGH && current_state == GPIO_LOW) {
        pressed = 1;
    }

    last_button_state = current_state;
    return pressed;
}
static uint8_t Dice_GenerateRandomValue(void) {
    // Seed bazat pe timpul de reacție al utilizatorului + număr de iterații
    srand((unsigned int)(Millis() ^ entropy_counter));
    return (uint8_t)((rand() % 6) + 1);
}