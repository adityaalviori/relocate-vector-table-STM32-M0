#ifndef _H_UTILS_
#define _H_UTILS_

#include "main.h"

#define TX_TIMEOUT 1000

void serialPutString(uint8_t *p_string);
HAL_StatusTypeDef serialPutByte(uint8_t param);
void intToStr(uint8_t *p_str, uint32_t intnum);
uint16_t modbusCRC16Cal(const unsigned char *buf, unsigned int len);

#endif