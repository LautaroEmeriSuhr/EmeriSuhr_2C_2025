/*! @mainpage Final 29-07-25
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |   HC_SR04      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_3		|
 * | 	Trig	 	| 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	|
 * 
 * 
 * |   HC_SR04      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_13		|
 * | 	Trig	 	| 	GPIO_12		|
 * | 	Gnd 	    | 	GND     	|
 *
 * |   BOMBA_1      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	PIN_1 	    |	GPIO_16     |
 * | 	Gnd 		| 	GND 		|
 * 
 * |   BOMBA_2      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	PIN_1 	    |	GPIO_17     |
 * | 	Gnd 		| 	GND 		|
 * 
 * | SENSOR PRESION |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	PIN_1 	    |	CH0      	|
 * | 	Gnd 		| 	GND 		|
 * 
 * |   Display      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	BCD1		| 	GPIO_20		|
 * | 	BCD2	 	| 	GPIO_21		|
 * | 	BCD3	 	| 	GPIO_22		|
 * | 	BCD4	 	| 	GPIO_23		|
 * | 	SEL1	 	| 	GPIO_19		|
 * | 	SEL2	 	| 	GPIO_18		|
 * | 	SEL3	 	| 	GPIO_9		|
 * | 	Gnd 	    | 	GND     	|
 * 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 29/07/25   | Document creation		                         |
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
#include "lcditse0803.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define LIMITE_AGUA_LIMPIA 2	 // Cantidad minimia de agua limpia en Litros
#define LIMITE_AGUA_SUCIA 25	 // Cantidad maxima de agua sucia en Litros
#define SENSIBILIDAD_PRESION 730 // Sensibilidad del sensor de presion (mV/Bar)
#define LIMITE_PRESION 500		 // Presion minima en mBar
#define CAUDAL_DUCHA 750		 // Caudal de la manguera en mL/min
#define UMBRAL_PRESION 500  	// Umbral de presion en mBar
#define PIN_BOMBA_1 GPIO_16	 	// Pin de la bomba de agua limpia
#define PIN_BOMBA_2 GPIO_17	 	// Pin de la bomba de agua sucia

/*==================[internal data definition]===============================*/
volatile uint16_t aguaLimpia = 0;
volatile uint16_t aguaSucia = 0;
volatile bool controlAguaLimpia = false;
volatile bool controlAguaSucia = false;
volatile bool on_off_aspiracion = false; // El sistema comienza apagado
/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea que controla el agua limpia y sucia.
 * 
 * Esta tarea lee los sensores de distancia para calcular los volúmenes de agua limpia y sucia,
 * y actualiza los estados de control correspondientes.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void tareaAgua(void *pvParameters)
{
	while (true)
	{
		HcSr04Init(GPIO_2, GPIO_3); // Sensor de distancia 1	
		uint16_t distancia_1 = HcSr04ReadDistanceInCentimeters();
		aguaSucia = (30) * (30) * (30 - distancia_1); // en cm3
		aguaSucia = aguaSucia / 1000;				// Convierto a Litros
		if (aguaSucia < LIMITE_AGUA_SUCIA)
		{
			controlAguaSucia = true;
		}
		else
		{
			controlAguaSucia = false;
		}
		HcSr04Deinit();

		HcSr04Init(GPIO_12, GPIO_13); // Sensor de distancia 2
		uint16_t distancia_2 = HcSr04ReadDistanceInCentimeters();
		aguaLimpia = (30) * (30) * (30 - distancia_2); // en cm3
		aguaLimpia = aguaLimpia / 1000;				 // Convierto a Litros
		if (aguaLimpia > LIMITE_AGUA_LIMPIA)
		{
			controlAguaLimpia = true;
		}
		else
		{
			controlAguaLimpia = false;
		}
		HcSr04Deinit();

		vTaskDelay(500);
	}
}

