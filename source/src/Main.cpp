#include <Arduino.h>
#include <LittleFS.h>

#include <Config.hpp>
#include <Power.hpp>
#include <Panel.hpp>
#include <Wireless.hpp>

extern void networkTask(void* parameter);
extern void sensorTask(void* parameter);
extern void serialTask(void* parameter);
#if (HAS_ST7789 == true)
extern void panelTask(void* parameter);
#endif

static TaskHandle_t networkHandle = nullptr;
static TaskHandle_t sensorHandle  = nullptr;
static TaskHandle_t serialHandle  = nullptr;

void setup() {
#if (HAS_BATTERY == true)
    Power::inst().onShutdown = []() {
#if (HAS_ST7789 == true)
        Panel::inst().setBrightness(0);
#endif
        Wireless::inst().control(false);
    };
    Power::inst().init();
#endif
    xTaskCreate(sensorTask, nullptr, 1024 * 6, nullptr, 5, &sensorHandle);

    Serial.setDebugOutput(false);
    Serial.begin(921600);
    Serial.flush();

    USBSerial.setTxBufferSize(8192);
    USBSerial.setRxBufferSize(8192);
#if (CORE_DEBUG_LEVEL == 0)
    USBSerial.setDebugOutput(false);
#else
    USBSerial.setDebugOutput(true);
#endif
    USBSerial.begin(921600);
    USBSerial.flush();

    disableCore0WDT();
#ifndef CONFIG_FREERTOS_UNICORE
    disableCore1WDT();
#endif

    if (psramInit()) { heap_caps_malloc_extmem_enable(64); }

    LittleFS.begin(true);

    ChipTemp::inst().init();
    Wireless::inst().init();

    srand(esp_random());

    xTaskCreate(networkTask, nullptr, 1024 * 6, nullptr, 3, &networkHandle);
    xTaskCreate(serialTask, nullptr, 1024 * 6, nullptr, 4, &serialHandle);
#if (HAS_ST7789 == true)
    xTaskCreate(panelTask, nullptr, 1024 * 5, nullptr, 2, nullptr);
#endif

#if (CORE_DEBUG_LEVEL != 0)
#warning "DEBUG VERSION! JUST FOR TEST!"
#endif
}

void loop() {
    static uint32_t count = 0;
    delay(50);
    count++;

#if (HAS_BATTERY == true)
    Power::inst().handle();
#endif
    if (count % 20 == 0) {
        ChipTemp::inst().handle();
        Wireless::inst().handle(count / 20);
        if (ChipTemp::inst().tempature > 70.0) { Wireless::inst().control(false); }
#if (HAS_BATTERY == true)
        if (ChipTemp::inst().tempature > 75.0) { Power::inst().shutdown(); }
#endif
    }
}
