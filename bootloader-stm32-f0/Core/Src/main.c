/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "iap.h"
#include "stdbool.h"
#include "utils.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART5_UART_Init(void);
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
int main(void) {
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
    MX_IWDG_Init();
    MX_TIM6_Init();
    MX_USART5_UART_Init();
    MX_USART2_UART_Init();
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    uint8_t buf[HEADER_SIZE] = {0};
    uint8_t recUart[1] = {0};
    uint8_t binBuff[2 * CHUNK_SIZE] = {0};
    uint16_t count = 0;
    uint16_t packetSize = 0;
    uint16_t binSize = 0;
    uint32_t buff_32bit[2 * CHUNK_SIZE] = {0};
    uint32_t receiveBuff_32bit[2 * CHUNK_SIZE] = {0};
    uint32_t flashDestination = 0;
    uint16_t pagesInParts = (PARTS_SIZE / ONE_PAGE_SIZE) + (PARTS_SIZE % ONE_PAGE_SIZE != 0);
    uint16_t checkSumFalse = 0;
    uint16_t chunksFromSource = 0;
    uint16_t chunksInProcess = 0;
    uint32_t flagAddressVal = 0;
    bool startBinStage = false;

    char stringBuff[30] = {0};

    HAL_StatusTypeDef uartState;

    LL_IWDG_SetReloadCounter(IWDG, 4095);  // 4095 for 27,17s
    LL_IWDG_ReloadCounter(IWDG);

    serialPutString((uint8_t *)"--BOOTLOADER APPLICATION--\r\n");
    // HAL_Delay(100);
    // serialPutString((uint8_t *)"Choose Number: \r\n");
    // HAL_Delay(100);
    // HAL_UART_Receive(&huart5, recUart, 1, 60000);  // waiting for user-input (1/2/3) in 60s
    // HAL_Delay(100);

    // intToStr(stringBuff, recUart[0]);
    // serialPutString(stringBuff);
    // serialPutString((uint8_t *)"\r\n");
    HAL_Delay(1000);

    // read flag address for OTA
    flagAddressVal = readSingleFlashMemory(FLAG_ADDRESS_OTA);
    intToStr(stringBuff, flagAddressVal);
    serialPutString(stringBuff);
    serialPutString((uint8_t *)"\r\n");

    // switch (recUart[0]) {
    switch (flagAddressVal) {
    case USER_APP_1_STAGE:
        goToApps(APPLICATION_ADDRESS_1);  // going to User App 1
        break;

    case USER_APP_2_STAGE:
        goToApps(APPLICATION_ADDRESS_2);  // going to User App 2
        break;

    case OTA_STAGE:
        serialPutString((uint8_t *)"Receiving Mode\r\n");  // ready to receiving serial from UART2
        break;

        // case 4:
        //     serialPutString((uint8_t *)"Writing to Flash Test\r\n");
        //     uint16_t idx = 0;
        //     for (size_t i = 0; i < 16; i += 4) {
        //         buff_32bit[idx] = (testChar_8bit[i + 3] << 24) |
        //                           (testChar_8bit[i + 2] << 16) |
        //                           (testChar_8bit[i + 1] << 8) |
        //                           (testChar_8bit[i]);
        //         idx++;
        //     }

        //     Flash_Write_Data(0x08023000, buff_32bit, 16);
        //     break;

        // case 5:
        //     eraseFlashMemory(APPLICATION_ADDRESS_1, PARTS_SIZE);
        //     serialPutString((uint8_t *)"Erase Flash Memory\r\n");
        //     uint16_t idxs = 0;
        //     for (size_t i = 0; i < 24; i += 4) {
        //         buff_32bit[idxs] = (testChar_8bit[i + 3] << 24) |
        //                            (testChar_8bit[i + 2] << 16) |
        //                            (testChar_8bit[i + 1] << 8) |
        //                            (testChar_8bit[i]);
        //         idxs++;
        //     }
        //     writeFlashMemory(APPLICATION_ADDRESS_2, buff_32bit, 6);
        //     serialPutString((uint8_t *)"Write Flash Memory\r\n");
        //     break;

        // case 6:
        //     serialPutString((uint8_t *)"Copy Flash Memory Start...\r\n");

        //     eraseFlashMemory(APPLICATION_ADDRESS_2, PARTS_SIZE);
        //     serialPutString((uint8_t *)"Erase Flash Memory Start...\r\n");

        //     copyFlashMemory(APPLICATION_ADDRESS_1, APPLICATION_ADDRESS_2);

        //     serialPutString((uint8_t *)"Copy Flash Memory Finished...\r\n");
        //     break;

    case ROLLBACK_STAGE:  // testing for rollback flash memory to source address
        rollbackFlashMemory(pagesInParts);
        HAL_Delay(1000);
        eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
        writeSingleFlashMemory(FLAG_ADDRESS_OTA, USER_APP_1_STAGE);
        goToApps(APPLICATION_ADDRESS_1);
        HAL_Delay(1000);

        break;

    default:
        // serialPutString((uint8_t *)"Wrong Number...\r\n");
        // NVIC_SystemReset();
        eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
        writeSingleFlashMemory(FLAG_ADDRESS_OTA, USER_APP_1_STAGE);
        goToApps(APPLICATION_ADDRESS_1);
        HAL_Delay(1000);
        break;
    }

    serialPutString((uint8_t *)"Waiting for Bytes...\r\n");
    while (1) {
        // LL_IWDG_ReloadCounter(IWDG);

        uartState = HAL_UART_Receive(&huart2, buf, HEADER_SIZE, RECEIVE_FROM_ESP32_TIMEOUT);
        if (uartState == HAL_OK) {
            // serialPutByte(buf[0]);
            // serialPutByte(buf[1]);
            // serialPutByte(buf[2]);
            // LL_IWDG_ReloadCounter(IWDG);
            switch (buf[0]) {  // header for erase flash page before flash
            case START_BIN_HEADER:
                if (!startBinStage) {  // ensure only once pass this process
                    backupFlashMemory(pagesInParts);

                    eraseFlashMemory(APPLICATION_ADDRESS_1, pagesInParts);
                    serialPutString((uint8_t *)"Erase Source Flash Memory Finished...\r\n");

                    flashDestination = APPLICATION_ADDRESS_1;

                    eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
                    writeSingleFlashMemory(FLAG_ADDRESS_OTA, ROLLBACK_STAGE);

                    startBinStage = true;
                }

                break;

            case PACKAGE_BIN_HEADER:                    // header for receive .bin (chunk size)
                if (startBinStage) {                    // must pass START_BIN_HEADER stage first
                    packetSize = buf[1] << 8 | buf[2];  // packet's  length
                }

                break;

            case FINISHED_BIN_HEADER:  // header for finished receive chunk
                serialPutString((uint8_t *)"\nFinished received chunk...\r\n");

                chunksFromSource = buf[1] << 8 | buf[2];
                intToStr(stringBuff, chunksFromSource);
                serialPutString((uint8_t *)"\nChunk From Source: ");
                serialPutString(stringBuff);

                intToStr(stringBuff, binSize);
                serialPutString((uint8_t *)"\n.bin Size: ");
                serialPutString(stringBuff);

                // in this section will be a decision will rollback or not
                // if true, continue to firmware
                // if false, rollback
                if (checkSumFalse > 0 || (chunksFromSource != chunksInProcess)) {
                    serialPutString((uint8_t *)"\n\nChecksum failed...\r\n");
                    rollbackFlashMemory(pagesInParts);
                    HAL_Delay(1000);
                    eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
                    writeSingleFlashMemory(FLAG_ADDRESS_OTA, USER_APP_1_STAGE);
                    goToApps(APPLICATION_ADDRESS_1);
                    HAL_Delay(1000);

                } else if (checkSumFalse == 0 && (chunksFromSource == chunksInProcess)) {
                    serialPutString((uint8_t *)"\n\nChecksum OK, go to user apps...\r\n");
                    HAL_Delay(1000);
                    eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
                    writeSingleFlashMemory(FLAG_ADDRESS_OTA, USER_APP_1_STAGE);
                    HAL_Delay(1000);
                    goToApps(APPLICATION_ADDRESS_1);
                }

                packetSize = 0;
                binSize = 0;

                break;

            default:
                break;
            }
            if (packetSize > HEADER_SIZE) {  // packet's size MUST greater than header's size
                uartState = HAL_UART_Receive(&huart2, binBuff, CHUNK_SIZE + CRC_SIZE, RECEIVE_FROM_ESP32_TIMEOUT);
                if (uartState == HAL_OK) {

                    // calculating crc's value from .bin chunk
                    uint16_t crcModbusCal = modbusCRC16Cal(binBuff, packetSize);

                    // .bin crc checking
                    if (crcModbusCal == (binBuff[CHUNK_SIZE + 1] << 8 | binBuff[CHUNK_SIZE])) {
                        for (uint32_t i = 0; i < CHUNK_SIZE + CRC_SIZE; i++) {
                            serialPutByte(binBuff[i]);
                        }

                        // convert from 8bit to 32bit in one array
                        uint32_t idx = 0;
                        uint32_t num = (packetSize / 4) + ((packetSize % 4) != 0);
                        char stringBuff[10] = {0};

                        for (size_t i = 0; i < packetSize; i += 4) {
                            buff_32bit[idx] = (binBuff[i + 3] << 24) |
                                              (binBuff[i + 2] << 16) |
                                              (binBuff[i + 1] << 8) |
                                              (binBuff[i]);

                            idx++;
                        }

                        // writing to flash
                        writeFlashMemory(flashDestination, buff_32bit, num);

                        flashDestination += packetSize;  // shifting flash address
                        binSize += packetSize;           // for checking file size after all .bin writes completely
                        chunksInProcess++;

                        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

                        serialPutString((uint8_t *)"====>CRC OK, Write Flash Memory\r\n");
                    } else {
                        checkSumFalse++;
                    }
                    LL_IWDG_ReloadCounter(IWDG);
                }
            }
        }

        // LL_IWDG_ReloadCounter(IWDG);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
    }
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1) {
    }
    LL_RCC_LSI_Enable();

    /* Wait till LSI is ready */
    while (LL_RCC_LSI_IsReady() != 1) {
    }
    LL_RCC_HSE_EnableCSS();
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLL_MUL_6, LL_RCC_PREDIV_DIV_1);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    LL_SetSystemCoreClock(48000000);

    /* Update the time base */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief IWDG Initialization Function
 * @param None
 * @retval None
 */
