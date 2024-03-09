#include <Arduino.h>
#include <Crc16.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define CHIP_SELECT 5
// #define CHIP_SELECT 27
// #define BIN_NAME "/userapp-stm32-f0.bin"
// #define BIN_NAME "/userapp-stm32-f0_02.bin"
#define BIN_NAME "/firmware.bin"
// #define BIN_NAME "/stm32-climate-controller.bin"

#define CHUNK_SIZE 1024
#define START_BIN_HEADER 0xaa
#define PACKAGE_BIN_HEADER 0xbb
#define FINISHED_BIN_HEADER 0xcc

uint16_t line = 1;
uint8_t binBuff[2 * CHUNK_SIZE] = {0};
uint16_t idxBuff = 0;

Crc16 crc16;

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);
    // Serial2.begin(9600);

    Serial.printf("SD Begin: %d\n", SD.begin(CHIP_SELECT));
    Serial.printf("SD File: %d\n", SD.exists(BIN_NAME));

    File binFile;
    Serial.println("Start Reading Binary");
    binFile = SD.open(BIN_NAME);

    uint32_t fileSize = binFile.size();
    Serial.printf(".bin Size: %d\n", fileSize);

    uint8_t d = round(fileSize / CHUNK_SIZE) + 1;
    Serial.printf("Amount of chunks: %d\n", d);

    // remaining .bin's size after divided by amount of chunks
    uint16_t remainFileSizeCal = fileSize - ((d - 1) * CHUNK_SIZE);

    uint8_t count = 0;
    int32_t remainFileSize = 0;

    // uint8_t testChar_8bit[] = {
    //     0x00,
    //     0x80,
    //     0x00,
    //     0x20,
    //     0x22,
    //     0x22,
    //     0x22,
    //     0x22,
    //     0x33,
    //     0x33,
    //     0x33,
    //     0x33,
    //     0x44,
    //     0x44,
    //     0x44,
    //     0x44,
    // };
    // uint32_t buff_32bit[16] = {0};
    // buff_32bit[0] = testChar_8bit[3] << 24 | testChar_8bit[2] << 16 | testChar_8bit[1] << 8 | testChar_8bit[0];
    // Serial.printf("%02x\n", buff_32bit[0]);

    delay(10000);

    Serial.println("Send Start Header");
    Serial2.write(START_BIN_HEADER);
    Serial2.write(0);
    Serial2.write(0);

    delay(20000);  // waiting for receiver ready

    if (binFile) {
        while (binFile.available()) {
            uint8_t b = binFile.read();

            binBuff[idxBuff] = b;
            idxBuff++;
            // Serial.printf("idxBuff: %d\n", idxBuff);

            // .bin size: 512 or remaining .bin after divided by amount of chunks
            if ((idxBuff >= CHUNK_SIZE && count < d) || ((idxBuff == (remainFileSizeCal)) && (remainFileSize == remainFileSizeCal))) {

                Serial.printf("idxBuff: %d\n", idxBuff);
                // Serial.printf("%02x", idxBuff >> 8);
                // Serial.printf(" %02x\n", idxBuff & 0xff);

                // sending package .bin header and .bin's length
                Serial2.write(PACKAGE_BIN_HEADER);
                Serial2.write(idxBuff >> 8);
                Serial2.write(idxBuff & 0xff);

                delay(1000);

                // calculating crc using modbus crc
                uint16_t crcModbus = crc16.Modbus(binBuff, 0, idxBuff);
                Serial.printf("\ncrcModbus_1: %02x, %d\n", crcModbus & 0xff, crcModbus & 0xff);
                Serial.printf("crcModbus_2: %02x, %d\n", crcModbus >> 8, crcModbus >> 8);

                // sending chunk's .bin and crc
                Serial2.write(binBuff, CHUNK_SIZE);
                Serial2.write(crcModbus & 0xff);
                Serial2.write(crcModbus >> 8);
                for (uint16_t i = 0; i < idxBuff; i++) {
                    Serial.printf("%02x", binBuff[i]);
                }

                idxBuff = 0;
                count++;
                memset(binBuff, 0, CHUNK_SIZE);
                remainFileSize = fileSize - (count * CHUNK_SIZE);
                Serial.printf("\ncount, remain: %d, %d \n", count, remainFileSize);
                delay(1000);
            }

            delay(1);
        }
    }
    Serial.println("\nFinish Reading Binary");

    delay(10000);

    // sending finished-send .bin header with zero length
    Serial2.write(FINISHED_BIN_HEADER);
    Serial2.write(d >> 8);
    Serial2.write(d & 0xff);
    Serial.println("Send Finished Header");
}

void loop() {
}
