#include "pitches.h"
#include "bsp/bsp.h"
#include "drivers/gpio/gpio.h"
#include "drivers/pwm/pwm.h"
#include "drivers/timer/timer0.h"
#include "drivers/dice/dice.h"
#include "bsp/nano.h"

#include <stdlib.h>

typedef struct {
    gpio_port_t port;
    uint8_t pin;
} dice_pin_t;//structura pini

typedef enum {
    DICE_STATE_IDLE,
    DICE_STATE_BEEP_ON,
    DICE_STATE_BEEP_OFF,
    DICE_STATE_SHOW_RESULT
} dice_state_t;//enumeram starile

typedef struct {
    const int* note;
    const int* durate;
    uint8_t lungime;
} melody_t;

// 1-FAIL (rapid descendent)
static const int m1_n[] = {
    NOTE_C4, 0, NOTE_B3, 0, NOTE_A3
};
static const int m1_d[] = {
    200,80,200,80,400
};

//2-neutru
static const int m2_n[] = {
    NOTE_G4, 0, NOTE_G4
};
static const int m2_d[] = {
    80,40,150
};

// 3-mic progres (urcare + bounce)
static const int m3_n[] = {
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5
};
static const int m3_d[] = {
    100,100,100,250
};

// 4-alerta (trill arcade)
static const int m4_n[] = {
    NOTE_E4, NOTE_E5, NOTE_E4, NOTE_E5, NOTE_G4
};
static const int m4_d[] = {
    60,60,60,60,250
};
// 5-WIN (urcare rapidă energică)
static const int m5_n[] = {
    NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_G5
};
static const int m5_d[] = {
    100,100,100,150,400
};
// 6-JACKPOT (FOARTE animat )
static const int m6_n[] = {
    NOTE_C5, NOTE_G4, NOTE_C5, 0,
    NOTE_E5, NOTE_G5, NOTE_C6
};
static const int m6_d[] = {
    80,80,120,60,
    80,120,300
};
static const melody_t playlist[6] = {
    { m1_n, m1_d, 5 },
    { m2_n, m2_d, 4 },
    { m3_n, m3_d, 6 },
    { m4_n, m4_d, 6 },
    { m5_n, m5_d, 6 },
    { m6_n, m6_d, 9 }
};
static const dice_pin_t led_pins[6] = {
    { D2 },{ D3 },{ D4 },{ D5 },{ D6 },{ D7 }
};

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
            if ((now - last_button_event) >= 200) {
                last_button_event = now;//memorez momentul apasarii
                dice_value = Dice_GenerateRandomValue();//generez v zar
                Dice_Display(dice_value);

                beep_index = 0;

                int note = playlist[dice_value - 1].note[0];
                if (note == 0) Dice_StopBeep();
                else Dice_StartBeep(note);

                state_timestamp = now;
                dice_state = DICE_STATE_BEEP_ON;
            }
        }
        break;

    case DICE_STATE_BEEP_ON:
        if ((now - state_timestamp) >= playlist[dice_value - 1].durate[beep_index]) {
            Dice_StopBeep();
            state_timestamp = now;
            dice_state = DICE_STATE_BEEP_OFF;
        }
        break;

    case DICE_STATE_BEEP_OFF:
        if ((now - state_timestamp) >= 50) { // pauză mai naturală
            beep_index++;

            if (beep_index < playlist[dice_value - 1].lungime) {

                int note = playlist[dice_value - 1].note[beep_index];

                if (note == 0) Dice_StopBeep();
                else Dice_StartBeep(note);

                state_timestamp = now;
                dice_state = DICE_STATE_BEEP_ON;
            } else {
                state_timestamp = now;
                dice_state = DICE_STATE_SHOW_RESULT;
            }
        }
        break;

    case DICE_STATE_SHOW_RESULT:
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

static void Dice_Clear(void) {//functie sting toate LED -urile
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);//pentru fiecare LED, pune pe LOW
    }
}

static void Dice_Display(uint8_t value) {
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Init(led_pins[i].port, led_pins[i].pin, GPIO_OUTPUT);

        if (i < value) {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_HIGH);
        } else {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
        }
    }
}

static void Dice_StartBeep(uint16_t frequency) {//funcția care pornește buzzerul
    PWM_Init(D11, frequency);
    PWM_SetDutyCycle(D11, 35); // mai soft
}

static void Dice_StopBeep(void) {
    PWM_SetDutyCycle(D11, 0);
    PWM_Stop(D11);//sa fiu sigura ca buzzerul chiar se opreste
}

static uint8_t Dice_IsButtonPressed(void) {
    gpio_state_t current_state = GPIO_Read(D12);//starea curentă a butonului de pe pinul D12
    uint8_t pressed = 0;

    if (last_button_state == GPIO_HIGH && current_state == GPIO_LOW) {
        pressed = 1;
    }

    last_button_state = current_state;//starea anterioasa este stare curenta
    return pressed;
}

static uint8_t Dice_GenerateRandomValue(void) {
    srand((unsigned int)(Millis() ^ entropy_counter));
    return (uint8_t)((rand() % 6) + 1);
}