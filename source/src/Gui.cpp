#include <Arduino.h>
#include <esp_heap_caps.h>
#include <LovyanGFX.hpp>
#include <Config.hpp>

#include <Panel.hpp>
#include <Power.hpp>

#if (HAS_ST7789 == true)
volatile bool bootFinish = false;
#else
volatile bool bootFinish = true;
#endif

#if (HAS_ST7789 == true)

#ifndef LVGL_WIDTH
#define LVGL_WIDTH 240
#endif
#ifndef LVGL_HEIGHT
#define LVGL_HEIGHT 280
#endif

extern uint16_t          sensorRawData[IR_WIDTH * IR_HEIGHT];
extern SemaphoreHandle_t sensorUpdateSemaphore;

static void buildClassicLut(uint16_t lut[256]) {
    static const uint32_t classicRgb[256] = {
        0x35343D, 0x33333D, 0x31323E, 0x30303F, 0x2E2E40, 0x2C2C41, 0x2A2A41, 0x282843,
        0x252543, 0x232344, 0x202045, 0x1D1D46, 0x1A1A45, 0x181847, 0x151548, 0x131349,
        0x10104A, 0x0E0E4C, 0x0C0C4D, 0x0A0A4D, 0x08084D, 0x07074F, 0x060652, 0x050555,
        0x040559, 0x03045C, 0x020461, 0x010565, 0x01066A, 0x01086E, 0x000973, 0x000C78,
        0x000F7E, 0x001283, 0x001688, 0x001A8E, 0x001E93, 0x002398, 0x00289D, 0x002EA2,
        0x0034A7, 0x003AAB, 0x0041AF, 0x0047B4, 0x004EB8, 0x0056BD, 0x005DC1, 0x0063C4,
        0x006BC7, 0x0070CB, 0x0075CD, 0x007AD0, 0x0082D5, 0x008BDA, 0x0093DE, 0x009CE4,
        0x00A4E8, 0x00ADEC, 0x00B4F0, 0x00BCF4, 0x00C3F7, 0x00CAFA, 0x00D0FC, 0x00D6FE,
        0x00DBFF, 0x00E0FF, 0x00E4FF, 0x00E8FF, 0x00EAFF, 0x00EAFE, 0x00EAFB, 0x00EAF8,
        0x00EAF6, 0x00EAF2, 0x00EAEE, 0x00EAEA, 0x00EAE6, 0x00EAE1, 0x00EADC, 0x00EAD6,
        0x00E9D1, 0x00E7CB, 0x00E5C5, 0x00E3BE, 0x00E0B8, 0x00DEB2, 0x00DBAB, 0x00D8A5,
        0x00D7A0, 0x00D59C, 0x00D397, 0x00D193, 0x00CF8F, 0x00CD8A, 0x00CB86, 0x00C981,
        0x00C87D, 0x00C678, 0x00C474, 0x00C270, 0x00C16C, 0x00BF68, 0x00BE63, 0x00BC5F,
        0x00BB5C, 0x00BA58, 0x00B854, 0x00B850, 0x00B74D, 0x00B64A, 0x00B646, 0x00B544,
        0x00B441, 0x00B43E, 0x00B43C, 0x01B439, 0x03B437, 0x06B535, 0x09B533, 0x0CB630,
        0x0FB72E, 0x13B82C, 0x17B92B, 0x1BBB28, 0x1FBC27, 0x24BE25, 0x28BF23, 0x2DC222,
        0x32C421, 0x37C61F, 0x3CC81E, 0x42CA1C, 0x47CC1A, 0x4CCF19, 0x52D118, 0x58D317,
        0x5DD616, 0x63D815, 0x69DA14, 0x6EDD13, 0x74DF12, 0x7AE211, 0x80E410, 0x86E70F,
        0x8CE90E, 0x92EC0E, 0x97EE0D, 0x9EF00C, 0xA4F20B, 0xA9F40B, 0xAEF60A, 0xB4F709,
        0xB9F909, 0xBFFB08, 0xC5FC08, 0xC9FC07, 0xCEFC07, 0xD3FC06, 0xD8FC05, 0xDCFC05,
        0xE1FC04, 0xE5FC04, 0xE9FC03, 0xEDFC03, 0xF1FC03, 0xF4FC02, 0xF8FC02, 0xFAFC01,
        0xFDFC00, 0xFFFB00, 0xFFFA00, 0xFFF700, 0xFFF500, 0xFFF200, 0xFFEE00, 0xFFEB00,
        0xFFE800, 0xFFE300, 0xFFDE00, 0xFFDA00, 0xFFD500, 0xFFD000, 0xFFCB00, 0xFFC600,
        0xFFC000, 0xFFBB00, 0xFFB500, 0xFFAF00, 0xFFA900, 0xFFA300, 0xFF9C00, 0xFF9600,
        0xFF9000, 0xFF8900, 0xFF8300, 0xFF7C00, 0xFF7600, 0xFF6F00, 0xFF6900, 0xFF6300,
        0xFF5D00, 0xFF5600, 0xFF5100, 0xFF4A00, 0xFF4400, 0xFF3E00, 0xFF3900, 0xFF3300,
        0xFF2D00, 0xFF2900, 0xFF2400, 0xFF1F00, 0xFF1A00, 0xFF1600, 0xFF1200, 0xFF0E00,
        0xFF0A00, 0xFF0600, 0xFF0400, 0xFF0100, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
        0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0101, 0xFF0101, 0xFF0202, 0xFF0202,
        0xFF0303, 0xFF0505, 0xFF0606, 0xFF0808, 0xFF0B0B, 0xFF0E0E, 0xFF1212, 0xFF1717,
        0xFF1D1D, 0xFF2525, 0xFF2E2E, 0xFF3838, 0xFF4343, 0xFF5151, 0xFF5E5E, 0xFF6E6E,
        0xFF7E7E, 0xFF8E8E, 0xFF9F9F, 0xFFB0B0, 0xFFC1C1, 0xFFD1D1, 0xFFE0E0, 0xFFEDED
    };
    for (int i = 0; i < 256; i++) {
        uint32_t c = classicRgb[i];
        lut[i]     = Panel::inst().color565((uint8_t)((c >> 16) & 0xFF), (uint8_t)((c >> 8) & 0xFF), (uint8_t)(c & 0xFF));
    }
}

