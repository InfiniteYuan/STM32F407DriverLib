/*
*********************************************************************************************************
*
*                                      GPIO CALSS HEADER FILE
*
* Filename      : GPIO.h
* Version       : V1.0
* Programmer(s) : Jason Xie
*
*********************************************************************************************************
*/
#ifndef _GPIO_H
#define _GPIO_H

/**	include header files	**/

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
//#include <misc.h>
//#include <stdbool.h>


/**
  *	@brief  The class of GPIO.
  *	@param  mPin[16]: The array of GPIO_Pin_x.
  *	@param  mPort:The port of GPIO,such as GPIOA,etc.
  * @param  mSelectPin:The pin you want to use.
  * @param  mMode:The GPIO's mode,such as "GPIO_Mode_Out_PP",etc
  * @param  mSpeed:The output's speed of GPIO,such as "GPIO_Speed_10MHz",etc
  */
class GPIO
{
private:
	static const  uint16_t     	mPin[16];
	GPIOMode_TypeDef			  mMode;
	GPIOSpeed_TypeDef 			mSpeed;

	/*	The function to configure the RCC of GPIO	*/
	void RCC_Configuration();

public:
	GPIO_TypeDef				*mPort;
	uint16_t					mSelectPin;

	/*	The Constructor of the Class	*/
	GPIO(GPIO_TypeDef *port = GPIOA, uint16_t pin = 1, GPIOMode_TypeDef mode = GPIO_Mode_AF, GPIOOType_TypeDef outtype = GPIO_OType_PP, GPIOPuPd_TypeDef pupd = GPIO_PuPd_UP, GPIOSpeed_TypeDef speed = GPIO_Speed_50MHz);

	/**
	*	@brief  Set the pin as high level or low level.
	*	@param  level: The level you wan to set,type:unsigned char ,value:0 or 1
	*/
	void SetLevel(u8 level);

	/**
	*	@brief  Get the level of the pin.
	*	@retval level:Return the pin's level,value:true(1),false(0)
	*/
	bool GetLevel();

	/**
	*	@brief  Invert the level of the pin.
	*/
	void Toggle();

	/**
	*	@brief  Change the GPIO's mode.
	*	@param  the GPIO's mode
	*/
	void ChangeMode(GPIOMode_TypeDef mode, GPIOOType_TypeDef outtype, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed);
};


#endif

