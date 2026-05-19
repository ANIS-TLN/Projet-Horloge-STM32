#ifndef INC_MAX7219_H_
#define INC_MAX7219_H_

#include "main.h"

// Le nombre de matrices connectées en série (Ton ami en a mis 5)
#define NUM_MATRICES 5

// --- Déclaration des fonctions ---
void MAX7219_Init(void);
void MAX7219_SetBrightness(uint8_t brightness); // De 0 (très faible) à 15 (très fort)
void MAX7219_Clear(void);

// Affiche une heure au format "HH:MM"
void MAX7219_DisplayTime(uint8_t hours, uint8_t minutes);
void MAX7219_DisplayZeros(void);
void MAX7219_TestPixel(uint8_t matrice, uint8_t colonne, uint8_t ligne);

#endif /* INC_MAX7219_H_ */
