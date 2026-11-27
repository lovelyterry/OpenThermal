#pragma once

#include <Config.hpp>

#if (HAS_ST7789 == true)

#include <LovyanGFX.hpp>
#include <Arduino.h>
#include <Config.hpp>

class Panel : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  panel;
    lgfx::Bus_SPI       bus;
    lgfx::Light_PWM     pwm;
#if (HAS_CST816S == true)
    lgfx::Touch_CST816S touch;
#endif
    SemaphoreHandle_t   xSemaphore = NULL;

public:
    static Panel& inst() {
        static Panel panel;
        return panel;
    }

    void lock() { xSemaphoreTake(xSemaphore, portMAX_DELAY); }
    void unlock() { xSemaphoreGive(xSemaphore); }

private:
    Panel(void) {
        xSemaphore = xSemaphoreCreateMutex();

        {
            auto cfg        = bus.config();
            cfg.spi_host    = SPI3_HOST;
            cfg.spi_mode    = 2;
            cfg.freq_write  = 80000000;
            cfg.freq_read   = 16000000;
            cfg.spi_3wire   = true;
            cfg.use_lock    = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk    = 13;
            cfg.pin_mosi    = 12;
            cfg.pin_miso    = -1;
            cfg.pin_dc      = 17;
            bus.config(cfg);
            panel.setBus(&bus);
        }

        {
            auto cfg             = panel.config();
            cfg.pin_cs           = 14;
            cfg.pin_rst          = 9;
            cfg.pin_busy         = -1;
            cfg.panel_width      = 240;
            cfg.panel_height     = 280;
            cfg.offset_x         = 0;
            cfg.offset_y         = 20;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = true;
            cfg.rgb_order        = true;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;
            panel.config(cfg);
        }

        {
            auto cfg        = pwm.config();
            cfg.pin_bl      = 18;
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            pwm.config(cfg);
            panel.setLight(&pwm);
        }

#if (HAS_CST816S == true)
        {
            auto cfg       = touch.config();
            cfg.x_min      = 0;
            cfg.x_max      = 239;
            cfg.y_min      = 0;
            cfg.y_max      = 279;
            cfg.pin_int    = 8;
            cfg.pin_rst    = -1;
            cfg.bus_shared = false;
            cfg.i2c_port   = 1;
            cfg.i2c_addr   = 0x15;
            cfg.pin_sda    = 10;
            cfg.pin_scl    = 11;
            cfg.freq       = 400000;
            touch.config(cfg);
            panel.setTouch(&touch);
        }
#endif

        setPanel(&panel);
    }
};

#endif