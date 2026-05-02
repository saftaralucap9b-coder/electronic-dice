
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
typedef enum {DICE_STATE_IDLE,
    DICE_STATE_BEEP_ON,
    DICE_STATE_BEEP_OFF,
    DICE_STATE_SHOW_RESULT
} dice_state_t;//enumeram starile
static const dice_pin_t led_pins[6] = {{ D2 },{ D3 },{ D4 },{ D5 },{ D6 },{ D7 }};
// Structură pentru a gestiona melodiile
typedef struct {
    const int* note;
    const int* durate;
    uint8_t lungime;
} melody_t;

// 1: "Aw, snap!" (Efect de eșec, scurt și grav)
static const int m1_notes[] = { NOTE_G2, NOTE_C2 };
static const int m1_dur[]   = { 150, 300 };

// 2: "Double Tap" (Stil militar/robot)
static const int m2_notes[] = { NOTE_E4, NOTE_E4 };
static const int m2_dur[]   = { 80, 80 };

// 3: "Level Up" (Urcare rapidă, veselă)
static const int m3_notes[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5 };
static const int m3_dur[]   = { 50, 50, 50, 150 };

// 4: "Retro Slide" (Cădere stil Mario)
static const int m4_notes[] = { NOTE_C5, NOTE_G4, NOTE_E4, NOTE_C4 };
static const int m4_dur[]   = { 80, 80, 80, 200 };

// 5: "Magic Spell" (Alternanță rapidă de frecvențe înalte)
static const int m5_notes[] = { NOTE_C5, NOTE_G5, NOTE_E5, NOTE_G5, NOTE_B5 };
static const int m5_dur[]   = { 60, 60, 60, 60, 150 };

// 6: "JACKPOT" (Fanfară de victorie)
static const int m6_notes[] = { NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6 };
static const int m6_dur[]   = { 80, 80, 80, 80, 100, 100, 400 };

static const melody_t melodii[6] = {
    { m1_notes, m1_dur, 2 },
    { m2_notes, m2_dur, 2 },
    { m3_notes, m3_dur, 4 },
    { m4_notes, m4_dur, 4 },
    { m5_notes, m5_dur, 5 },
    { m6_notes, m6_dur, 7 }
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
        if ((now - last_button_event) >= 200) {
            last_button_event = now;
            dice_value = Dice_GenerateRandomValue(); // Valoare între 1 și 6
            Dice_Display(dice_value);

            beep_index = 0; // Indexul notei curente din melodie
            // Pornim prima notă a melodiei specifice
            Dice_StartBeep(melodii[dice_value - 1].note[0]);
            state_timestamp = now;
            dice_state = DICE_STATE_BEEP_ON;
        }
    }
    break;
    case DICE_STATE_BEEP_ON:
    // Folosim durata specifică notei curente din melodie
    if ((now - state_timestamp) >= melodii[dice_value - 1].durate[beep_index]) {
        Dice_StopBeep();
        state_timestamp = now;
        dice_state = DICE_STATE_BEEP_OFF;
    }
    break;
       case DICE_STATE_BEEP_OFF:
    // O pauză de doar 20-30ms face ca notele să pară legate, dar distincte
    if ((now - state_timestamp) >= 30) { 
        beep_index++;
        if (beep_index < melodii[dice_value - 1].lungime) {
            Dice_StartBeep(melodii[dice_value - 1].note[beep_index]);
            state_timestamp = now;
            dice_state = DICE_STATE_BEEP_ON;
        } else {
            // Melodia s-a terminat, oprim PWM-ul de tot acum
            PWM_Stop(D11);
            state_timestamp = now;
            dice_state = DICE_STATE_SHOW_RESULT;
        }
    }
    break;
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
static void Dice_Clear(void) {//functie sting toate LED -urile
    for (uint8_t i = 0; i < 6; i++) {
        GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);//pentru fiecare LED, pune pe LOW
    }
}
static void Dice_Display(uint8_t value) {//afisez pe led val.zar 
    for (uint8_t i = 0; i < 6; i++) {
        if (i < value) {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_HIGH);
        } else {
            GPIO_Write(led_pins[i].port, led_pins[i].pin, GPIO_LOW);
        }//aprind led uri in functie de valoare,restul le las stinse
    }
}
static void Dice_StartBeep(uint16_t frequency) {
    PWM_Init(D11, frequency); 
    PWM_SetDutyCycle(D11, 127); // 50% duty cycle pentru un sunet clar de tip "square wave"
}
static void Dice_StopBeep(void) {
    PWM_SetDutyCycle(D11, 0);
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
    return (uint8_t)((rand() % 6) + 1); //setez seed-ul cu o valoare bazată pe timpul curent (Millis()) și un contor intern (entropy_counter) pentru a obține rezultate diferite la fiecare execuție
}