#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>

#include <lwip/sockets.h>

#include <Wireless.hpp>
#include <Power.hpp>
#include <Config.hpp>
#include <HTPA32.hpp>

extern void serialSendFrame(const uint8_t* data, size_t len);
extern void networkSendFrame(const uint8_t* data, size_t len);

uint16_t             sensorRawData[IR_WIDTH * IR_HEIGHT] = { 0 };
SemaphoreHandle_t    sensorUpdateSemaphore               = nullptr;
bool                 sensorPause                         = false;
extern volatile bool bootFinish;

void sensorTask(void*) {
    sensorUpdateSemaphore = xSemaphoreCreateBinary();
    HTPA32::init();
    sensorPause = false;

    for (;;) {
        if (Power::inst().readKey()) { sensorPause = !sensorPause; }
        if (!sensorPause || bootFinish == false) {
            static uint16_t rawBuf[IR_WIDTH * IR_HEIGHT];
            HTPA32::handle(rawBuf);
            for (int y = 0; y < IR_HEIGHT; y++) {
                for (int x = 0; x < IR_WIDTH; x++) {
                    sensorRawData[y * IR_WIDTH + x] = rawBuf[x * IR_WIDTH + y];
                }
            }
#if 0
            for (int y = 0; y < IR_HEIGHT; y++) {
                for (int x = 0; x < IR_WIDTH; x++) {
                    float tempC = (float)sensorRawData[y * IR_WIDTH + x] / 10.0f - 273.15f;
                    USBSerial.printf("%.2f ", tempC);
                }
                USBSerial.printf("\r\n");
            }
#endif
        } else {
            delay(100);
        }
        xSemaphoreGive(sensorUpdateSemaphore);

        static constexpr size_t payloadLen = IR_WIDTH * IR_HEIGHT * sizeof(uint16_t);
        static uint8_t          frameBuf[8 + payloadLen + 2];
        frameBuf[0] = 0xAA;
        frameBuf[1] = 0x55;
        frameBuf[2] = 0x5A;
        frameBuf[3] = 0xA5;
        frameBuf[4] = (uint8_t)(IR_WIDTH & 0xFF);
        frameBuf[5] = (uint8_t)((IR_WIDTH >> 8) & 0xFF);
        frameBuf[6] = (uint8_t)(IR_HEIGHT & 0xFF);
        frameBuf[7] = (uint8_t)((IR_HEIGHT >> 8) & 0xFF);
        memcpy(&frameBuf[8], sensorRawData, payloadLen);

        uint16_t checksum = 0;
        for (size_t i = 0; i < payloadLen; i++) { checksum += frameBuf[8 + i]; }
        frameBuf[8 + payloadLen]     = (uint8_t)(checksum & 0xFF);
        frameBuf[8 + payloadLen + 1] = (uint8_t)((checksum >> 8) & 0xFF);

        serialSendFrame(frameBuf, sizeof(frameBuf));
        networkSendFrame(frameBuf, sizeof(frameBuf));
    }
}
