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
#define APPLICATION_ADDRESS_1 (uint32_t)0x08008000
#define APPLICATION_ADDRESS_2 (uint32_t)0x08023000
#define TX_TIMEOUT 1000
#define CHUNK_SIZE 1024
#define CRC_SIZE 2
#define HEADER_SIZE 3
#define RECEIVE_FROM_ESP32_TIMEOUT 2000
#define PARTS_SIZE 107000
#define ONE_PAGE_SIZE 2048
#define TOTAL_PAGES 125
#define START_BIN_HEADER 0xaa
#define PACKAGE_BIN_HEADER 0xbb
#define FINISHED_BIN_HEADER 0xcc
#define USER_APP_1_STAGE 1
#define USER_APP_2_STAGE 2
#define OTA_STAGE 3
#define ROLLBACK_STAGE 7
#define FLAG_ADDRESS_OTA (uint32_t)0x0080072e0

/**
 * @brief  Convert an Integer to a string
 * @param  p_str: The string output pointer
 * @param  intnum: The integer to be converted
 * @retval None
 */
void intToStr(uint8_t *p_str, uint32_t intnum) {
    uint32_t i, divider = 1000000000, pos = 0, status = 0;

    for (i = 0; i < 10; i++) {
        p_str[pos++] = (intnum / divider) + 48;

        intnum = intnum % divider;
        divider /= 10;
        if ((p_str[pos - 1] == '0') & (status == 0)) {
            pos = 0;
        } else {
            status++;
        }
    }
}

/**
 * @brief  Print a string on the serial monitor
 * @param  p_string: The string to be printed
 * @retval None
 */
void serialPutString(uint8_t *p_string) {
    uint16_t length = 0;

    while (p_string[length] != '\0') {
        length++;
    }
    HAL_UART_Transmit(&huart5, p_string, length, TX_TIMEOUT);
}

/**
 * @brief  Transmit a byte to the serial monitor
 * @param  param The byte to be sent
 * @retval HAL_StatusTypeDef HAL_OK if OK
 */
HAL_StatusTypeDef serialPutByte(uint8_t param) {
    return HAL_UART_Transmit(&huart5, &param, 1, TX_TIMEOUT);
}

/**
 * @brief  Jump to application by flash address
 * @param  applicationAddress flash address for jumping to application
 */
void goToApps(uint32_t applicationAddress) {
    uint32_t JumpAddress;
    typedef void (*pFunction)(void);
    pFunction JumpToApplication;

    if (((*(__IO uint32_t *)applicationAddress) & 0x2FFE0000) == 0x20000000) {
        serialPutString("Going to User Application\r\n");
        // JumpAddress = *(__IO uint32_t *)(APPLICATION_ADDRESS + 4);
        JumpAddress = *(__IO uint32_t *)(applicationAddress + 4);
        JumpToApplication = (pFunction)JumpAddress;
        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t *)applicationAddress);
        JumpToApplication();
    }
}
/**
 * @brief  Calculate CRC value form receving bytes
 * @param  buf bytes in array
 * @param  len array length
 * @retval CRC value
 */
static uint16_t modbusCRC16Cal(const unsigned char *buf, unsigned int len) {
    static const uint16_t table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040};

    uint8_t xor = 0;
    uint16_t crc = 0xFFFF;

    while (len--) {
        xor = (*buf++) ^ crc;
        crc >>= 8;
        crc ^= table[xor];
    }

    return crc;
}

static uint32_t GetPage(uint32_t Address) {
    for (int indx = 0; indx < TOTAL_PAGES; indx++) {
        if ((Address < (0x08000000 + (FLASH_PAGE_SIZE * (indx + 1)))) && (Address >= (0x08000000 + FLASH_PAGE_SIZE * indx))) {
            return (0x08000000 + FLASH_PAGE_SIZE * indx);
        }
    }

    return 0;
}
/*REFERENCE, WILL BE DELETED SOON*/
// uint32_t Flash_Write_Data(uint32_t StartPageAddress, uint32_t *Data, uint16_t numberofwords) {

