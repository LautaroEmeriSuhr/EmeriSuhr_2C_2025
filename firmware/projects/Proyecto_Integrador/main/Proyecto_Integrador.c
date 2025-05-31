/*! @mainpage Proyecto Integrador
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
 * | 23/05/2025 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_1_MS 1 // Periodo de 1  milisegundo
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

// Agregar la tarea dentro de un timer porque el sistema operativo tiene una ventana de 20 ms

const int pasos[4][4] = {
	{1, 1, 0, 0},
	{0, 1, 1, 0},
	{0, 0, 1, 1},
	{1, 0, 0, 1}};

const gpio_t lista[] = {
	GPIO_0, // A
	GPIO_1, // B
	GPIO_2, // C
	GPIO_3	// D
};

static void moverMotor(void *pvParameter)
{
	int indice_pasos = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notificación
		for (int i = 0; i < 4; i++)
		{
			GPIOState(lista[i], pasos[indice_pasos][i]);
		}
		indice_pasos = (indice_pasos + 1) % 4; // Incrementa el índice de pasos y lo reinicia si llega a 4
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	GPIOInit(GPIO_0, GPIO_OUTPUT);
	GPIOInit(GPIO_1, GPIO_OUTPUT);
	GPIOInit(GPIO_2, GPIO_OUTPUT);
	GPIOInit(GPIO_3, GPIO_OUTPUT);

	timer_config_t timer_motor = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_1_MS,
		.func_p = moverMotor,
		.param_p = NULL
	};

	TimerInit(&timer_motor); // Inicializa el timer con la configuración
	TimerStart(TIMER_A); // Inicia el timer

}
/*==================[end of file]============================================*/