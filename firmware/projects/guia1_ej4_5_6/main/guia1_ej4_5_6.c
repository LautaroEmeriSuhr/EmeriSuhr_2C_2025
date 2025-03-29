/*! @mainpage Blinking switch
 *
 * \section genDesc General Description
 *
 * This example makes LED_1 and LED_2 blink if SWITCH_1 or SWITCH_2 are pressed.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"
#include "gpio_mcu.h"

typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

void config_GPIO(uint8_t bcd, gpioConf_t * vector_gpioConf)
{
	for (int i = 0; i < 4; i++)
	{
		if (bcd & (1 << i))
		{
			GPIOOn(vector_gpioConf[i].pin);
		}
		else
		{
			GPIOOff(vector_gpioConf[i].pin);
		}
	}
}

int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (int i = 0; i < digits; ++i)
	{
		bcd_number[digits - i - 1] = data % 10;
		data /= 10;
	}
	return 0;
}

void graficarDatos(uint32_t dato, uint8_t digitos, gpioConf_t *vectorGPIO1, gpioConf_t *vectorGPIO2)
{
	uint8_t auxiliar[3] = {0};
	convertToBcdArray(dato, digitos, &auxiliar[0]);

	for(int i=0; i<3; ++i)
	{
		config_GPIO(dato, &auxiliar[0]);
		GPIOOn(vectorGPIO1[i].pin);
		GPIOOff(vectorGPIO1[i].pin);
	}
}


/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t bcd_numer[5] = {0};

	convertToBcdArray(123, 3, &bcd_numer[0]);

	gpioConf_t gpio[4] =
		{
			{GPIO_20, GPIO_OUTPUT},
			{GPIO_21, GPIO_OUTPUT},
			{GPIO_22, GPIO_OUTPUT},
			{GPIO_23, GPIO_OUTPUT}};

	for (int i = 0; i < 4; i++)
	{
		GPIOInit(gpio[i].pin, gpio[i].dir);
	}
	gpioConf_t vectorGPIO2[3] =
{
	{GPIO_19, GPIO_OUTPUT},
	{GPIO_18, GPIO_OUTPUT},
	{GPIO_9, GPIO_OUTPUT}};

}
	uint32_t dato = 123;
	graficarDatos(123, 3, &vectorGPIO2[0], &gpio[0]);
	//config_GPIO(5, &gpio);
}
