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
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
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
#define LIMITE_AGUA_LIMPIA 2  		// Cantidad minimia de agua limpia en Litros
#define LIMITE_AGUA_SUCIA  25 		// Cantidad maxima de agua sucia en Litros
#define SENSIBILIDAD_PRESION 730 	// Sensibilidad del sensor de presion (mV/Bar)	
#define LIMITE_PRESION 500 			// Presion minima en mBar
#define CAUDAL_DUCHA 750 			// Caudal de la manguera en mL/min
/*==================[internal data definition]===============================*/
volatile uint16_t aguaLimpia = 0;
volatile uint16_t aguaSucia = 0;
/*==================[internal functions declaration]=========================*/
void tareaAguaLimpia (void *pvParameters)
{
	while(true)
	{
		uint16_t distancia = HcSr04ReadDistanceInCentimeters;
		aguaLimpia = (30) * (30) * (30 - distancia); // en cm3
		aguaLimpia = aguaLimpia / 1000; // Convierto a Litros
		if(aguaLimpia > LIMITE_AGUA_LIMPIA) 
		{
	
		}
		vTaskDelay(500);
	}
}

void tareaAguaSucia (void *pvParameters)
{
	while(true)
	{
		uint16_t distancia = HcSr04ReadDistanceInCentimeters;
		aguaSucia = (30) * (30) * (30 - distancia); // en cm3
		aguaSucia = aguaSucia / 1000; // Convierto a Litros

		if(aguaSucia < LIMITE_AGUA_SUCIA)
		{

		}
		vTaskDelay(500);
	}
}

void tareaPresion (void *pvParameters)
{
	while(true)
	{
		uint16_t presion_mV = 0;
		AnalogInputReadSingle(CH0, &presion_mV);
		uint16_t presion = (presion_mV / SENSIBILIDAD_PRESION) / 1000 ; // Convierto Bar a mBar dvidiendo por 1000
		if(presion < 500)
		{
			//Prendo BOMBA_1
		}
		else
		{
			//Apago BOMBA_1
		}
		vTaskDelay(250);
	}
}
void tareaMostrarDatos (void *pvParameters)
{
	while(true)
	{
		uint16_t tiempo_restante = (aguaLimpia / CAUDAL_DUCHA * 1000); // (L / mL/min * 1000 ml/L) = min
		LcdItsE0803Write(tiempo_restante);
		vTaskDelay(500);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/