/*! @mainpage Actividad 1 Proyecto 2
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn HC-SR04 Connection
 *
 * |   HC_SR04      |   EDU-CIAA	|
 * |:--------------:|:-------------:|
 * | 	Vcc 	    |	5V      	|
 * | 	Echo		| 	GPIO_3		|
 * | 	Trig	 	| 	GPIO_2		|
 * | 	Gnd 	    | 	GND     	|
 * 
 * @section hardconn LCD-EDU-ESP Connection
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
 * | 11/04/2025 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <hc_sr04.h>
#include <led.h>
#include <lcditse0803.h>
#include <switch.h>
/*==================[macros and definitions]=================================*/
#define Delay_time 1000	/* Tiempo de espera en milisegundos */

/*==================[internal data definition]===============================*/

bool on_off = true;	/* Variable para el estado del LED */
bool hold = false;	/* Variable para el estado de hold */

uint16_t distancia = 0;
uint8_t teclas;

TaskHandle_t medir_distancia_handle = NULL;	/* Handle para la tarea de medir distancia */
TaskHandle_t leer_teclas_handle = NULL;	/* Handle para la tarea de leer teclas */

/*==================[internal functions declaration]=========================*/

static void medir_distancia(void *pvParameter)
{
	while(true){
	if(on_off)
	{
		distancia = HcSr04ReadDistanceInCentimeters();	/* Lee la distancia en cm */
		if (distancia<10)
		{
			LedOff(LED_1);	/* Apaga el LED_1 */
			LedOff(LED_2);	/* Apaga el LED_2 */
			LedOff(LED_3);	/* Apaga el LED_3 */
		}
		if (distancia >=10 && distancia < 20)
		{
			LedOn(LED_1);	/* Enciende el LED_1 */
			LedOff(LED_2);	/* Apaga el LED_2 */
			LedOff(LED_3);	/* Apaga el LED_3 */
		}
		if (distancia >=20 && distancia < 30)
		{
			LedOn(LED_1);	/* Enciende el LED_1 */
			LedOn(LED_2);	/* Enciende el LED_2 */
			LedOff(LED_3);	/* Apaga el LED_3 */
		}
		if (distancia >= 30)
		{
			LedOn(LED_1);	/* Enciende el LED_1 */
			LedOn(LED_2);	/* Enciende el LED_2 */
			LedOn(LED_3);	/* Enciende el LED_3 */
		}
		if(!hold)
		{
		LcdItsE0803Write(distancia);	/* Muestra la distancia en el LCD */	
		}
		else 
		{
			LcdItsE0803Off(); /* Apaga el LCD */
			LedsOffAll();	/* Apaga todos los LEDs */
		}
	}
	vTaskDelay(Delay_time / portTICK_PERIOD_MS);	/* Espera 1 segundo */
}
}


static void leer_teclas(void *pvParameter)
{
	while(true)
	{
		teclas = SwitchesRead();	/* Lee el estado de los switches */
		switch(teclas){
    		case SWITCH_1:
    			on_off = !on_off;	/* Cambia el estado de on_off */
    		break;
    		case SWITCH_2:
    			hold = !hold;	/* Cambia el estado de hold */
    		break;
    	}
		vTaskDelay(Delay_time / portTICK_PERIOD_MS);	/* Espera 1 segundo */
	}
}


/*==================[external functions definition]==========================*/
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);	/* Inicializa el HC-SR04 */
	LcdItsE0803Init();	/* Inicializa el LCD */
	LedsInit();	/* Inicializa los LEDs */
	SwitchesInit();	/* Inicializa los switches */

	xTaskCreate(medir_distancia, "medir_distancia", 2048, NULL, 1, &medir_distancia_handle);	/* Crea la tarea de medir distancia */
	xTaskCreate(leer_teclas, "leer_teclas", 2048, NULL, 1, &leer_teclas_handle);	/* Crea la tarea de leer teclas */

	
}
/*==================[end of file]============================================*/