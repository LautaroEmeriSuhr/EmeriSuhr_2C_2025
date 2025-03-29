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
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingieria.uner.edu.ar)
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/

struct leds
{
    uint8_t mode;        //ON, OFF, TOGGLE;
	uint8_t n_led;       // indica el número de led a controlar
	uint8_t n_ciclos;    //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds; 

void funcion_prueba(struct leds *my_leds)
{
	if (!led) return;  // Verifica que el puntero no sea NULL

    switch (led->mode) {
        case ON:
            set_led_state(led->n_led, ON);
            break;
        case OFF:
            set_led_state(led->n_led, OFF);
            break;
        case TOGGLE:
            for (uint8_t i = 0; i < led->n_ciclos; i++) {
                set_led_state(led->n_led, ON);
                usleep(led->periodo * 1000);  // Conversión a microsegundos
                set_led_state(led->n_led, OFF);
                usleep(led->periodo * 1000);
            }
            break;
        default:
            printf("Modo inválido\n");
    }
}
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();

    while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
    			LedToggle(LED_1);
    		break;
    		case SWITCH_2:
    			LedToggle(LED_2);
    		break;
    	}
	    LedToggle(LED_3);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}
