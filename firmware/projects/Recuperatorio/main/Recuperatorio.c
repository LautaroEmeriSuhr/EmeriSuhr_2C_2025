/*! @mainpage Recuperatorio
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 26/06/2025 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define LIMITE_MAX_AGUA 22
#define LIMITE_MIN_AGUA 28
volatile uint16_t distancia = 0;
/*==================[internal data definition]===============================*/
#define PIN_VALVULA_AGUA GPIO_10

/*==================[internal functions declaration]=========================*/
static void TareaAgua(void *pvParameters)
{
	while(true)
	{
		distancia = HcSr04ReadDistanceInCentimeters;
		if(distancia > LIMITE_MAX_AGUA)
		{
			GPIOState(PIN_VALVULA_AGUA, false);
		}
		if(distancia < LIMITE_MIN_AGUA)
		{
			GPIOState(PIN_VALVULA_AGUA, true);
		}


		vTaskDelay(2000); // Delay cada 2 segundos
	}
}

static void TareaComida(void *pvParameters)
{
	while(true)
	{

		vTaskDelay(5000); // Delay cada 5 segundos
	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/