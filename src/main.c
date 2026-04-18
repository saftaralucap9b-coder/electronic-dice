int main(void) {
    Timer0_Init();
    Dice_Init();

    while (1) {
        Dice_Update();
    }
}