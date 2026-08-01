#include <stdio.h>
#include "driver/rmt_tx.h"
#include "driver/rmt_types.h"
#include "freertos/FreeRTOS.h"

// Define the electrical pulse timings for the WS2812 LED.
// Based on a 10MHz clock (1 tick = 100ns).
// WS2812 "1" Bit: High for 800ns, Low for 400ns
rmt_symbol_word_t set = {
    .duration0 = 8, .level0 = 1, 
    .duration1 = 4, .level1 = 0 
};

// WS2812 "0" Bit: High for 400ns, Low for 800ns
rmt_symbol_word_t clear = {
    .duration0 = 4, .level0 = 1, 
    .duration1 = 8, .level1 = 0 
};

void toggle_rgb_rmt(){
    // 1. Configure the RMT hardware routing blueprint
    rmt_tx_channel_config_t rmt_tx = {}; // ZERO-INITIALIZE to clear garbage flags (like DMA)
    rmt_tx.gpio_num = 2;                 // Physical pin 2
    rmt_tx.clk_src = RMT_CLK_SRC_DEFAULT;
    rmt_tx.resolution_hz = 10000000;     // 10MHz engine clock
    rmt_tx.mem_block_symbols = 48;       // Safely claim 48 symbols (1 channel). 64 consumes 2 channels!
    rmt_tx.trans_queue_depth = 4;        // FreeRTOS software queue size

    // 2. Claim the hardware engine from ESP-IDF
    rmt_channel_handle_t rmt_channel;
    rmt_new_tx_channel(&rmt_tx, &rmt_channel);

    // 3. Define our colors using 0xGGRRBB format to match WS2812 protocol
    uint32_t red_led_data   = 0x00ff00; // Green=0, Red=255, Blue=0
    uint32_t green_led_data = 0xff0000;
    uint32_t blue_led_data  = 0x0000ff;
    uint32_t full_led_data  = 0xffffff;

    // Allocate memory for the literal electrical pulse sequences
    rmt_symbol_word_t red_led[24];
    rmt_symbol_word_t green_led[24];
    rmt_symbol_word_t blue_led[24];
    rmt_symbol_word_t off_led[24];
    rmt_symbol_word_t full_led[24];

    // 4. Construct the physical payloads. 
    // We shift by (23 - i) to extract the Most Significant Bit first, exactly as WS2812 expects.
    for(int i = 0; i < 24; i++){
        red_led[i]   = (red_led_data >> (23 - i) & 1) ? set : clear;
        green_led[i] = (green_led_data >> (23 - i) & 1) ? set : clear;
        blue_led[i]  = (blue_led_data >> (23 - i) & 1) ? set : clear;
        full_led[i]  = (full_led_data >> (23 - i) & 1) ? set : clear;
        off_led[i]   = clear; // All zeroes = Black
    }

    // 5. Power on the RMT engine
    rmt_enable(rmt_channel);

    // 6. Create the Copy Encoder (moves our array into hardware RAM without changing it)
    rmt_copy_encoder_config_t copy_cfg = {};
    rmt_encoder_handle_t copy_encoder;
    rmt_new_copy_encoder(&copy_cfg, &copy_encoder);

    // 7. Fire the sequence (Loop count 0 means fire exactly once)
    rmt_transmit_config_t tx_config = { .loop_count = 0 };

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        rmt_transmit(rmt_channel, copy_encoder, red_led, sizeof(red_led), &tx_config);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        rmt_transmit(rmt_channel, copy_encoder, green_led, sizeof(green_led), &tx_config);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        rmt_transmit(rmt_channel, copy_encoder, blue_led, sizeof(blue_led), &tx_config);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        rmt_transmit(rmt_channel, copy_encoder, full_led, sizeof(full_led), &tx_config);
        
        vTaskDelay(pdMS_TO_TICKS(2000));
        rmt_transmit(rmt_channel, copy_encoder, off_led, sizeof(off_led), &tx_config);
    }
}

void app_main(void)
{
    toggle_rgb_rmt();
}
