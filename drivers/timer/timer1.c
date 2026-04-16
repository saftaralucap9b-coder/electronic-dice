#include "timer1.h"

void Timer1_Init(uint32_t frequency) {
    // Clear control registers
    TCCR1A = 0;
    TCCR1B = 0;

    // Set Fast PWM mode with ICR1 as TOP (Mode 14)
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);

    // Calculate TOP value based on frequency
    uint32_t top = (16000000UL / (8UL * frequency)) - 1;

    // Set TOP value
    ICR1 = (uint16_t)top;

    // Set prescaler to 8
    TCCR1B |= (1 << CS11);

    // Enable non-inverting mode on OC1A and OC1B
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);
}