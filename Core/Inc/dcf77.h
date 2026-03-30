#ifndef INC_DCF77_H_
#define INC_DCF77_H_

#include "main.h"   // Pour avoir accès aux fonctions HAL et aux noms des pins
#include <stdio.h>  // Pour le printf

// --- Déclaration des fonctions publiques ---

// Fonction à appeler une seule fois au démarrage
void DCF77_Init(void);

// Fonction à appeler en boucle dans le while(1)
void DCF77_Process(void);

#endif /* INC_DCF77_H_ */