//     static FLASH_EraseInitTypeDef EraseInitStruct;
//     uint32_t PAGEError;
//     int sofar = 0;

//     /* Unlock the Flash to enable the flash control register access *************/
//     HAL_FLASH_Unlock();

//     /* Erase the user Flash area*/

//     uint32_t StartPage = GetPage(StartPageAddress);
//     uint32_t EndPageAdress = StartPageAddress + numberofwords * 4;
//     uint32_t EndPage = GetPage(EndPageAdress);

//     /* Fill EraseInit structure*/
//     EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
//     EraseInitStruct.PageAddress = StartPage;
//     EraseInitStruct.NbPages = ((EndPage - StartPage) / FLASH_PAGE_SIZE) + 1;

//     if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
//         /*Error occurred while page erase.*/
//         return HAL_FLASH_GetError();
//     }

//     /* Program the user Flash area word by word*/

//     while (sofar < numberofwords) {
//         if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, StartPageAddress, Data[sofar]) == HAL_OK) {
//             StartPageAddress += 4;  // use StartPageAddress += 2 for half word and 8 for double word
//             sofar++;
//         } else {
//             /* Error occurred while writing data in Flash memory*/
//             return HAL_FLASH_GetError();
//         }
//     }

//     /* Lock the Flash to disable the flash control register access (recommended
//        to protect the FLASH memory against possible unwanted operation) *********/
//     HAL_FLASH_Lock();

//     return 0;
// }
/*REFERENCE, WILL BE DELETED SOON*/

/**
 * @brief  Erase flash memory before writing
 * @param  startPageAddress start flash address to erase
 * @param  len flash address length to erase
 */
void eraseFlashMemory(uint32_t startPageAddress, uint16_t len) {
    static FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError;

    HAL_FLASH_Unlock();

    uint32_t StartPage = GetPage(startPageAddress);
    uint32_t EndPageAdress = startPageAddress + len;
    uint32_t EndPage = GetPage(EndPageAdress);

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = StartPage;
    EraseInitStruct.NbPages = len;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        /*Error occurred while page erase.*/
        return HAL_FLASH_GetError();
    }

    HAL_FLASH_Lock();
}

/**
 * @brief  Write to flash memory
 * @param  startPageAddress start flash address to write
 * @param  data array for write
 * @param  size size of array
 */
void writeFlashMemory(uint32_t startPageAddress, uint32_t *data, uint16_t size) {
    uint16_t idx = 0;

    HAL_FLASH_Unlock();

    while (idx < size) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startPageAddress, data[idx]) == HAL_OK) {
            startPageAddress += 4;  // use StartPageAddress += 2 for half word and 8 for double word
            idx++;
        } else {
            /* Error occurred while writing data in Flash memory*/
            return HAL_FLASH_GetError();
        }
    }

    HAL_FLASH_Lock();
}

/**
 * @brief  Read flash memory at specific address until specific size
 * @param  startPageAddress start flash address to read
 * @param  receiveBuff array for result of read
 * @param  size size of flash to read
 */
void readFlashMemory(uint32_t startPageAddress, uint32_t *receiveBuff, uint16_t size) {
    HAL_FLASH_Unlock();
    while (1) {
        *receiveBuff = *(__IO uint32_t *)startPageAddress;
        startPageAddress += 4;
        receiveBuff++;
        if (!(size--)) {
            break;
        }
    }
    HAL_FLASH_Lock();
}

/**
 * @brief  Copy flash memory
 * @param  srcAddress source of flash memory
 * @param  dstAddress destination
 */
