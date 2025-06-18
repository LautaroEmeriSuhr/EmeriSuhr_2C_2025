/*! @mainpage Proyecto Integrador
 *
 * @section genDesc General Description
 *
 * La hipoglucemia se define como < 0.40 - 0.45 mg/ml en sangre
 * El GIR normal es de 5-7 mg/kg/min
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
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "timer_mcu.h"

#include "led.h"
#include "neopixel_stripe.h"
#include "ble_mcu.h"

/*==================[macros and definitions]=================================*/

#define PASOS_POR_VUELTA 2048								 // pasos // medir en el LAB
#define VOLUMEN_POR_VUELTA 0.2								 // mL 	  // medir en el LAB
#define PASOS_POR_ML (PASOS_POR_VUELTA / VOLUMEN_POR_VUELTA) // 20480 pasos/mL

#define PIN_A GPIO_19
#define PIN_B GPIO_20
#define PIN_C GPIO_21
#define PIN_D GPIO_22

#define PIN_VALVULA_GLUCOSA GPIO_15
#define PIN_VALVULA_H20 GPIO_17

#define CONFIG_BLINK_PERIOD 500
#define LED_BT LED_1

/*==================[internal data definition]===============================*/

const int pasos[4][4] =
	{
		{1, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 1},
		{1, 0, 0, 1}};

const gpio_t lista[] = {PIN_A, PIN_B, PIN_C, PIN_D};
TaskHandle_t motor_task_handle = NULL;
volatile uint32_t pasos_por_segundo = 0; // valor inicial de seguridad
volatile bool control_motor = false;
volatile uint32_t pasos_motor = 0;

static uint8_t caudal_valvulas_ml_s = 2; // MEDIDO EN MI CASA
volatile bool control_valvula = false;

volatile uint8_t concentracion_inicial_glucosa = 0; // en mg/ml
volatile uint16_t peso_bebe = 0;					// en kg
volatile uint8_t gir = 0;							// en mg/kg/min
volatile uint16_t volumen_final = 0;				// en ml
volatile uint8_t concentracion_final_glucosa = 0;	// en mg/ml
volatile int indice_pasos = 0;
volatile uint32_t indice_motor = 0;

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función que se invoca en la interrupción del timer
 */

void motorTimerCallback(void *pvParameter)
{
	vTaskNotifyGiveFromISR(motor_task_handle, pdFALSE);
}

/**
 * @brief Tarea encargada de mover el motor paso a paso
 */

static void moverMotor(void *pvParameter)
{
	int indice_pasos = 0;
	uint32_t indice_motor = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (control_motor)
		{
			if (indice_motor < pasos_motor)
			{
				indice_motor++;
				// Ejecutar un paso
				for (int i = 0; i < 4; i++)
				{
					GPIOState(lista[i], pasos[indice_pasos][i]);
				}
				indice_pasos = (indice_pasos + 1) % 4;
			}
			else
			{
				indice_motor = 0;
				control_motor = false;
			}
		}
	}
}

/**
 * @brief Función a ejecutarse ante un interrupción de recepción
 * a través de la conexión BLE.
 *
 * @param data      Puntero a array de datos recibidos
 * @param length    Longitud del array de datos recibidos
 */
