/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 *
 * |   	DHT11		|   ESP-EDU 	|
 * |:--------------:|:--------------|
 * | 	VCC     	|	3V3     	|
 * | 	DATA	 	| 	GPIO 20		|
 * | 	GND		 	| 	GND 		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/06/2025 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr(lautaro.emeri@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "dht11.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define LIMITE_HUMEDAD 85
#define LIMITE_TEMPERATURA 2
#define LIMITE_RADIACION 1.32 // 1.32 = (3.3/100) * 40
#define LED_ROJO LED_3
#define LED_AMARILLO LED_2
#define LED_VERDE LED_1
#define GPIO_DHT11 GPIO_20 
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

static void medir_Humedad_Temperatura_Radiacion(void *PvParameters)
{
	float *humedad, *temperatura;
	uint16_t radiacion = 0;
	uint16_t aux_radiacion = 0;
	bool control1 = false;
	bool control2 = false;
	while(true)
	{
		if(dht11Read(&humedad,&temperatura))
		{
			if(humedad < LIMITE_HUMEDAD && temperatura > LIMITE_TEMPERATURA)
			{
				UartSendString(UART_PC,"Temperatura: ");
				UartSendByte(UART_PC,(char *)UartItoa(temperatura, 10));
				UartSendString(UART_PC,"°C - ");
				UartSendString(UART_PC,"Humedad: ");
				UartSendByte(UART_PC,(char *)UartItoa(humedad, 10));
				UartSendString(UART_PC,"%\r\n");
				control1 = true;
			}
			if(humedad >= LIMITE_HUMEDAD && temperatura < LIMITE_TEMPERATURA)
			{
				UartSendString(UART_PC,"Temperatura: ");
				UartSendByte(UART_PC,(char *)UartItoa(temperatura, 10));
				UartSendString(UART_PC,"°C - ");
				UartSendString(UART_PC,"Humedad: ");
				UartSendByte(UART_PC,(char *)UartItoa(humedad, 10));
				UartSendString(UART_PC,"% - ");	
				UartSendString(UART_PC, "RIESGO DE NEVADA \r\n");
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}

		AnalogInputSingleRead(CH0, &radiacion);
		if(radiacion > LIMITE_RADIACION)
		{
			aux_radiacion = radiacion * (100 / 3.3);
			UartSendString(UART_PC, "Radiacion ");
			UartSendByte(UART_PC, (char *)UartItoa(aux_radiacion, 10));
			UartSendString(UART_PC, "mR/h - RADIACIÓN ELEVADA");
			LedOn(LED_AMARILLO);
			LedOff(LED_VERDE);
			LedOff(LED_ROJO);

		}else
		{
			aux_radiacion = radiacion * (100 / 3.3);
			UartSendString(UART_PC, "Radiacion ");
			UartSendByte(UART_PC, (char *)UartItoa(aux_radiacion, 10));
			UartSendString(UART_PC, "mR/h");
			control2 = true;
		}
		vTaskDelay(pdMS_TO_TICKS(5000));

		if(control1 && control2)
		{
			LedOn(LED_VERDE);
			LedOff(LED_AMARILLO);
			LedOff(LED_ROJO);
			control1 = false;
			control2 = false;
		}

	}
}

/*==================[external functions definition]==========================*/
void app_main(void){
	analog_input_config_t adc_config =
	{
		.input = CH0,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
	};
	AnalogInputInit(&adc_config);

	serial_config_t my_uart = 
	{
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL
	};
	UartInit(&my_uart);

	dht11Init(GPIO_DHT11);
	LedsInit();


	xTaskCreate(medir_Humedad_Temperatura_Radiacion, "Medir Humedad y Temperatura", 2048, NULL, 5, NULL);


}
/*==================[end of file]============================================*/