#ifndef INC_LCD_I2C_H_
#define INC_LCD_I2C_H_

#include "main.h"

// --- Configuration de l'adresse ---
// Si ton écran est 0x27, laisse 0x4E. S'il est 0x3F, mets 0x7E.
//#define SLAVE_ADDRESS_LCD 0x4E
#define SLAVE_ADDRESS_LCD 0x4E  // Essaye 0x7E à la place de 0x4E

// --- Déclaration des fonctions ---
void lcd_init(void);                     // Initialise l'écran
void lcd_send_cmd(char cmd);             // Envoie une commande (ex: effacer)
void lcd_send_data(char data);           // Envoie un seul caractère
void lcd_send_string(char *str);         // Envoie une phrase entière
void lcd_put_cur(int row, int col);      // Place le curseur (Ligne 0-1, Colonne 0-15)
void lcd_clear(void);                    // Efface tout l'écran

#endif /* INC_LCD_I2C_H_ */
