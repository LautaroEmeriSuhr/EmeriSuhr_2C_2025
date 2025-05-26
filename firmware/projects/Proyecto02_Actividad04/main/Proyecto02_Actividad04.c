/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * Diseñar e implementar una aplicación, basada en el driver analog io mcu.y el driver 
 * de transmisión serie uart mcu.h, que digitalice una señal analógica y la transmita a
 * un graficador de puerto serie de la PC. Se debe tomar la entrada CH1 del conversor AD
 * y la transmisión se debe realizar por la UART conectada al puerto serie de la PC, en 
 * un formato compatible con un graficador por puerto serie. 
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
 * | 16/05/2025 | Document creation		                         |
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
TaskHandle_t tarea_lector_ecg_handle = NULL;	/* Handle de la tarea de lector ECG */

/*==================[internal data definition]===============================*/

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

TaskHandle_t main_task_handle = NULL;
const char ecg[] = {
	17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 17, 17, 16, 16, 16, 16, 17, 17, 18, 18, 18, 17, 17, 17, 17,
	18, 18, 19, 21, 22, 24, 25, 26, 27, 28, 29, 31, 32, 33, 34, 34, 35, 37, 38, 37, 34, 29, 24, 19, 15, 14, 15, 16, 17, 17, 17, 16, 15, 14, 13, 13, 13, 13, 13, 13, 13, 12, 12,
	10, 6, 2, 3, 15, 43, 88, 145, 199, 237, 252, 242, 211, 167, 117, 70, 35, 16, 14, 22, 32, 38, 37, 32, 27, 24, 24, 26, 27, 28, 28, 27, 28, 28, 30, 31, 31, 31, 32, 33, 34, 36,
	38, 39, 40, 41, 42, 43, 45, 47, 49, 51, 53, 55, 57, 60, 62, 65, 68, 71, 75, 79, 83, 87, 92, 97, 101, 106, 111, 116, 121, 125, 129, 133, 136, 138, 139, 140, 140, 139, 137,
	133, 129, 123, 117, 109, 101, 92, 84, 77, 70, 64, 58, 52, 47, 42, 39, 36, 34, 31, 30, 28, 27, 26, 25, 25, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25,
	24, 24, 24, 24, 24, 24, 24, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 20, 20, 20, 19, 19, 18, 18, 18, 19, 19, 19, 19, 18, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17,
	17, 17, 17};

void tarea_conversor_adc(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);				  /* Espera a que se le notifique */
		AnalogInputReadSingle(CH1, &adc_value);					  /* Lee el valor del ADC */
		UartSendString(UART_PC, ">ADCValue:");					  /* Envía el mensaje */
		UartSendString(UART_PC, (char *)UartItoa(adc_value, 10)); /* Envía el valor del ADC */
		UartSendString(UART_PC, "\r\n");						  /* Envía un salto de línea */
	}
}

void lector_ecg(void *pvParameter)
{
	uint16_t i = 0;		   /* Variable para recorrer el buffer */
	uint8_t ecg_value = 0; /* Variable para almacenar el valor del ECG */
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		ecg_value = ecg[i];			  /* Obtengo el valor del ECG */
		AnalogOutputWrite(ecg_value); // escribo el valor y lo convierto en analógico
		i++;
		if (i == sizeof(ecg))
		{
			i = 0;
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

	analog_input_config_t adc_config = {
		.input = CH1,		/*!< Inputs: CH0, CH1, CH2, CH3 */
		.mode = ADC_SINGLE, /*!< Mode: single read or continuous read */
		.func_p = NULL,		/*!< Pointer to callback function for convertion end (only for continuous mode) */
		.param_p = NULL,	/*!< Pointer to callback function parameters (only for continuous mode) */
		.sample_frec = 0	/*!< Sample frequency min: 20kHz - max: 2MHz (only for continuous mode)  */
	};

	AnalogInputInit(&adc_config); /* Inicializa el ADC */
	AnalogOutputInit();			  /* Inicializa el DAC */

	timer_config_t timer_1 = {
		.timer = TIMER_A,	  /*!< Selected timer */
		.period = 20000,	  /*!< Period (in us) */
		.func_p = FuncTimerA, /*!< Pointer to callback function to call periodically */
		.param_p = NULL		  /*!< Pointer to callback function parameter */
	};

	TimerInit(&timer_1); /* Inicializa el timer */

	timer_config_t timer_2 = {
		.timer = TIMER_B,	  /*!< Selected timer */
		.period = 25000,	  /*!< Period (in us) */
		.func_p = FuncTimerB, /*!< Pointer to callback function to call periodically */
		.param_p = NULL		  /*!< Pointer to callback function parameter */
	};

	TimerInit(&timer_2); /* Inicializa el timer */

	serial_config_t my_uart = {
		.port = UART_PC,	 /*!< port */
		.baud_rate = 115200, /*!< baudrate (bits per second) */
		.func_p = NULL,		 /*!< Pointer to callback function to call when receiving data (= UART_NO_INT if not requiered)*/
		.param_p = NULL		 /*!< Pointer to callback function parameters */
	};

	UartInit(&my_uart); /* Inicializa el UART */

	xTaskCreate(&tarea_conversor_adc, "tarea_adc", 512, NULL, 5, &tarea_conversor_adc_handle);
	xTaskCreate(lector_ecg, "lector_ecg", 2048, NULL, 1, &tarea_lector_ecg_handle);

	TimerStart(timer_1.timer); /* Inicia el timer */
	TimerStart(timer_2.timer); /* Inicia el timer */
}
/*==================[end of file]============================================*/