#include "iap.h"

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

uint32_t GetPage(uint32_t Address) {
    for (int indx = 0; indx < TOTAL_PAGES; indx++) {
        if ((Address < (0x08000000 + (FLASH_PAGE_SIZE * (indx + 1)))) && (Address >= (0x08000000 + FLASH_PAGE_SIZE * indx))) {
            return (0x08000000 + FLASH_PAGE_SIZE * indx);
        }
    }

    return 0;
}

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


