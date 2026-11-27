#pragma once

#include <Config.hpp>
#include <functional>
#include "driver/temp_sensor.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_system.h"

class ChipTemp {
public:
    static ChipTemp& inst() {
        static ChipTemp chipTemp{};
        return chipTemp;
    }

    float tempature = 0;
    int   status    = 0;

    virtual void init() {
        status = temp_sensor_set_config(temp_sensor_config_t{
            .dac_offset = TSENS_DAC_L2,
            .clk_div    = 6,
        });
        status = temp_sensor_start();
    }

    virtual void handle() {
        status = temp_sensor_read_celsius(&tempature);
        if (status != ESP_OK) {
            tempature = 0;
            init();
        }
    }
};

static inline float calculateBatteryCapacity(float voltage) {
#define MAX_VOLTAGE 4.15
#define MIN_VOLTAGE 3.60
    if (voltage >= MAX_VOLTAGE) {
        return 100.0;
    } else if (voltage <= MIN_VOLTAGE) {
        return 0.0;
    } else {
        return (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100.0;
    }
}

template <size_t N = 10>
class SlidingWindow {
    float  buf[N] = { 0 };
    float  sum    = 0;
    size_t idx    = 0;
    size_t count  = 0;

public:
    float filter(float value) {
        if (count == N) {
            sum -= buf[idx];
        } else {
            count++;
        }
        buf[idx] = value;
        sum += value;
        idx = (idx + 1) % N;
        return sum / count;
    }
};

class Power {
public:
    static Power&
    inst() {
        static Power power{};
        return power;
    }

    float                 voltage       = 0.0;
    bool                  keyLevel      = false;
    uint32_t              keyPress      = 0;
    bool                  status        = false;
    volatile bool         powerLocked   = false;
    bool                  keyUnlocked   = false;
    uint32_t              pressStartMs  = 0;
    uint32_t              pressDuration = 0;
    bool                  keyStatus     = false;
    esp_timer_handle_t    bootLockTimer = nullptr;
    std::function<void()> onShutdown    = nullptr;

#if (HAS_BATTERY == true)
    static void bootLockTimerCb(void* arg) {
        auto* self = static_cast<Power*>(arg);
        if (!self || self->powerLocked) { return; }
        if (gpio_get_level((gpio_num_t)KEY_DOWN) == 0) {
            self->powerLocked = true;
            self->control(true);
        } else {
            self->shutdown();
        }
    }
#endif

    virtual void init() {
#if (HAS_BATTERY == true)
        esp_reset_reason_t reason       = esp_reset_reason();
        bool               isWarmReboot = (reason == ESP_RST_SW || reason == ESP_RST_PANIC || reason == ESP_RST_INT_WDT || reason == ESP_RST_TASK_WDT || reason == ESP_RST_WDT);

        pinMode(ADC_BAT, INPUT);
        pinMode(KEY_DOWN, INPUT_PULLUP);
        gpio_hold_dis((gpio_num_t)POWER_EN);
        pinMode(POWER_EN, OUTPUT);

        if (isWarmReboot) {
            powerLocked = true;
            keyUnlocked = true;
            control(true);
        } else {
            powerLocked = false;
            keyUnlocked = false;
            control(false);
            if (!bootLockTimer) {
                esp_timer_create_args_t args = {};
                args.callback                = &Power::bootLockTimerCb;
                args.arg                     = this;
                args.dispatch_method         = ESP_TIMER_TASK;
                args.name                    = "pwr_boot_lock";
                if (esp_timer_create(&args, &bootLockTimer) == ESP_OK) {
                    uint64_t nowUs    = (uint64_t)esp_timer_get_time();
                    uint64_t targetUs = 2000ULL * 1000ULL;
                    uint64_t delayUs  = (nowUs < targetUs) ? (targetUs - nowUs) : 0ULL;
                    esp_timer_start_once(bootLockTimer, delayUs);
                }
            }
        }
#endif
    }

    bool readKey() {
#if (HAS_BATTERY == true)
        if (digitalRead(KEY_DOWN) == LOW && keyStatus == false) {
            keyStatus = true;
            return true;
        }
        if (digitalRead(KEY_DOWN) == HIGH) { keyStatus = false; }
#endif
        return false;
    }

    virtual void handle() {
#if (HAS_BATTERY == true)
        uint32_t nowMs                = (uint32_t)(esp_timer_get_time() / 1000ULL);
        voltage                       = analogReadMilliVolts(ADC_BAT) * 206.0 / 150.0 / 1000.0;
        static SlidingWindow<10> vbat = {};
        voltage                       = vbat.filter(voltage);
        if (nowMs > 3000 && voltage > 0.5f && voltage < 3.55f) {
            shutdown();
            return;
        }

        bool pressed = (gpio_get_level((gpio_num_t)KEY_DOWN) == 0);
        keyLevel     = !pressed;

        if (powerLocked) {
            if (pressed) {
                if (keyUnlocked) {
                    if (pressStartMs == 0) { pressStartMs = nowMs; }
                    pressDuration = nowMs - pressStartMs;
                }
            } else {
                pressStartMs  = 0;
                pressDuration = 0;
                keyUnlocked   = true;
            }
            if (status && keyUnlocked && pressDuration >= 2000) {
                shutdown();
                return;
            }
        }
#endif
    }

    virtual void control(bool enable) {
#if (HAS_BATTERY == true)
        gpio_hold_dis((gpio_num_t)POWER_EN);
        gpio_set_level((gpio_num_t)POWER_EN, enable ? 1 : 0);
        if (enable) { gpio_hold_en((gpio_num_t)POWER_EN); }
        status = enable;
#endif
    }

    virtual void shutdown() {
#if (HAS_BATTERY == true)
        if (bootLockTimer) { esp_timer_stop(bootLockTimer); }
        if (onShutdown) { onShutdown(); }
        powerLocked = false;
        control(false);
        while (gpio_get_level((gpio_num_t)KEY_DOWN) == 0) { delay(20); }
        delay(100);
        for (;;) {
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
            gpio_wakeup_enable((gpio_num_t)KEY_DOWN, GPIO_INTR_LOW_LEVEL);
            esp_sleep_enable_gpio_wakeup();
            esp_light_sleep_start();
            if (gpio_get_level((gpio_num_t)KEY_DOWN) == 0) {
                delay(50);
                if (gpio_get_level((gpio_num_t)KEY_DOWN) == 0) { ESP.restart(); }
            }
        }
#endif
    }
};
