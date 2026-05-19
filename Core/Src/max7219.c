#include "max7219.h"

extern SPI_HandleTypeDef hspi1;

// --- NOUVELLE POLICE 5x7 ---
// Dessinée spécialement pour tes matrices à 5 colonnes !
const uint8_t font5x7[10][5] = {
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x62, 0x51, 0x49, 0x49, 0x46}, // 2
    {0x22, 0x41, 0x49, 0x49, 0x36}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x2F, 0x49, 0x49, 0x49, 0x31}, // 5
    {0x3E, 0x49, 0x49, 0x49, 0x32}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x26, 0x49, 0x49, 0x49, 0x3E}  // 9
};

// Le dessin des deux points ":" (Centré sur la colonne 3)
const uint8_t font_colon[5] = {0x00, 0x00, 0x14, 0x00, 0x00};

void MAX7219_Write(uint8_t address, uint8_t data) {
    uint8_t buffer[2] = {address, data};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    for (int i = 0; i < NUM_MATRICES; i++) {
        HAL_SPI_Transmit(&hspi1, buffer, 2, 100);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void MAX7219_WriteSpecific(uint8_t matrix_index, uint8_t address, uint8_t data) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    for (int i = NUM_MATRICES - 1; i >= 0; i--) {
        if (i == matrix_index) {
            uint8_t buffer[2] = {address, data};
            HAL_SPI_Transmit(&hspi1, buffer, 2, 100);
        } else {
            uint8_t nop_buffer[2] = {0x00, 0x00};
            HAL_SPI_Transmit(&hspi1, nop_buffer, 2, 100);
        }
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void MAX7219_Init(void) {
    MAX7219_Write(0x09, 0x00);

    // --- CORRECTION CRUCIALE ICI ---
    // 0x04 veut dire : on n'utilise que 5 colonnes (0,1,2,3,4) au lieu de 8 !
    // Ça va régler ton problème de double couleur !
    MAX7219_Write(0x0B, 0x04);

    MAX7219_Write(0x0C, 0x01);
    MAX7219_Write(0x0F, 0x00);
    MAX7219_SetBrightness(2);  // Lumière douce
    MAX7219_Clear();
}

void MAX7219_SetBrightness(uint8_t brightness) {
    if(brightness > 15) brightness = 15;
    MAX7219_Write(0x0A, brightness);
}

void MAX7219_Clear(void) {
    // On efface seulement les 5 colonnes
    for (int col = 1; col <= 5; col++) {
        MAX7219_Write(col, 0x00);
    }
}

void MAX7219_DisplayTime(uint8_t hours, uint8_t minutes) {
    uint8_t h_dizaine = hours / 10;
    uint8_t h_unite = hours % 10;
    uint8_t m_dizaine = minutes / 10;
    uint8_t m_unite = minutes % 10;

    // On boucle seulement sur les 5 colonnes (de 1 à 5)
    for (int col = 1; col <= 5; col++) {
        MAX7219_WriteSpecific(0, col, font5x7[h_dizaine][col-1]);
        MAX7219_WriteSpecific(1, col, font5x7[h_unite][col-1]);
        MAX7219_WriteSpecific(2, col, font_colon[col-1]);
        MAX7219_WriteSpecific(3, col, font5x7[m_dizaine][col-1]);
        MAX7219_WriteSpecific(4, col, font5x7[m_unite][col-1]);
    }
}

void MAX7219_DisplayZeros(void) {
    // La police du chiffre '0' est dans font5x7[0]
    for (int col = 1; col <= 5; col++) {
        MAX7219_WriteSpecific(0, col, font5x7[0][col-1]);
        MAX7219_WriteSpecific(1, col, font5x7[0][col-1]);
        MAX7219_WriteSpecific(2, col, font5x7[0][col-1]);
        MAX7219_WriteSpecific(3, col, font5x7[0][col-1]);
        MAX7219_WriteSpecific(4, col, font5x7[0][col-1]);
    }
}

// --- LE SCANNER DE DEBOGAGE ---
// Allume UN SEUL point précis sur une matrice précise.
void MAX7219_TestPixel(uint8_t matrice, uint8_t colonne, uint8_t ligne) {
    MAX7219_Clear(); // On éteint tout

    // Le bit_mask permet d'allumer une seule ligne.
    // Ligne 1 = 0x01, Ligne 2 = 0x02, Ligne 3 = 0x04, etc.
    uint8_t bit_mask = (1 << (ligne - 1));

    MAX7219_WriteSpecific(matrice, colonne, bit_mask);
}