void copyFlashMemory(uint32_t srcAddress, uint32_t dstAddress) {
    char stringBuff1[30] = {0};
    uint32_t receiveBuff[2 * CHUNK_SIZE] = {0};
    uint32_t num = (CHUNK_SIZE / 4) + ((CHUNK_SIZE % 4) != 0);
    uint32_t part = (PARTS_SIZE / num) + ((PARTS_SIZE / num) != 0);
    LL_IWDG_ReloadCounter(IWDG);
    for (size_t i = 0; i < part; i++) {

        // intToStr(stringBuff1, srcAddress);
        // serialPutString(stringBuff1);
        // serialPutString((uint8_t *)"\r\n");

        // intToStr(stringBuff1, dstAddress);
        // serialPutString(stringBuff1);
        // serialPutString((uint8_t *)"\r\n");
        // serialPutString((uint8_t *)"============================\r\n");
        // serialPutString((uint8_t *)".");

        readFlashMemory(srcAddress, receiveBuff, num);
        // char stringBuff[30] = {0};
        // for (size_t i = 0; i < num; i++) {
        //     intToStr(stringBuff, receiveBuff_32bit[i]);
        //     serialPutString(stringBuff);
        //     serialPutString((uint8_t *)"\r\n");

        //     LL_IWDG_ReloadCounter(IWDG);
        // }
        writeFlashMemory(dstAddress, receiveBuff, num);

        srcAddress += num;
        dstAddress += num;
    }
    serialPutString((uint8_t *)"\r\n");
}

/**
 * @brief  Step of backup flash memory
 */
void backupFlashMemory(uint16_t pageInParts) {
    LL_IWDG_ReloadCounter(IWDG);
    serialPutString((uint8_t *)"Backup Flash Memory Start...\r\n");

    eraseFlashMemory(APPLICATION_ADDRESS_2, pageInParts);
    serialPutString((uint8_t *)"Erase Destination Flash Memory Finished...\r\n");

    copyFlashMemory(APPLICATION_ADDRESS_1, APPLICATION_ADDRESS_2);
    serialPutString((uint8_t *)"Copy Flash Memory Finished...\r\n");
}

/**
 * @brief  Step of rollback flash memory
 */
void rollbackFlashMemory(uint16_t pageInParts) {
    LL_IWDG_ReloadCounter(IWDG);
    serialPutString((uint8_t *)"Rollback Flash Memory Start...\r\n");

    eraseFlashMemory(APPLICATION_ADDRESS_1, pageInParts);
    serialPutString((uint8_t *)"Erase Source Flash Memory Finished...\r\n");

    copyFlashMemory(APPLICATION_ADDRESS_2, APPLICATION_ADDRESS_1);
    serialPutString((uint8_t *)"Copy Flash Memory Finished...\r\n");
}

/**
 * @brief  Writing value to specific's single address
 * @param  startPageAddress start flash address to write
 * @param  value value to write
 */
void writeSingleFlashMemory(uint32_t startPageAddress, uint32_t value) {
    HAL_FLASH_Unlock();

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startPageAddress, value);

    HAL_FLASH_Lock();
}

/**
 * @brief  Reading value from specific's single address
 * @param  startPageAddress start flash address to read
 *
 * @return value from specific's single address
 */
uint32_t readSingleFlashMemory(uint32_t startPageAddress) {
    HAL_FLASH_Unlock();

    uint32_t value = *(__IO uint32_t *)startPageAddress;

    HAL_FLASH_Lock();

    return value;
}

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
        serialPutString((uint8_t *)"Receving Mode\r\n");  // ready to receiving serial from UART2
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
        serialPutString((uint8_t *)"Wrong Number...\r\n");
        NVIC_SystemReset();
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
                backupFlashMemory(pagesInParts);

                eraseFlashMemory(APPLICATION_ADDRESS_1, pagesInParts);
                serialPutString((uint8_t *)"Erase Source Flash Memory Finished...\r\n");

                flashDestination = APPLICATION_ADDRESS_1;

                eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
                writeSingleFlashMemory(FLAG_ADDRESS_OTA, ROLLBACK_STAGE);
                break;

            case PACKAGE_BIN_HEADER:  // header for receive .bin (chunk size)

                packetSize = buf[1] << 8 | buf[2];  // packet's  length

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
                } else if (checkSumFalse == 0 && (chunksFromSource == chunksInProcess)) {
                    serialPutString((uint8_t *)"\n\nChecksum OK, go to user apps...\r\n");
                    // HAL_Delay(1000);
                    // eraseFlashMemory(FLAG_ADDRESS_OTA, 1);
                    // writeSingleFlashMemory(FLAG_ADDRESS_OTA, USER_APP_1_STAGE);
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
