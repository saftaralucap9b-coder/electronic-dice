#include "drivers/timer/timer0.h"
#include "dice.h"

int main(void) {
    Timer0_Init();
    Dice_Init();

    while (1) {
        Dice_Update();
    }

    return 0;
}