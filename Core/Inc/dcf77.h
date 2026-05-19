#ifndef INC_DCF77_H_
#define INC_DCF77_H_

#include "main.h"   // Pour avoir accès aux fonctions HAL et aux noms des pins
#include <stdio.h>  // Pour le printf


// --- Déclaration des fonctions publiques ---

// Fonction à appeler une seule fois au démarrage
void DCF77_Init(void);

// Fonction à appeler en boucle dans le while(1)
void DCF77_Process(void);
// Ajoute ça tout en bas
uint8_t DCF77_NouvelleHeureDispo(uint8_t *h, uint8_t *m);
void DCF77_Reset_Recherche(void);

#endif /* INC_DCF77_H_ */
