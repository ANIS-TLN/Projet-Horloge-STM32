#include "buzzer.h"

// Allume le buzzer (Low Level Trigger = on envoie RESET)
void Buzzer_On(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
}

// Éteint le buzzer (Low Level Trigger = on envoie SET)
void Buzzer_Off(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}

// Fait un bip d'une certaine durée puis s'éteint
void Buzzer_Beep(uint16_t duree_ms) {
    Buzzer_On();
    HAL_Delay(duree_ms);
    Buzzer_Off();
}

void Buzzer_Sonnerie_Douce(void) {
    for(int i=0; i<3; i++) {
        Buzzer_On();
        HAL_Delay(40);   // Bip très court
        Buzzer_Off();
        HAL_Delay(60);   // Mini silence
        Buzzer_On();
        HAL_Delay(40);   // Bip très court
        Buzzer_Off();
        HAL_Delay(500);  // Pause avant le prochain double-bip
    }
}
