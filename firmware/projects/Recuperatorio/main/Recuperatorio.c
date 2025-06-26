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
 * |  Valvula Agua    |   ESP32   	  |
 * |:----------------:|:--------------|
 * | 	PIN_1	 	  | 	GPIO_10   |
 * |    GND           |     GND       |
 * 
 * |  Valvula Comida  |   ESP32   	  |
 * |:----------------:|:--------------|
 * | 	PIN_1	 	  | 	GPIO_23   |
 * |    GND           |     GND       |
 * 
 * |   HC_SR04      |   EDU-CIAA    |
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo	    | 	GPIO_3		|
 * | 	Trig	    | 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	|
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
#include "analog_io_mcu.h"
#include "uart_mcu.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define LIMITE_MAX_AGUA 22 	   // (30 - 7.96) si esta distancia disminuye yo quiero que se apague
#define LIMITE_MIN_AGUA 28 	   // (30 - 1.59) si esta distancia aumenta yo quiero que se prenda
#define LIMITE_MAX_COMIDA 665  // en mV
#define LIMITE_MIN_COMIDA 2150 // en mV
volatile uint16_t comida = 0;
volatile uint16_t agua = 0;
volatile bool on_off_agua = true;
volatile bool on_off_comida = true;
/*==================[internal data definition]===============================*/
#define PIN_VALVULA_AGUA GPIO_21
#define PIN_VALVULA_COMIDA GPIO_22
/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea que controla la válvula de agua según la distancia medida.
 * 
 * Lee la distancia con el sensor ultrasónico y calcula el volumen de agua.
 * Abre o cierra la válvula de agua según los límites definidos.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void TareaAgua(void *pvParameters)
{
	while (true)
	{
		if (on_off_agua)
		{
			uint16_t distancia = HcSr04ReadDistanceInCentimeters;
			agua = (3.14) * (10) * (10) * (30 - distancia);
			if (distancia > LIMITE_MIN_AGUA)   // distancia mayor implica volumen menor
			{
				GPIOState(PIN_VALVULA_AGUA, true);
			}
			if(distancia < LIMITE_MAX_AGUA)  // distancia menor implica volumen mayor
			{
				GPIOState(PIN_VALVULA_AGUA, false);
			}
		}
		vTaskDelay(2000); // Delay cada 2 segundos
	}
}

/**
 * @brief Tarea que controla la válvula de comida según el valor analógico leído.
 * 
 * Lee el valor del sensor de comida, calcula la cantidad y controla la válvula.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void TareaComida(void *pvParameters)
{
	while (true)
	{
		if (on_off_comida)
		{
			uint16_t comida_mV = 0;
			AnalogInputReadSingle(CH0, &comida_mV);

			comida = (comida_mV * 500) / 3300 - 500;

			if (comida_mV > LIMITE_MIN_COMIDA && comida_mV < LIMITE_MAX_COMIDA)
			{
				GPIOState(PIN_VALVULA_COMIDA, true);
			}
			else
			{
				GPIOState(PIN_VALVULA_COMIDA, false);
			}
		}
		vTaskDelay(5000); // Delay cada 5 segundos
	}
}

/**
 * @brief Tarea que informa por UART la cantidad de comida y agua.
 * 
 * Envía periódicamente el estado de comida y agua por UART.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void TareaInformar(void *pvParameters)
{
	while (true)
	{
		UartSendString(UART_PC, "Firulais tiene ");
		UartSendString(UART_PC, (char *)UartItoa(comida, 10));
		UartSendString(UART_PC, " g de alimento y ");
		UartSendString(UART_PC, (char *)UartItoa(agua, 10));
		UartSendString(UART_PC, " ml de agua.");
		
		vTaskDelay(10000); // Delay 10 segundos
	}
}

/**
 * @brief Alterna el estado de encendido/apagado de la comida.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void comidaToggle(void *pvParameters)
{
	on_off_comida =! on_off_comida; 
}

/**
 * @brief Alterna el estado de encendido/apagado del agua.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void aguaToggle(void *pvParameters)
{
	on_off_agua =! on_off_agua;
}

/**
 * @brief Función de callback para recepción por UART.
 * 
 * Cambia el estado de los toggles según la tecla recibida ('w' para agua, 'f' para comida).
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void FuncionTeclas(void *pvParameters)
{
	uint8_t letra = 0;	
	UartReadByte(UART_PC, &letra);	
	if(letra == 'w')
	{
		on_off_agua;
	}
	if(letra == 'f')
	{
		on_off_comida;
	}

}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	//Inicializaciones
	serial_config_t uart_config =
	{
		.port = UART_PC,
		.baud_rate =  115200, 
		.func_p = FuncionTeclas,
		.param_p = NULL 
	};
	UartInit(&uart_config);

	analog_input_config_t adc_config = 
	{ 
		.input = CH0, 
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p= NULL, 
		.sample_frec = 0
	};
    AnalogInputInit(&adc_config);

	SwitchesInit();

	//Tareas
	xTaskCreate(TareaAgua, "Tarea Agua", 2048, NULL, 5, NULL);
	xTaskCreate(TareaComida, "Tarea Comida", 2048, NULL, 5, NULL);
	xTaskCreate(TareaInformar, "Tarea Informar", 2048, NULL, 5, NULL);

	//Switches
	SwitchActivInt(SWITCH_1, on_off_agua, NULL);
	SwitchActivInt(SWITCH_2, on_off_comida, NULL);


}
/*==================[end of file]============================================*/