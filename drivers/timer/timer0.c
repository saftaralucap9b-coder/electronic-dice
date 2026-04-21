#include "timer0.h"

// System tick counter, increments every 1ms
static volatile uint32_t system_millis = 0;

void Timer0_Init(void) {
    // Set CTC mode
    TCCR0A |= (1 << WGM01);

    // Set Compare Match value for 1ms
    OCR0A = 249;

    // Enable Compare Match A Interrupt
    TIMSK0 |= (1 << OCIE0A);

    // Set Prescaler to 64
    TCCR0B |= (1 << CS01) | (1 << CS00);

    // Enable global interrupts
    sei();
}

ISR(TIMER0_COMPA_vect) {
    system_millis++;
}

uint32_t Millis(void) {
    uint32_t m;

    // Atomic read
    uint8_t oldSREG = SREG;
    cli();
    m = system_millis;
    SREG = oldSREG;

    return m;
}