/**
 * @brief Tarea que informa periódicamente los volúmenes de agua limpia y sucia.
 * 
 * Envía por UART la cantidad de agua limpia y sucia.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void TareaInformarVolumenes(void *pvParameters)
{
	while (true)
	{
		UartSendString(UART_PC, "Tanque de agua limpia ");
		UartSendString(UART_PC, (char *)UartItoa(aguaLimpia, 10));
		UartSendString(UART_PC, "litros.");
		UartSendString(UART_PC, "Tanque de agua sucia ");
		UartSendString(UART_PC, (char *)UartItoa(aguaSucia, 10));
		UartSendString(UART_PC, "litros.");

		vTaskDelay(500);
	}
}

/**
 * @brief Tarea que controla la presión del agua.
 * 
 * Lee el valor del sensor de presión y activa la bomba si la presión es baja.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */

static void tareaPresion(void *pvParameters)
{
	while (true)
	{
		uint16_t presion_mV = 0;
		AnalogInputReadSingle(CH0, &presion_mV);
		uint16_t presion = (presion_mV / SENSIBILIDAD_PRESION) / 1000; // Convierto Bar a mBar dvidiendo por 1000
		if (presion < UMBRAL_PRESION && controlAguaLimpia)
		{
			GPIOState(PIN_BOMBA_1, true);
		}
		else
		{
			GPIOState(PIN_BOMBA_1, false);
		}
		vTaskDelay(250);
	}
}

/**
 * @brief Tarea que controla la aspiración de agua sucia.
 * 
 * Activa o desactiva la bomba de aspiración según el estado del sistema.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void tareaAspiracion(void *pvParameters)
{
	while (true)
	{
		if (on_off_aspiracion && controlAguaSucia)
		{
			GPIOState(PIN_BOMBA_2, true);
		}
		else
		{
			GPIOState(PIN_BOMBA_2, false);
		}
		vTaskDelay(500);
	}
}

/**
 * @brief Tarea que muestra los datos en el LCD.
 * 
 * Muestra el tiempo restante de agua limpia y el estado de las luces LED.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void tareaMostrarDatos(void *pvParameters)
{
	while (true)
	{
		uint16_t tiempo_restante = (aguaLimpia / CAUDAL_DUCHA * 1000); // (L / mL/min * 1000 ml/L) = min
		LcdItsE0803Write(tiempo_restante);
		if (aguaLimpia < LIMITE_AGUA_LIMPIA)
		{
			LedOn(LED_1);
			LedOff(LED_2);
			LedOff(LED_3);
		}
		if (aguaSucia > LIMITE_AGUA_SUCIA)
		{
			LedOn(LED_2);
			LedOff(LED_1);
			LedOff(LED_3);
		}
		if (aguaLimpia >= LIMITE_AGUA_LIMPIA && aguaSucia <= LIMITE_AGUA_SUCIA)
		{
			LedOn(LED_3);
			LedOff(LED_1);
			LedOff(LED_2);
		}
		vTaskDelay(500);
	}
}

/**
 * @brief Alterna el estado de encendido/apagado de la aspiración.
 * 
 * Esta función se utiliza como callback para el switch que controla la aspiración.
 * 
 * @param pvParameters Parámetro de la tarea (no utilizado).
 */
static void on_off(void *pvParameters)
{
	on_off_aspiracion = !on_off_aspiracion;
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	//Inicializaciones
	serial_config_t uart_config =
	{
		.port = UART_PC,
		.baud_rate =  115200, 
		.func_p = NULL,
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

	LedsInit();
	SwitchesInit();
	LcdItsE0803Init();
	GPIOInit(PIN_BOMBA_1, GPIO_OUTPUT);
	GPIOInit(PIN_BOMBA_2, GPIO_OUTPUT);
	
	// Tareas
	xTaskCreate(tareaAgua, "Tarea Agua", 2048, NULL, 5, NULL);
	xTaskCreate(TareaInformarVolumenes, "Tarea Informar Volumenes", 2048, NULL, 5, NULL);
	xTaskCreate(tareaPresion, "Tarea Presion", 2048, NULL, 5, NULL);
	xTaskCreate(tareaAspiracion, "Tarea Aspiracion", 2048, NULL, 5, NULL);
	xTaskCreate(tareaMostrarDatos, "Tarea Mostrar Datos", 2048, NULL, 5, NULL);

	// Switches
	SwitchActivInt(SWITCH_1, on_off, NULL);
}
/*==================[end of file]============================================*/