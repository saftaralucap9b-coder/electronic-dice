#ifndef TIMER0_H
#define TIMER0_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void Timer0_Init(void);
uint32_t Millis(void);

#endif // TIMER0_H