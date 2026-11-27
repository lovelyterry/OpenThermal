#include <Arduino.h>

void serialSendFrame(const uint8_t* data, size_t len) {
    if (USBSerial && USBSerial.availableForWrite() >= (int)len) {
        USBSerial.write(data, len);
    }
}

void serialTask(void*) {
    for (;;) {
        while (USBSerial.available() > 0) { USBSerial.read(); }
        delay(10);
    }
}
