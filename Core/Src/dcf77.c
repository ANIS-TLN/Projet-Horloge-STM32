#include "dcf77.h"
#include "lcd_i2c.h"  // Pour contrôler l'écran
#include "buzzer.h"   // Pour faire des bips

// --- Variables privées du module ---
static GPIO_PinState etat_precedent = GPIO_PIN_RESET;
static uint32_t temps_front_montant = 0;
static uint32_t temps_front_descendant = 0;

static uint8_t tableau_bits[60] = {0};
static uint8_t index_bit = 0;

typedef enum { CHERCHE_SYNCHRO, LECTURE_DONNEES, DECODAGE } Etat_DCF;
static Etat_DCF etat_actuel_dcf = CHERCHE_SYNCHRO;
static uint8_t dcf_heures = 0;
static uint8_t dcf_minutes = 0;
static uint8_t flag_nouvelle_heure = 0;


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
                lcd_clear();
                lcd_put_cur(0, 0); lcd_send_string("SYNCHRO OK !");
                lcd_put_cur(1, 0); lcd_send_string("Dechiffrement...");

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
                    // --- Ligne à rajouter pour voir les 0 et les 1 sur le PC ! ---
                                        printf("Bit %02d : %d\r\n", index_bit, bit_recu);
                    // ---> MAGIE : Un petit bip à chaque bit reçu ! <---
                    Buzzer_On();
                    HAL_Delay(30);
                    Buzzer_Off();

                    index_bit++;
                    if (index_bit == 59) {
                        etat_actuel_dcf = DECODAGE;
                    }
                } else {
                    printf("Erreur (bruit parasite), retour recherche synchro...\r\n");
                    lcd_clear();
                    lcd_put_cur(0, 0); lcd_send_string("Bruit parasite!");
                    lcd_put_cur(1, 0); lcd_send_string("Re-synchronise..");

                    etat_actuel_dcf = CHERCHE_SYNCHRO;
                }
            }
        }
        etat_precedent = etat_actuel;
    }

    // 3. DECODAGE
    if (etat_actuel_dcf == DECODAGE) {

            // --- Calcul des Minutes ---
            uint8_t min_unite = tableau_bits[21] + (tableau_bits[22]*2) + (tableau_bits[23]*4) + (tableau_bits[24]*8);
            uint8_t min_dizaine = tableau_bits[25] + (tableau_bits[26]*2) + (tableau_bits[27]*4);
            uint8_t minutes = (min_dizaine * 10) + min_unite;

            // --- Calcul des Heures ---
            uint8_t h_unite = tableau_bits[29] + (tableau_bits[30]*2) + (tableau_bits[31]*4) + (tableau_bits[32]*8);
            uint8_t h_dizaine = tableau_bits[33] + (tableau_bits[34]*2);
            uint8_t heures = (h_dizaine * 10) + h_unite;

            // --- Sauvegarde pour le main.c ---
            dcf_heures = heures;
            dcf_minutes = minutes;
            flag_nouvelle_heure = 1;

            // --- Affichage et Sonnerie ---
            lcd_clear();
            lcd_put_cur(0, 0); lcd_send_string("Trame Succes !");
            // --- Lignes à ajouter pour Tera Term ---
                    printf("\r\n=================================\r\n");
                    printf(" TRAME DECODEE AVEC SUCCES !\r\n");
                    printf(" HEURE  : %02d:%02d\r\n", heures, minutes);
                    printf("=================================\r\n\r\n");
            Buzzer_Sonnerie_Douce();

            etat_actuel_dcf = CHERCHE_SYNCHRO;
    }
}
uint8_t DCF77_NouvelleHeureDispo(uint8_t *h, uint8_t *m) {
    if (flag_nouvelle_heure == 1) {
        *h = dcf_heures;
        *m = dcf_minutes;
        flag_nouvelle_heure = 0; // On baisse le drapeau
        return 1; // Vrai : une nouvelle heure est dispo !
    }
    return 0; // Faux : rien de nouveau
}

void DCF77_Reset_Recherche(void) {
    etat_actuel_dcf = CHERCHE_SYNCHRO;
    index_bit = 0;
    printf("\r\n--- RECHERCHE DCF77 DEMARREE ---\r\n");
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("Recherche DCF...");
}
