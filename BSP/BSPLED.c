#include <stdlib.h>
#include "Common.h"
#include "MM32F103.h"
#include "BSPLED.h"


#define LEDn							1
										
#define LEDR_PIN						GPIO_Pin_9
#define LEDR_GPIO_PORT					GPIOB
#define LEDR_GPIO_CLK					RCC_APB2Periph_GPIOB


static uint16_t LEDFlashTime[LEDn] = {0};//��˸���ʱ��


/***************************************************
���ã�	LED�Ƴ�ʼ��.
������	LedNum��LED1��LED2
����:	��
***************************************************/
void BSPLEDInit(Led_Enum LedNum)
{
	GPIO_InitTypeDef  GPIOInitStructure;
	
	switch(LedNum)
	{
		case LEDR:
		{
			RCC_APB2PeriphClockCmd(LEDR_GPIO_CLK, ENABLE);
			
			GPIOInitStructure.GPIO_Pin  =  LEDR_PIN;
			GPIOInitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			GPIOInitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init(LEDR_GPIO_PORT, &GPIOInitStructure);
			break;
		}
	}
}

/***************************************************
���ã�	LED�ƴ�.
������	LedNum�� LED1��LED2
����:	��
***************************************************/
void BSPLEDOn(uint8_t LedNum) 
{
	switch(LedNum)
	{
		case LEDR:
		{	
			GPIO_ResetBits(LEDR_GPIO_PORT, LEDR_PIN);
			break;
		}
	}
}

/***************************************************
���ã�	LED�ƹر�.
������	LedNum�� LED1��LED2
����:	��
***************************************************/
void BSPLEDOff(uint8_t LedNum) 
{
	switch(LedNum)
	{
		case LEDR:
		{
			GPIO_SetBits(LEDR_GPIO_PORT, LEDR_PIN);
			break;
		}
	}
}

/***************************************************
���ã�	LED����˸��.
������	LedNum�� LED1��LED2
		time:��˸���,��λ:ms
����:	��
��ע:	ʹ��LED����˸������Ҫ��BSPLEDFlashRoutine()�������ʱ��.
***************************************************/
void BSPLEDFlashOn(Led_Enum LedNum, uint16_t Time)
{
	switch(LedNum)
	{
		case LEDR:
		{
			LEDFlashTime[LEDR] = Time;
			break;
		}
	}	
}

/***************************************************
���ã�	LED����˸�ر�.
������	LedNum�� LED1��LED2
����:	��
��ע:	ʹ��LED����˸������Ҫ��BSPLEDFlashRoutine()�������ʱ��.
***************************************************/
void BSPLEDFlashOff(Led_Enum LedNum)
{
	switch(LedNum)
	{
		case LEDR:
		{
			LEDFlashTime[LEDR] = 0;
			break;
		}
	}		
}

/***************************************************
���ã�	LED����������.
������	��
����:	��
***************************************************/
void BSPLEDFlashRoutine(void)
{
	static uint8_t ledFlag[LEDn] = {0};
	static uint16_t LEDTime = 0;
	Led_Enum LedNum;
	uint8_t i;
	
	if(LEDFlashTime[LEDR] > 0)
	{
		LEDTime++;
		for(i=0; i<LEDn; i++)
		{
			LedNum = (Led_Enum)i;
			if(LEDTime % (LEDFlashTime[LedNum]) == 0)
			{
				if(ledFlag[LedNum] == 1)
				{
					BSPLEDOn(LedNum);
				}
				else
				{
					BSPLEDOff(LedNum);
				}
				ledFlag[LedNum] = !ledFlag[LedNum];
			}
		}
	}
	else
	{
		LEDTime = 0;
	}
}

