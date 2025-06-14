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
#define PASOS_POR_VUELTA 2048
#define VOLUMEN_POR_VUELTA 0.2f								 // mL
#define PASOS_POR_ML (PASOS_POR_VUELTA / VOLUMEN_POR_VUELTA) // 10240 pasos/mL
#define SEGUNDOS_POR_HORA 3600

#define PIN_A GPIO_19
#define PIN_B GPIO_20
#define PIN_C GPIO_21
#define PIN_D GPIO_22

#define PIN_RELE GPIO_23

#define CONFIG_BLINK_PERIOD 500
#define LED_BT	LED_1

/*==================[internal data definition]===============================*/
const int pasos[4][4] =
	{
		{1, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 1, 1},
		{1, 0, 0, 1}};

const gpio_t lista[] = {PIN_A, PIN_B, PIN_C, PIN_D};
static float pasos_por_segundo = 1000; // valor inicial de seguridad
static TaskHandle_t motor_task_handle = NULL;

volatile uint8_t PORCENTAJE_INICIAL = 0;
volatile uint8_t PESO_BEBE = 0;
volatile uint8_t GIR = 0;
volatile uint8_t PORCENTAJE_FINAL = 0;

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
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		for (int i = 0; i < 4; i++)
		{
			GPIOState(lista[i], pasos[indice_pasos][i]);
		}
		indice_pasos = (indice_pasos + 1) % 4;
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
		PORCENTAJE_INICIAL = 0;
		while (data[i] != 'I')
		{
			/* Convertir el valor ASCII a un valor entero */
			PORCENTAJE_INICIAL = PORCENTAJE_INICIAL * 10;
			PORCENTAJE_INICIAL = PORCENTAJE_INICIAL + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'P')
	{
		PESO_BEBE = 0;
		while (data[i] != 'B')
		{
			/* Convertir el valor ASCII a un valor entero */
			PESO_BEBE = PESO_BEBE * 10;
			PESO_BEBE = PESO_BEBE + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'G')
	{
		GIR = 0;
		while (data[i] != 'I')
		{
			/* Convertir el valor ASCII a un valor entero */
			GIR = GIR * 10;
			GIR = GIR + (data[i] - '0');
			i++;
		}
	}
	if (data[0] == 'P')
	{
		PORCENTAJE_FINAL = 0;
		while (data[i] != 'F')
		{
			/* Convertir el valor ASCII a un valor entero */
			PORCENTAJE_FINAL = PORCENTAJE_FINAL * 10;
			PORCENTAJE_FINAL = PORCENTAJE_FINAL + (data[i] - '0');
			i++;
		}
	}
}

/**
 * @brief Tarea encargada controlar la válvula
 */
static void controlarValvula(void *pvParameter)
{
	while (true)
	{
		GPIOState(PIN_RELE, 0);			 // Activar válvula
		vTaskDelay(pdMS_TO_TICKS(1000)); // Mantener abierta por 1 segundo
		GPIOState(PIN_RELE, 1);			 // Desactivar válvula
		vTaskDelay(pdMS_TO_TICKS(1000)); // Esperar 1 segundo antes de la siguiente activación
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	/*
	// Entradas del enfermero
	float GIR, peso, concentracion;

	printf("Ingrese GIR (mg/kg/min): ");
	scanf("%f", &GIR);

	printf("Ingrese peso del paciente (kg): ");
	scanf("%f", &peso);

	printf("Ingrese concentracion de glucosa (mg/mL): ");
	scanf("%f", &concentracion);

	// Cálculo del caudal
	float caudal_ml_h = (GIR * peso * 60) / concentracion;
	float pasos_totales_h = caudal_ml_h * PASOS_POR_ML;
	pasos_por_segundo = pasos_totales_h / SEGUNDOS_POR_HORA;

	float periodo_ms = 1.0 / pasos_por_segundo;

	printf("\nCaudal: %.2f mL/h\n", caudal_ml_h);
	printf("Pasos por segundo: %.2f\n", pasos_por_segundo);
	printf("Periodo entre pasos: %.2f ms\n", periodo_ms);
	*/

	// float periodo_ms = 1000.0 / pasos_por_segundo; // Periodo en milisegundos
	/*
	// Configuración de pines
	GPIOInit(PIN_A, GPIO_OUTPUT);
	GPIOInit(PIN_B, GPIO_OUTPUT);
	GPIOInit(PIN_C, GPIO_OUTPUT);
	GPIOInit(PIN_D, GPIO_OUTPUT);

	// Crear e iniciar timer
	timer_config_t timerMotor =
		{
			.timer = TIMER_A,
			.period = 10000, // Periodo en microsegundos (100 us)
			.func_p = motorTimerCallback,
			.param_p = NULL};

	TimerInit(&timerMotor);
	TimerStart(timerMotor.timer);
	*/

	/*
		// Tarea del relé para probar en clase
	GPIOInit(PIN_RELE, GPIO_OUTPUT);
	// Crear tarea del relé
	xTaskCreate(&controlarValvula, "Controlar Válvula", 2048, NULL, 5, NULL);
	*/

	static neopixel_color_t color;
	ble_config_t ble_configuration = {
		"ESP_EDU_LAUTARO",
		read_data};

	LedsInit();
	BleInit(&ble_configuration);
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
