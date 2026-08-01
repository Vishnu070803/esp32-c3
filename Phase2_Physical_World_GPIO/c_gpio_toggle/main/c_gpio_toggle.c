#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define GPIO_NUM_7_MASK (1 << 7)
#define GPIO_PIN (7)

#define GPIO_BASE_ADDR (0x60004000)
#define GPIO_FUNC7_OUT_SEL_CFG_REG ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x570))
#define GPIO_ENABLE_REG            ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x0020))
#define GPIO_ENABLE_W1TS_REG       ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x0024))
#define GPIO_ENABLE_W1TC_REG       ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x0028))
#define GPIO_OUT_REG               ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x0004))
#define GPIO_OUT_W1TS_REG          ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x0008))
#define GPIO_OUT_W1TC_REG          ((volatile uint32_t *)(GPIO_BASE_ADDR + 0x000C))

#define GPIO_FUNC_OEN_SEL (1 << 9)
#define GPIO_FUNC_OUT_SEL (0x80)

#define IOMUX_BASE_ADDR (0x60009000)
#define IO_MUX_GPIO7_REG ((volatile uint32_t *)(IOMUX_BASE_ADDR + 0x0020))


#define MCU_SEL (1 << 12)
#define FUN_DRV (2 << 10)
#define FUN_IE ~(1 << 9)
#define FUN_WPU ~(1 << 8)
#define FUN_WPD ~(1 << 7)
#define FILTER_EN ~(1 << 15)


void baremetal_way(){
	// 1. Configure the IO MUX (The Physical Switchboard)
	// MCU_SEL (Bit 12): Connect Pin 7 to the GPIO Matrix (Function 1), overriding JTAG/UART.
	// FUN_DRV (Bit 10): Set drive strength to ~20mA so the LED gets enough current.
	*(IO_MUX_GPIO7_REG) = (*(IO_MUX_GPIO7_REG) | MCU_SEL | FUN_DRV);
	
	// Disable unwanted electrical properties for a pure output pin:
	// FUN_WPU / FUN_WPD: Turn off internal Pull-Up and Pull-Down resistors.
	// FUN_IE: Turn off Input Enable (blinds the CPU from reading this pin's voltage).
	// FILTER_EN: Turn off the glitch filter (only used for physical inputs like buttons).
	*(IO_MUX_GPIO7_REG) = (*(IO_MUX_GPIO7_REG) & FUN_WPU & FUN_IE & FUN_WPD & FILTER_EN);

	// 2. Configure the GPIO Matrix (The Router)
	// GPIO_FUNC_OUT_SEL (0x80): Special index 128 forces the pin to listen to software (GPIO_OUT_REG) instead of hardware peripherals.
	// GPIO_FUNC_OEN_SEL (Bit 9): Forces the pin's input/output direction to be controlled by our software GPIO_ENABLE_REG.
	*(GPIO_FUNC7_OUT_SEL_CFG_REG) = (*(GPIO_FUNC7_OUT_SEL_CFG_REG) | GPIO_FUNC_OEN_SEL | GPIO_FUNC_OUT_SEL);

	// 3. Enable Output Direction (Open the Gate)
	// Note: W1TS (Write 1 To Set) is atomic. You could use `=` instead of `|=` here, because writing 0s is ignored by the hardware!
	*(GPIO_ENABLE_W1TS_REG) |= GPIO_NUM_7_MASK;
	
	while(1){
		vTaskDelay(pdMS_TO_TICKS(3000));
		
		// 4. Control the Voltage (The Generator)
		// W1TS atomically forces Bit 7 in the main GPIO_OUT_REG to 1 (Outputs 3.3V)
		*(GPIO_OUT_W1TS_REG) |= GPIO_NUM_7_MASK;
		printf("GPIO %d is high \n", GPIO_PIN);
		
		vTaskDelay(pdMS_TO_TICKS(3000));
		
		// W1TC atomically forces Bit 7 in the main GPIO_OUT_REG to 0 (Outputs 0V)
		*(GPIO_OUT_W1TC_REG) |= GPIO_NUM_7_MASK;
		printf("GPIO %d is low \n", GPIO_PIN);
	}
	return;
}

void default_way(){
			if(gpio_reset_pin(GPIO_NUM_7)){
			printf("Pin reset failed, exiting....");
			return;
		}
		if(gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT)){
			printf("Failed to set the gpio mode at pin %d, exiting", GPIO_NUM_7);
			return;
		}
	while(1){
		if(gpio_set_level(GPIO_NUM_7, 1)){
			printf("Failed to set high, exiting....");
			return;
		}
		printf("GPIO %d is high \n", GPIO_NUM_7);
		vTaskDelay(pdMS_TO_TICKS(3000));
		if(gpio_set_level(GPIO_NUM_7, 0)){
			printf("Failed to set low, exiting....");
			return;
		}
		printf("GPIO %d is low \n", GPIO_NUM_7);
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

void app_main(void)
{
	baremetal_way();
	return;
}
