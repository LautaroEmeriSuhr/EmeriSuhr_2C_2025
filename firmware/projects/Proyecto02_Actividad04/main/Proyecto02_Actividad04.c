/*! @mainpage Template
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
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr (lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <analog_io_mcu.h>
#include <uart_mcu.h>
#include <timer_mcu.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
/*==================[macros and definitions]=================================*/

TaskHandle_t tarea_conversor_adc_handle = NULL; /* Handle de la tarea de conversor ADC */
TaskHandle_t tarea_lector_ecg_handle = NULL; /* Handle de la tarea de lector ECG */

/*==================[internal data definition]===============================*/

analog_input_config_t adc_config = {
	.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
	.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
	.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
	.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
	.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
};

/*==================[internal functions declaration]=========================*/
void FuncTimerA(void *param)
{
	/* Función invocada en la interrupción del timer A */
	vTaskNotifyGiveFromISR(tarea_conversor_adc_handle, pdFALSE); /* Envía una notificación a la tarea asociada al conversor ADC */
}

void FuncTimerB(void *param)
{
	/* Función invocada en la interrupción del timer B */
	vTaskNotifyGiveFromISR(tarea_lector_ecg_handle, pdFALSE); /* Envía una notificación a la tarea asociada al conversor ADC */
}

uint16_t adc_value = 0; /* Variable para almacenar el valor del ADC */

#define BUFFER_SIZE 231

TaskHandle_t main_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};

void tarea_conversor_adc(void *pvParameter)
{
	while(true){
	ulTaskNotifyTake(pdTRUE, portMAX_DELAY);				  	/* Espera a que se le notifique */
	AnalogInputReadSingle(adc_config.input, &adc_value);	  	/* Lee el valor del ADC */
	UartSendString(UART_PC, ">ADCValue:");				  		/* Envía el mensaje */
	UartSendString(UART_PC, (char *)UartItoa(adc_value, 10));  	/* Envía el valor del ADC */
	UartSendString(UART_PC, "\r\n");						  	/* Envía un salto de línea */
	}
}

void lector_ecg(void *pvParameter)
{
	while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); /* Espera a que se le notifique */
		for(int i=0; i<BUFFER_SIZE; i++)
		{
			AnalogOutputWrite((uint8_t*) ecg[i]);	/* Envía el valor del ECG */
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	AnalogInputInit(&adc_config); /* Inicializa el ADC */

	timer_config_t timer_1 = {
		.timer = TIMER_A,	  /*!< Selected timer */
		.period = 20000,	  /*!< Period (in us) */
		.func_p = FuncTimerA, /*!< Pointer to callback function to call periodically */
		.param_p = NULL		  /*!< Pointer to callback function parameter */
	};

timer_config_t timer_2 = {
		.timer = TIMER_B,	  /*!< Selected timer */
		.period = 20000,	  /*!< Period (in us) */
		.func_p = FuncTimerB, /*!< Pointer to callback function to call periodically */
		.param_p = NULL		  /*!< Pointer to callback function parameter */
	};


	serial_config_t my_uart = {
		.port = UART_PC,	/*!< port */
		.baud_rate = 115200, /*!< baudrate (bits per second) */
		.func_p = NULL,		/*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL		/*!< Pointer to callback function parameters */
	};

	TimerInit(&timer_1); /* Inicializa el timer */
	UartInit(&my_uart);	 /* Inicializa el UART */

	TimerStart(timer_1.timer); /* Inicia el timer */
	TimerStart(timer_2.timer); /* Inicia el timer */

	xTaskCreate(tarea_conversor_adc,"tarea_conversor_adc", 2048, NULL, 1, &tarea_conversor_adc_handle);
	xTaskCreate(lector_ecg,"lector_ecg", 2048, NULL, 1, &tarea_lector_ecg_handle);
}
/*==================[end of file]============================================*/