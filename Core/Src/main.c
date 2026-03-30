/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  GPIO_PinState etat_precedent = GPIO_PIN_RESET;

    // Les chronomètres
    uint32_t temps_front_montant = 0;
    uint32_t temps_front_descendant = 0;

    // La mémoire du DCF77
    uint8_t tableau_bits[60] = {0}; // Notre train de 60 wagons
    uint8_t index_bit = 0;          // Le numéro du wagon actuel

    // Les 3 états de notre système
    typedef enum { CHERCHE_SYNCHRO, LECTURE_DONNEES, DECODAGE } Etat_DCF;
    Etat_DCF etat_actuel_dcf = CHERCHE_SYNCHRO;

    printf("\r\n--- DEMARRAGE SYSTEME DCF77 ---\r\n");
    printf("Recherche du signal de synchronisation (Attente max 1 minute)...\r\n");
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    /* Infinite loop */
      /* USER CODE BEGIN WHILE */
      while (1)
      {
          GPIO_PinState etat_actuel = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);

          if (etat_actuel != etat_precedent) {

              // ---------------------------------------------------------
              // 1. FRONT MONTANT (Début d'un BIP, fin d'un silence)
              // ---------------------------------------------------------
              if (etat_actuel == GPIO_PIN_SET) {
                  temps_front_montant = HAL_GetTick();

                  // On calcule combien de temps a duré le silence précédent
                  uint32_t duree_silence = temps_front_montant - temps_front_descendant;

                  // Si le silence a duré plus de 1500ms, c'est la SYNCHRO !
                  if (duree_silence > 1500) {
                      printf("\r\n>>> SYNCHRONISATION TROUVEE ! (Nouvelle minute) <<<\r\n");
                      etat_actuel_dcf = LECTURE_DONNEES; // On passe en mode lecture
                      index_bit = 0;                     // On repart au wagon zéro
                  }
              }

              // ---------------------------------------------------------
              // 2. FRONT DESCENDANT (Fin d'un BIP, début d'un silence)
              // ---------------------------------------------------------
              else {
                  temps_front_descendant = HAL_GetTick();

                  // On calcule combien de temps a duré le bip (pour avoir 0 ou 1)
                  uint32_t duree_bip = temps_front_descendant - temps_front_montant;

                  if (etat_actuel_dcf == LECTURE_DONNEES) {
                      uint8_t bit_recu = 2; // 2 = erreur par défaut

                      if (duree_bip > 70 && duree_bip < 130) {
                          bit_recu = 0;
                      }
                      else if (duree_bip > 170 && duree_bip < 230) {
                          bit_recu = 1;
                      }

                      // Si le bit est valide, on le range dans le tableau
                      if (bit_recu != 2) {
                          tableau_bits[index_bit] = bit_recu;
                          printf("Bit %02d : %d\r\n", index_bit, bit_recu);
                          index_bit++;

                          // Si on a lu nos 59 bits, on lance le décodage !
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

          // ---------------------------------------------------------
          // 3. LE DECODAGE DE L'HEURE (Conversion BCD)
          // ---------------------------------------------------------
                if (etat_actuel_dcf == DECODAGE) {

                    // --- L'HEURE ---
                    uint8_t min_unite = tableau_bits[21] + (tableau_bits[22]*2) + (tableau_bits[23]*4) + (tableau_bits[24]*8);
                    uint8_t min_dizaine = tableau_bits[25] + (tableau_bits[26]*2) + (tableau_bits[27]*4);
                    uint8_t minutes = (min_dizaine * 10) + min_unite;

                    uint8_t h_unite = tableau_bits[29] + (tableau_bits[30]*2) + (tableau_bits[31]*4) + (tableau_bits[32]*8);
                    uint8_t h_dizaine = tableau_bits[33] + (tableau_bits[34]*2);
                    uint8_t heures = (h_dizaine * 10) + h_unite;

                    // --- LA DATE ---
                    uint8_t jour_unite = tableau_bits[36] + (tableau_bits[37]*2) + (tableau_bits[38]*4) + (tableau_bits[39]*8);
                    uint8_t jour_dizaine = tableau_bits[40] + (tableau_bits[41]*2);
                    uint8_t jour = (jour_dizaine * 10) + jour_unite;

                    uint8_t mois_unite = tableau_bits[45] + (tableau_bits[46]*2) + (tableau_bits[47]*4) + (tableau_bits[48]*8);
                    uint8_t mois_dizaine = tableau_bits[49]; // Le bit 49 vaut 10
                    uint8_t mois = (mois_dizaine * 10) + mois_unite;

                    uint8_t annee_unite = tableau_bits[50] + (tableau_bits[51]*2) + (tableau_bits[52]*4) + (tableau_bits[53]*8);
                    uint8_t annee_dizaine = tableau_bits[54] + (tableau_bits[55]*2) + (tableau_bits[56]*4) + (tableau_bits[57]*8);
                    uint8_t annee = (annee_dizaine * 10) + annee_unite;

                    // --- SAISON (ÉTÉ / HIVER) ---
                    char saison[10] = "Inconnu";
                    if (tableau_bits[17] == 1) {
                        sprintf(saison, "ETE");
                    } else if (tableau_bits[18] == 1) {
                        sprintf(saison, "HIVER");
                    }

                    // --- AFFICHAGE COMPLET TERA TERM ---
                    printf("\r\n=================================\r\n");
                    printf("         TRAME COMPLETE !        \r\n");
                    printf("=================================\r\n");
                    printf(" HEURE  : %02d:%02d\r\n", heures, minutes);
                    printf(" DATE   : %02d/%02d/20%02d\r\n", jour, mois, annee);
                    printf(" SAISON : Heure d'%s\r\n", saison);
                    printf("=================================\r\n\r\n");

                    // On retourne en attente de la minute suivante
                    etat_actuel_dcf = CHERCHE_SYNCHRO;
          }

          HAL_Delay(5); // Anti-rebond

        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */
      }}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
