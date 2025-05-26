/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Cree un nuevo proyecto en el que modifique la actividad del punto 2 agregando ahora
 * el puerto serie. Envíe los datos de las mediciones para poder observarlos en un terminal 
 * en la PC, siguiendo el siguiente formato:
 * 
 * 3 dígitos ascii + 1 carácter espacio + dos caracteres para la unidad (cm) + cambio de línea 
 * “ \r\n”
 * Además debe ser posible controlar la EDU-ESP de la siguiente manera:
 * Con las teclas “O” y “H”, replicar la funcionalidad de las teclas 1 y 2 de la EDU-ESP
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
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/04/2025 | Document creation		                         |
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
#include <timer_mcu.h>
#include <uart_mcu.h>

/*==================[macros and definitions]=================================*/

#define Delay_time 1000	/* Tiempo de espera en milisegundos */
#define CONFIG_BLINK_PERIOD_LED_1_US 1000000 /* Periodo del timer del LED_1 en microsegundos */

/*==================[internal data definition]===============================*/

bool on_off = true;	/* Variable para el estado del LED */
bool hold = false;	/* Variable para el estado de hold */

uint16_t distancia = 0; /* Variable para almacenar la distancia medida */
uint8_t teclas; 	/* Variable para almacenar el estado de los switches */

TaskHandle_t medir_distancia_handle = NULL;	/* Handle para la tarea de medir distancia */
TaskHandle_t leer_teclas_handle = NULL;	/* Handle para la tarea de leer teclas */

/*==================[external functions definition]==========================*/

static void medir_distancia(void *pvParameter) 	/* Tarea que mide la distancia */
{
	while(true){
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);	/* Espera a que se envíe una notificación */

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
	
	UartSendString(UART_PC, (char*) UartItoa(distancia,10));	/* Envía la distancia por UART */
	UartSendString(UART_PC, " ");	/* Envía un espacio */
	UartSendString(UART_PC, "cm");	/* Envía la unidad en la que estoy midiendo*/
	UartSendString(UART_PC, "\r\n");	/* Envía un salto de línea */
}
}

void On_Off() 	/* Función que cambia el estado de on_off */
{
	on_off = !on_off;	/* Cambia el estado de on_off */
}

void Hold() 	/* Función que cambia el estado de hold */
{
	hold = !hold;	/* Cambia el estado de hold */
}

void FuncTimerA(void* param) /* Función invocada en la interrupción del timer A */
{
	vTaskNotifyGiveFromISR(medir_distancia_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

void FuncUART(void *param) /* Función que se ejecuta cuando se recibe un byte por UART */
{
	uint8_t letra = 0;	/* Variable para almacenar la letra leída */
	UartReadByte(UART_PC, &letra);	/* Lee un byte del UART */
	if(letra == 'O')
	{
		Hold();	/* Cambia el estado de hold */
	}
	if(letra == 'H')
	{
		On_Off();	/* Cambia el estado de on_off */
	}

}
void app_main(void){

	HcSr04Init(GPIO_3, GPIO_2);	/* Inicializa el HC-SR04 */
	LcdItsE0803Init();	/* Inicializa el LCD */
	LedsInit();	/* Inicializa los LEDs */
	SwitchesInit();	/* Inicializa los switches */

	timer_config_t timer_led_1 = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_LED_1_US,
        .func_p = FuncTimerA,
        .param_p = NULL
    };

	serial_config_t my_uart = {
		.port = UART_PC,
		.baud_rate = 9600,
		.func_p = FuncUART,
		.param_p = NULL
	};

	UartInit(&my_uart);	/* Inicializa el UART */

	TimerInit(&timer_led_1);	/* Inicializa el timer del LED_1 */

	SwitchActivInt(SWITCH_1, On_Off, NULL);	/* Activa la interrupción del switch 1 */
	SwitchActivInt(SWITCH_2, Hold, NULL);	/* Activa la interrupción del switch 2 */

	TimerStart(timer_led_1.timer);	/* Inicia el timer del LED_1 */
} 
/*==================[end of file]============================================*/