static void calcBSplineWeightsQ12(float t, uint16_t w[4]) {
    float t2 = t * t;
    float t3 = t2 * t;
    float u  = 1.0f - t;
    float f0 = (u * u * u) / 6.0f;
    float f1 = (3.0f * t3 - 6.0f * t2 + 4.0f) / 6.0f;
    float f2 = (-3.0f * t3 + 3.0f * t2 + 3.0f * t + 1.0f) / 6.0f;
    float f3 = t3 / 6.0f;
    w[0]     = (uint16_t)lroundf(f0 * 4096.0f);
    w[2]     = (uint16_t)lroundf(f2 * 4096.0f);
    w[3]     = (uint16_t)lroundf(f3 * 4096.0f);
    w[1]     = (uint16_t)(4096 - w[0] - w[2] - w[3]);
}

void panelTask(void*) {
    Panel::inst().init();
    Panel::inst().setRotation(1);
    Panel::inst().setBrightness(0);
    Panel::inst().setSwapBytes(true);

    extern bool sensorPause;
    sensorPause = false;

    Panel::inst().fillScreen(Panel::inst().color565(255, 255, 255));
#if (HAS_BATTERY == true)
    while (!Power::inst().powerLocked) { delay(20); }
#endif
    Panel::inst().setBrightness(50);
    Panel::inst().setFont(&fonts::efontCN_16_b);
    Panel::inst().setTextColor(0, 0xFFFFFF);

    uint32_t bootStartMs = millis();
    for (uint32_t i = 0;; i++) {
        switch (i % 4) {
        case 0:
            Panel::inst().drawString(" Booting    ", 105, 112);
            break;
        case 1:
            Panel::inst().drawString(" Booting·   ", 105, 112);
            break;
        case 2:
            Panel::inst().drawString(" Booting··  ", 105, 112);
            break;
        case 3:
            Panel::inst().drawString(" Booting··· ", 105, 112);
            break;
        default:
            Panel::inst().drawString(" Booting    ", 105, 112);
            break;
        }
        if (millis() - bootStartMs >= 2000) { break; }
        delay(500);
    }

    sensorPause = false;
    bootFinish  = true;

    static uint16_t lut[256];
    buildClassicLut(lut);

    const uint16_t bgColor = Panel::inst().color565(32, 32, 32);
    const uint16_t scrH    = Panel::inst().height();
    const uint16_t dstW    = 240;
    const uint16_t dstH    = 240;
    const uint16_t offX    = 0;
    const uint16_t offY    = (scrH - dstH) / 2;
    const uint16_t infoX   = offX + dstW + 2;

    Panel::inst().lock();
    Panel::inst().fillScreen(bgColor);
    Panel::inst().setFont(&fonts::Font2);
    Panel::inst().unlock();

    while (sensorUpdateSemaphore == nullptr) { delay(10); }

    static uint16_t localFrame[IR_WIDTH * IR_HEIGHT];
    static uint16_t tmpBuf[IR_HEIGHT * 240];
    uint16_t*       frameBuf = (uint16_t*)heap_caps_malloc((size_t)dstW * dstH * sizeof(uint16_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA);

    static uint8_t  xIdxTbl[240][4];
    static uint16_t wxTbl[240][4];
    static uint8_t  yIdxTbl[240][4];
    static uint16_t wyTbl[240][4];
    for (uint16_t dx = 0; dx < dstW; dx++) {
        float sx = (dstW > 1) ? ((float)dx * (IR_WIDTH - 1) / (dstW - 1)) : 0.0f;
        int   ix = (int)floorf(sx);
        float tx = sx - ix;
        for (int k = 0; k < 4; k++) {
            xIdxTbl[dx][k] = (uint8_t)constrain(ix + k - 1, 0, IR_WIDTH - 1);
        }
        calcBSplineWeightsQ12(tx, wxTbl[dx]);
    }
    for (uint16_t dy = 0; dy < dstH; dy++) {
        float sy = (dstH > 1) ? ((float)dy * (IR_HEIGHT - 1) / (dstH - 1)) : 0.0f;
        int   iy = (int)floorf(sy);
        float ty = sy - iy;
        for (int k = 0; k < 4; k++) {
            yIdxTbl[dy][k] = (uint8_t)constrain(iy + k - 1, 0, IR_HEIGHT - 1);
        }
        calcBSplineWeightsQ12(ty, wyTbl[dy]);
    }

    for (;;) {
#if (HAS_CST816S == true)
        uint16_t touchX = 0, touchY = 0;
        if (Panel::inst().getTouch(&touchX, &touchY)) {
            (void)touchX;
            (void)touchY;
        }
#endif
        if (xSemaphoreTake(sensorUpdateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
            memcpy(localFrame, sensorRawData, sizeof(localFrame));

            uint16_t minVal = localFrame[0];
            uint16_t maxVal = localFrame[0];
            for (int i = 1; i < IR_WIDTH * IR_HEIGHT; i++) {
                if (localFrame[i] < minVal) { minVal = localFrame[i]; }
                if (localFrame[i] > maxVal) { maxVal = localFrame[i]; }
            }
            uint16_t centerVal  = localFrame[(IR_HEIGHT / 2) * IR_WIDTH + (IR_WIDTH / 2)];
            uint32_t span       = (maxVal > minVal) ? (uint32_t)(maxVal - minVal) : 1u;
            uint32_t invSpanQ16 = (255u << 16) / span;

            if (frameBuf != nullptr) {
                for (int sy = 0; sy < IR_HEIGHT; sy++) {
                    const uint16_t* srcRow = &localFrame[sy * IR_WIDTH];
                    uint16_t*       tmpRow = &tmpBuf[sy * dstW];
                    for (uint16_t dx = 0; dx < dstW; dx++) {
                        const uint8_t*  xi = xIdxTbl[dx];
                        const uint16_t* wx = wxTbl[dx];
                        tmpRow[dx]         = (uint16_t)(((uint32_t)srcRow[xi[0]] * wx[0] + (uint32_t)srcRow[xi[1]] * wx[1] + (uint32_t)srcRow[xi[2]] * wx[2] + (uint32_t)srcRow[xi[3]] * wx[3] + 2048u) >> 12);
                    }
                }

                for (uint16_t dy = 0; dy < dstH; dy++) {
                    const uint16_t* r0  = &tmpBuf[(size_t)yIdxTbl[dy][0] * dstW];
                    const uint16_t* r1  = &tmpBuf[(size_t)yIdxTbl[dy][1] * dstW];
                    const uint16_t* r2  = &tmpBuf[(size_t)yIdxTbl[dy][2] * dstW];
                    const uint16_t* r3  = &tmpBuf[(size_t)yIdxTbl[dy][3] * dstW];
                    const uint16_t* wy  = wyTbl[dy];
                    uint32_t        wy0 = wy[0], wy1 = wy[1], wy2 = wy[2], wy3 = wy[3];
                    uint16_t*       dst = &frameBuf[(size_t)dy * dstW];

                    for (uint16_t dx = 0; dx < dstW; dx++) {
                        uint32_t val = ((uint32_t)r0[dx] * wy0 + (uint32_t)r1[dx] * wy1 + (uint32_t)r2[dx] * wy2 + (uint32_t)r3[dx] * wy3 + 2048u) >> 12;
                        int      idx = (val > minVal) ? (int)(((val - minVal) * invSpanQ16) >> 16) : 0;
                        if (idx > 255) { idx = 255; }
                        dst[dx] = lut[idx];
                    }
                }
            }

            Panel::inst().lock();
            if (frameBuf != nullptr) {
                Panel::inst().startWrite();
                Panel::inst().pushImageDMA(offX, offY, dstW, dstH, frameBuf);
                Panel::inst().waitDMA();
                Panel::inst().endWrite();
            }

            char info[24];
            Panel::inst().setTextColor(0xFFFF, bgColor);
            Panel::inst().drawString("MAX", infoX, 20);
            Panel::inst().fillRect(infoX, 36, 38, 16, bgColor);
            snprintf(info, sizeof(info), "%.1f", (float)maxVal / 10.0f - 273.15f);
            Panel::inst().drawString(info, infoX, 36);

            Panel::inst().drawString("MIN", infoX, 76);
            Panel::inst().fillRect(infoX, 92, 38, 16, bgColor);
            snprintf(info, sizeof(info), "%.1f", (float)minVal / 10.0f - 273.15f);
            Panel::inst().drawString(info, infoX, 92);

            Panel::inst().drawString("CTR", infoX, 132);
            Panel::inst().fillRect(infoX, 148, 38, 16, bgColor);
            snprintf(info, sizeof(info), "%.1f", (float)centerVal / 10.0f - 273.15f);
            Panel::inst().drawString(info, infoX, 148);

#if (HAS_BATTERY == true)
            Panel::inst().drawString("BAT", infoX, 188);
            Panel::inst().fillRect(infoX, 204, 38, 16, bgColor);
            snprintf(info, sizeof(info), "%.1fV", Power::inst().voltage);
            Panel::inst().drawString(info, infoX, 204);
#endif
            Panel::inst().unlock();
        }
    }
}

#endif
