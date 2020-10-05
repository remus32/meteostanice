#include "main.h"
#include "ulp_main.h"

#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/adc.h"

#include "esp32/ulp.h"

extern const uint8_t bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t bin_end[]   asm("_binary_ulp_main_bin_end");

void ws_ulp_start() {
    ESP_ERROR_CHECK(ulp_load_binary(
        0, bin_start, (bin_end - bin_start) / sizeof(uint32_t)
    ));

    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_ulp_enable();

    esp_deep_sleep_disable_rom_logging();

    ESP_ERROR_CHECK(
        ulp_run(&ulp_ws_fff - RTC_SLOW_MEM)
    );
}
