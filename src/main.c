#include "drivers/timer/timer0.h"
#include "drivers/dice/dice.h"
#include "bsp/pitches.h"

int main(void) {
    Timer0_Init();
    Dice_Init();

    while (1) {
        Dice_Update();
    }

    return 0;
}