void read_data(uint8_t *data, uint8_t length)
{
	uint8_t i = 1;
	if (data[0] == 'P')
	{
		concentracion_inicial_glucosa = 0;
		while (data[i] != 'I')
		{
			/* Convertir el valor ASCII a un valor entero */
			concentracion_inicial_glucosa = concentracion_inicial_glucosa * 10;
			concentracion_inicial_glucosa = concentracion_inicial_glucosa + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'K')
	{
		peso_bebe = 0;
		while (data[i] != 'B')
		{
			/* Convertir el valor ASCII a un valor entero */
			peso_bebe = peso_bebe * 10;
			peso_bebe = peso_bebe + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'G')
	{
		gir = 0;
		while (data[i] != 'I')
		{
			/* Convertir el valor ASCII a un valor entero */
			gir = gir * 10;
			gir = gir + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'V')
	{
		volumen_final = 0;
		while (data[i] != 'T')
		{
			/* Convertir el valor ASCII a un valor entero */
			volumen_final = volumen_final * 10;
			volumen_final = volumen_final + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'R')
	{
		concentracion_final_glucosa = 0;
		while (data[i] != 'F')
		{
			/* Convertir el valor ASCII a un valor entero */
			concentracion_final_glucosa = concentracion_final_glucosa * 10;
			concentracion_final_glucosa = concentracion_final_glucosa + (data[i] - '0');
			i++;
		}
	}

	if (data[0] == 'D')
	{
		control_valvula = true;
	}
}

/**
 * @brief Tarea encargada controlar la válvula
 */
static void prepararMezcla(void *pvParameter)
{
	uint8_t volumen_glucosa;
	uint8_t volumen_H2O;

	uint8_t tiempo_glucosa;
	uint8_t tiempo_H2O;
	while (true)
	{
		if (control_valvula)
		{

			volumen_glucosa = (concentracion_final_glucosa * volumen_final) / concentracion_inicial_glucosa;
			volumen_H2O = volumen_final - volumen_glucosa;

			tiempo_glucosa = volumen_glucosa / caudal_valvulas_ml_s;
			tiempo_H2O = volumen_H2O / caudal_valvulas_ml_s;

			control_valvula = false;

			TimerStop(TIMER_A);

			GPIOState(PIN_VALVULA_GLUCOSA, 1); // Activar
			vTaskDelay(pdMS_TO_TICKS(tiempo_glucosa * 1000));
			GPIOState(PIN_VALVULA_GLUCOSA, 0); // Desactivar

			GPIOState(PIN_VALVULA_H20, 1); // Activar
			vTaskDelay(pdMS_TO_TICKS(tiempo_H2O * 1000));
			GPIOState(PIN_VALVULA_H20, 0); // Desactivar

			pasos_motor = PASOS_POR_ML * volumen_final;
			control_motor = true;

		}

		vTaskDelay(pdMS_TO_TICKS(100)); // pequeña espera para no saturar
		TimerStart(TIMER_A);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	// Configuración de pines
	GPIOInit(PIN_A, GPIO_OUTPUT);
	GPIOInit(PIN_B, GPIO_OUTPUT);
	GPIOInit(PIN_C, GPIO_OUTPUT);
	GPIOInit(PIN_D, GPIO_OUTPUT);

	// Tarea del relé para probar en clase
	GPIOInit(PIN_VALVULA_GLUCOSA, GPIO_OUTPUT);
	GPIOInit(PIN_VALVULA_H20, GPIO_OUTPUT);

	static neopixel_color_t color;
	ble_config_t ble_configuration = {
		"ESP_EDU_LAUTARO",
		read_data};

	LedsInit();
	BleInit(&ble_configuration);

	// Crear tarea del control de las valvulas
	xTaskCreate(&prepararMezcla, "Preparar Mezcla", 4096, NULL, 5, NULL);

	xTaskCreate(&moverMotor, "Mover Motor", 2048, NULL, 5, &motor_task_handle);

	uint32_t caudal_ml_s = (gir * peso_bebe) / (concentracion_final_glucosa * 60);

	// Calcular frecuencia de pasos (pasos por segundo)
	pasos_por_segundo = caudal_ml_s * PASOS_POR_ML;

	printf(pasos_por_segundo);


	// Crear e iniciar timer
	timer_config_t timerMotor =
		{
			.timer = TIMER_A,
			.period = 10000,
			.func_p = motorTimerCallback,
			.param_p = NULL};

	TimerInit(&timerMotor);
	TimerStart(timerMotor.timer);

	while (1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		switch (BleStatus())
		{
		case BLE_OFF:
			LedOff(LED_BT);
			break;
		case BLE_DISCONNECTED:
			LedToggle(LED_BT);
			break;
		case BLE_CONNECTED:
			LedOn(LED_BT);
			break;
		}
	}
}
/*==================[end of file]============================================*/
