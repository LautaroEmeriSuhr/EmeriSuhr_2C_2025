/*! @mainpage Ejercicios 4, 5 y 6 de la Guia 1
 *
 * @section genDesc General Description
 *
 * El programa recibe un numero cualquiera luego separa cada uno de los digitos y los convierte a BCD. 
 * Para luego graficarlos en la pantalla lcd de 3 digitos.
 * 
 *
 * @section hardConn Hardware Connection
 *
 ** \brief Driver for using the 3 digits numeric display in ESP-EDU.
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
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 28/03/2025 | Document creation		                         |
 *
 * @author Lautaro Emeri Suhr
 *
 */

 /*==================[inclusions]=============================================/*/
 #include <stdio.h>
 #include <stdint.h>
 #include <gpio_mcu.h>
 
 /**
  * Este struct representa la configuracion de un pin GPIO
 */
 
 typedef struct
 {
	 gpio_t pin; /*!< GPIO pin number */
	 io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
 } gpioConf_t;
 
 
 
  /**
  * @brief int8_t convertToBcdArray
  * @param data numero a convertir a BCD
  * @param digits numero de digitos a convertir
  * @param bcd_number puntero a la variable donde se guardara el resultado
  * @note el resultado se guardara en el arreglo bcd_number de menor a mayor
  * @return 0 
  */
 int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 {
	 for (int i = 0; i < digits; i++)
	 {
		 bcd_number[digits - i - 1] = (data % 10);
		 data /= 10;
	 }
	 return 0;
 }
 
 /**
  * @fn void config_gpio
  * @brief Configura los pines GPIO para graficar el numero en BCD
  * @param gpio arreglo con los pines a configurar
  * @param bcd_number numero a convertir a BCD
  * @return 
  */
 void config_gpio(gpioConf_t *gpio, uint8_t bcd_number)
 {
	 for (int i = 0; i < 4; i++)
	 {
		 if ((1 << i) & bcd_number)
		 {
			 GPIOOn(gpio[i].pin);
		 }
		 else
		 {
			 GPIOOff(gpio[i].pin);
		 }
	 }
 }
 
 /**
  * @fn void Grafica_bcd
  * @brief Grafica el numero en BCD en la pantalla de 3 digitos
  * @param datos numero a graficar
  * @param digitos_salida numero de digitos a graficar
  * @param gpio_selector_pantalla puntero con el cual se selecciona la pantalla a utilizar
  * @param gpio arreglo con los pines a configurar  
  * @return 
  */
 
 void Grafica_bcd(uint32_t datos, uint8_t digitos_salida, gpioConf_t *gpio_selector_pantalla, gpioConf_t *gpio_bcd)
 {
	 uint8_t numero_por_partes[3] = {0};
	 convertToBcdArray(datos, digitos_salida, &numero_por_partes[0]);
 
	 for (int i = 0; i < 3; i++)
	 {
		 config_gpio(gpio_bcd, numero_por_partes[i]);
		 GPIOOn(gpio_selector_pantalla[i].pin);
		 GPIOOff(gpio_selector_pantalla[i].pin);
	 }
 }
 
 void app_main(void)
 {
 
	 uint8_t bcd_numer[5] = {0};
	 gpioConf_t gpio_bcd[4] =
		 {
			 {GPIO_20, GPIO_OUTPUT},
			 {GPIO_21, GPIO_OUTPUT},
			 {GPIO_22, GPIO_OUTPUT},
			 {GPIO_23, GPIO_OUTPUT}};
 
	 for (int i = 0; i < 4; i++)
	 {
		 GPIOInit(gpio_bcd[i].pin, gpio_bcd[i].dir);
	 }
 
	 gpioConf_t gpio_selector_pantalla[3] =
		 {
			 {GPIO_19, GPIO_OUTPUT},
			 {GPIO_18, GPIO_OUTPUT},
			 {GPIO_9, GPIO_OUTPUT}};
 
	 for (int i = 0; i < 3; i++)
	 {
		 GPIOInit(gpio_selector_pantalla[i].pin, gpio_selector_pantalla[i].dir);
	 }
 
	 Grafica_bcd(555, 3, &gpio_selector_pantalla[0], &gpio_bcd[0]);
 }