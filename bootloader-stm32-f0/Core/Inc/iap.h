#ifndef _H_IAP_
#define _H_IAP_

#include "main.h"

#define APPLICATION_ADDRESS_1 (uint32_t)0x08008000
#define APPLICATION_ADDRESS_2 (uint32_t)0x08023000


#define TOTAL_PAGES 125
#define CHUNK_SIZE 1024
#define PARTS_SIZE 107000

void goToApps(uint32_t applicationAddress);
uint32_t GetPage(uint32_t Address);
void eraseFlashMemory(uint32_t startPageAddress, uint16_t len);
void writeFlashMemory(uint32_t startPageAddress, uint32_t *data, uint16_t size);
void readFlashMemory(uint32_t startPageAddress, uint32_t *receiveBuff, uint16_t size);
void copyFlashMemory(uint32_t srcAddress, uint32_t dstAddress);
void backupFlashMemory(uint16_t pageInParts);
void rollbackFlashMemory(uint16_t pageInParts);
void writeSingleFlashMemory(uint32_t startPageAddress, uint32_t value);
uint32_t readSingleFlashMemory(uint32_t startPageAddress);

#endif