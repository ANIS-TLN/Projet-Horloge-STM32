#include "dcf77.h"

// --- Variables privées du module ---
static GPIO_PinState etat_precedent = GPIO_PIN_RESET;
static uint32_t temps_front_montant = 0;
static uint32_t temps_front_descendant = 0;

static uint8_t tableau_bits[60] = {0};
static uint8_t index_bit = 0;

typedef enum { CHERCHE_SYNCHRO, LECTURE_DONNEES, DECODAGE } Etat_DCF;
static Etat_DCF etat_actuel_dcf = CHERCHE_SYNCHRO;

// --- Initialisation ---
void DCF77_Init(void) {
    printf("\r\n--- DEMARRAGE SYSTEME DCF77 ---\r\n");
    printf("Recherche du signal de synchronisation (Attente max 1 minute)...\r\n");
}

// --- Le cerveau du décodage ---
void DCF77_Process(void) {
    GPIO_PinState etat_actuel = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

    if (etat_actuel != etat_precedent) {

        // 1. FRONT MONTANT
        if (etat_actuel == GPIO_PIN_SET) {
            temps_front_montant = HAL_GetTick();
            uint32_t duree_silence = temps_front_montant - temps_front_descendant;

            if (duree_silence > 1500) {
                printf("\r\n>>> SYNCHRONISATION TROUVEE ! <<<\r\n");
                etat_actuel_dcf = LECTURE_DONNEES;
                index_bit = 0;
            }
        }
        // 2. FRONT DESCENDANT
        else {
            temps_front_descendant = HAL_GetTick();
            uint32_t duree_bip = temps_front_descendant - temps_front_montant;

            if (etat_actuel_dcf == LECTURE_DONNEES) {
                uint8_t bit_recu = 2;

                if (duree_bip > 70 && duree_bip < 130) {
                    bit_recu = 0;
                } else if (duree_bip > 170 && duree_bip < 230) {
                    bit_recu = 1;
                }

                if (bit_recu != 2) {
                    tableau_bits[index_bit] = bit_recu;
                    // printf("Bit %02d : %d\r\n", index_bit, bit_recu); // Décommenter pour débug
                    index_bit++;

                    if (index_bit == 59) {
                        etat_actuel_dcf = DECODAGE;
                    }
                } else {
                    printf("Erreur (bruit parasite), retour recherche synchro...\r\n");
                    etat_actuel_dcf = CHERCHE_SYNCHRO;
                }
            }
        }
        etat_precedent = etat_actuel;
    }

    // 3. DECODAGE
    if (etat_actuel_dcf == DECODAGE) {
        // --- HEURE ---
        uint8_t min_unite = tableau_bits[21] + (tableau_bits[22]*2) + (tableau_bits[23]*4) + (tableau_bits[24]*8);
        uint8_t min_dizaine = tableau_bits[25] + (tableau_bits[26]*2) + (tableau_bits[27]*4);
        uint8_t minutes = (min_dizaine * 10) + min_unite;

        uint8_t h_unite = tableau_bits[29] + (tableau_bits[30]*2) + (tableau_bits[31]*4) + (tableau_bits[32]*8);
        uint8_t h_dizaine = tableau_bits[33] + (tableau_bits[34]*2);
        uint8_t heures = (h_dizaine * 10) + h_unite;

        // --- DATE ---
        uint8_t jour_unite = tableau_bits[36] + (tableau_bits[37]*2) + (tableau_bits[38]*4) + (tableau_bits[39]*8);
        uint8_t jour_dizaine = tableau_bits[40] + (tableau_bits[41]*2);
        uint8_t jour = (jour_dizaine * 10) + jour_unite;

        uint8_t mois_unite = tableau_bits[45] + (tableau_bits[46]*2) + (tableau_bits[47]*4) + (tableau_bits[48]*8);
        uint8_t mois_dizaine = tableau_bits[49];
        uint8_t mois = (mois_dizaine * 10) + mois_unite;

        uint8_t annee_unite = tableau_bits[50] + (tableau_bits[51]*2) + (tableau_bits[52]*4) + (tableau_bits[53]*8);
        uint8_t annee_dizaine = tableau_bits[54] + (tableau_bits[55]*2) + (tableau_bits[56]*4) + (tableau_bits[57]*8);
        uint8_t annee = (annee_dizaine * 10) + annee_unite;

        // --- AFFICHAGE ---
        printf("\r\n=================================\r\n");
        printf(" HEURE  : %02d:%02d\r\n", heures, minutes);
        printf(" DATE   : %02d/%02d/20%02d\r\n", jour, mois, annee);
        printf("=================================\r\n\r\n");
        printf("hello");

        etat_actuel_dcf = CHERCHE_SYNCHRO;
    }
}