static void MX_IWDG_Init(void) {

    /* USER CODE BEGIN IWDG_Init 0 */

    /* USER CODE END IWDG_Init 0 */

    /* USER CODE BEGIN IWDG_Init 1 */

    /* USER CODE END IWDG_Init 1 */
    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_256);
    LL_IWDG_SetReloadCounter(IWDG, 4095);
    while (LL_IWDG_IsReady(IWDG) != 1) {
    }

    LL_IWDG_ReloadCounter(IWDG);
    /* USER CODE BEGIN IWDG_Init 2 */

    /* USER CODE END IWDG_Init 2 */
}

/**
 * @brief TIM6 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM6_Init(void) {

    /* USER CODE BEGIN TIM6_Init 0 */

    /* USER CODE END TIM6_Init 0 */

    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);

    /* USER CODE BEGIN TIM6_Init 1 */

    /* USER CODE END TIM6_Init 1 */
    TIM_InitStruct.Prescaler = 48000;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0xFFFF;
    LL_TIM_Init(TIM6, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM6);
    /* USER CODE BEGIN TIM6_Init 2 */

    /* USER CODE END TIM6_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

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
    if (HAL_UART_Init(&huart2) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * @brief USART5 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART5_UART_Init(void) {

    /* USER CODE BEGIN USART5_Init 0 */

    /* USER CODE END USART5_Init 0 */

    /* USER CODE BEGIN USART5_Init 1 */

    /* USER CODE END USART5_Init 1 */
    huart5.Instance = USART5;
    huart5.Init.BaudRate = 115200;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;
    huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart5) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN USART5_Init 2 */

    /* USER CODE END USART5_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /* USER CODE BEGIN MX_GPIO_Init_1 */
    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USER CODE BEGIN MX_GPIO_Init_2 */
    /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM7 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM7) {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
