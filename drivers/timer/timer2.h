#ifndef TIMER2_H
#define TIMER2_H

#include <stdint.h>
#include <avr/io.h>

void Timer2_Init(uint32_t frequency);void Timer2_FastPWM_Init(uint16_t prescaler);
void Timer2_SetDutyCycleA(uint8_t duty);
void Timer2_SetDutyCycleB(uint8_t duty);
void Timer2_Stop(void);

#endif // TIMER2_H