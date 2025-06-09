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
 * | 	DATA	 	| 	GPIO_#		|
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
#include "uart_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define LIMITE_HUMEDAD 85
#define LIMITE_TEMPERATURA 2
#define LIMITE_RADIACION 1.32 // 1.32 = (3.3/100) * 40
#define LED_AMARILLO LED_2 
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

static void medir_Humedad_Temperatura(void *PvParameters)
{
	float *humedad, *temperatura;
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
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

static void medir_Radiacion(void *PvParameters)
{
	uint16_t radiacion = 0;
	uint16_t aux_radiacion = 0;
	while(true)
	{
		AnalogInputSingleRead(CH0, &radiacion);
		if(radiacion > LIMITE_RADIACION)
		{
			aux_radiacion = radiacion * (100 / 3.3);
			UartSendString(UART_PC, "Radiacion ");
			UartSendByte(UART_PC, (char *)UartItoa(aux_radiacion, 10));
			UartSendString(UART_PC, "mR/h - RADIACIÓN ELEVADA");
			LedOn(LED_AMARILLO);

		}else
		{
			aux_radiacion = radiacion * (100 / 3.3);
			UartSendString(UART_PC, "Radiacion ");
			UartSendByte(UART_PC, (char *)UartItoa(aux_radiacion, 10));
			UartSendString(UART_PC, "mR/h");
		}
	}

}

/*==================[external functions definition]==========================*/
void app_main(void){
	printf("Hello world!\n");
}
/*==================[end of file]============================================*/