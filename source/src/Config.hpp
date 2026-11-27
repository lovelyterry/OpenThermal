#pragma once

#define INFRARED_MINI 0
#define INFRARED_PRO  1

#define PRODUCT         INFRARED_PRO
#define PRODUCT_VERSION "1.0.0"
#define MAX_CLIENTS     5

#if (PRODUCT == INFRARED_MINI)
#define PRODUCT_NAME "Infrared-mini"

#define HAS_CST816S false
#define HAS_ST7789  false
#define HAS_BATTERY false
#if (HAS_BATTERY == true)
#define KEY_DOWN -1
#define POWER_EN -1
#define ADC_BAT  -1
#endif
#define HAS_HTPA32X32 true
#if (HAS_HTPA32X32 == true)
#define HTPA_SCL 9
#define HTPA_SDA 8
#endif
#endif

#if (PRODUCT == INFRARED_PRO)
#define PRODUCT_NAME "Infrared-pro"

#define HAS_CST816S true
#define HAS_ST7789  true
#define HAS_BATTERY true
#if (HAS_BATTERY == true)
#define KEY_DOWN 37
#define POWER_EN 36
#define ADC_BAT  4
#endif
#define HAS_HTPA32X32 true
#if (HAS_HTPA32X32 == true)
#define HTPA_SCL 7
#define HTPA_SDA 6
#endif
#endif

#if (HAS_HTPA32X32 == true)
#define IR_WIDTH  (uint16_t)(32)
#define IR_HEIGHT (uint16_t)(32)
#endif
