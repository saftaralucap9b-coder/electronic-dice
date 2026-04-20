#ifndef TIMER1_H
#define TIMER1_H

#include <stdint.h>
#include <avr/io.h>

void Timer1_FastPWM_Init(uint16_t prescaler, uint16_t top);
void Timer1_SetDutyCycleA(uint16_t duty);
void Timer1_SetDutyCycleB(uint16_t duty);
void Timer1_Stop(void);

#endif // TIMER1_H