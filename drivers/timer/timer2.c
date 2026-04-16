#include "timer2.h"

void Timer2_Init(uint32_t frequency) {
    // Clear control registers
    TCCR2A = 0;
    TCCR2B = 0;

    // Set Fast PWM mode (Mode 3)
    TCCR2A |= (1 << WGM20) | (1 << WGM21);

    // Set prescaler (default 64 for ~1kHz)
    TCCR2B |= (1 << CS22);

    // Enable non-inverting mode on OC2A and OC2B
    TCCR2A |= (1 << COM2A1) | (1 << COM2B1);
}