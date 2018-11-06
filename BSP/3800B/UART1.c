#include <stdlib.h>
#include "MM32F103.h"
#include "Common.h"
#include "FIFO.h"
#include "BSP.h"


#define CALL_BACK_MAX			8
#define UART_WRITE_WAITTING		1


static void Null(void);


static FIFO Read;
static FIFO Write;
static uint16_t WriteDly = 0;
static CallBackISR ReadISR[CALL_BACK_MAX] = 
{
	Null,
	Null,
	Null,
	Null,
	Null,
	Null,
	Null,
	Null
};
static CallBackISR WriteISR[CALL_BACK_MAX] = 
{
	Null,
	Null,
	Null,
	Null,
	Null,
	Null,
	Null,
	Null
};


static void WriteData(void* DataTo, const void* DataFrom)
{
	*((uint8_t*)DataTo) = *((uint8_t*)DataFrom);
}

static void Null(void)
{
}

//*******************************************************************
//���ã�UART��ʼ��
//������BPS-������
//���أ���
//*******************************************************************
void UART1Init(uint32_t BPS)
{
	GPIO_InitTypeDef GPIOInit;
    UART_InitTypeDef USARTInit;
    NVIC_InitTypeDef NVICInit; 
	
	//��ʼ��USART2�ж����ȼ�
	NVICInit.NVIC_IRQChannel = UART1_IRQn;
	NVICInit.NVIC_IRQChannelPreemptionPriority = 0;
	NVICInit.NVIC_IRQChannelSubPriority = 0;
	NVICInit.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVICInit);

    //��ʼ��UART1�ܽ�	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
    GPIOInit.GPIO_Pin = GPIO_Pin_9;
	GPIOInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOInit.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIOInit);
    GPIOInit.GPIO_Pin = GPIO_Pin_10;
	GPIOInit.GPIO_Speed = GPIO_Speed_50MHz;
	GPIOInit.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIOInit);
	
	//��ʼ��UART1����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_UART1, ENABLE);
	USARTInit.UART_BaudRate = BPS;
	USARTInit.UART_WordLength = UART_WordLength_8b;
	USARTInit.UART_StopBits = UART_StopBits_1;
	USARTInit.UART_Parity = UART_Parity_No;
	USARTInit.UART_Mode = UART_Mode_Tx | UART_Mode_Rx;
	USARTInit.UART_HardwareFlowControl = UART_HardwareFlowControl_None;
	UART_Init(UART1, &USARTInit);
	UART_ITConfig(UART1, UART_IT_TXIEN, ENABLE);
	UART_ITConfig(UART1, UART_IT_RXIEN, ENABLE);

    //��ʼ��������
	Read = FIFOCreate(128, sizeof(uint8_t));
	FIFOCallbackWrite(Read, WriteData);
	Write = FIFOCreate(32, sizeof(uint8_t));
	FIFOCallbackWrite(Write, WriteData);
}

//*******************************************************************
//���ã�UART��Ӷ��жϳ���
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���������8��
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
//*******************************************************************
bool UART1AddReadISR(CallBackISR Function)
{
	static uint8_t count = 0;

    if(count < CALL_BACK_MAX)
    {
        ReadISR[count] = Function;
        ++count;
        return(TRUE);
    }
	return(FALSE);
}

//*******************************************************************
//���ã�UART���д�жϳ���
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���������8��
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
//*******************************************************************
bool UART1AddWriteISR(CallBackISR Function)
{
	static uint8_t count = 0;

    if(count < CALL_BACK_MAX)
    {
        WriteISR[count] = Function;
        ++count;
        return(TRUE);
    }
	return(FALSE);
}

//*******************************************************************
//���ã�UART��
//��������
//���أ���
//*******************************************************************
void UART1Open(void)
{
	UART_Cmd(UART1, ENABLE);
}

//*******************************************************************
//���ã�UART��
//��������
//���أ���
//*******************************************************************
void UART1Close(void)
{
	FIFOClear(Read);
	FIFOClear(Write);
	UART_Cmd(UART1, DISABLE);
}

//*******************************************************************
//���ã�UARTд����
//������Data-���͵�����
//���أ���
//*******************************************************************
void UART1Write(uint8_t Data)
{
	uint8_t tmp = Data;
	
	__disable_irq();
	if(WriteDly)
	{
	    __enable_irq();
		FIFOPush(Write, &tmp);			
	}
	else
	{
		if(UART_GetFlagStatus(UART1, UART_FLAG_TXEPT) == SET)
		{	
			UART_SendData(UART1, Data);
			__enable_irq();
			return;
		}
		__enable_irq();
		FIFOPush(Write, &tmp);	
	}
}

//*******************************************************************
//���ã���ѯUART�Ƿ������ݿ��Զ�ȡ
//��������
//���أ�TRUE-�����ݿɶ�
//      FALSE-�����ݿɶ�
//*******************************************************************
bool UART1AllowRead(void)
{
	if(FIFOCount(Read))
	{
		return(TRUE);
	}
	return(FALSE);
}

//*******************************************************************
//���ã�UART������
//��������
//���أ����յ�����
//*******************************************************************
uint8_t UART1Read(void)
{
	uint8_t tmp;
	
	FIFOPop(Read, &tmp);	
	return(tmp);
}

//*******************************************************************
//���ã�UART��ն�������
//��������
//���أ���
//*******************************************************************
void UART1BufClear(void)
{
	FIFOClear(Read);
	FIFOClear(Write);
}

//*******************************************************************
//���ã�UART����������
//��������
//���أ���
//*******************************************************************
void UART1Routine(void)
{
    uint8_t tmp;

	__disable_irq();
	if(WriteDly)
	{
		--WriteDly;
		__enable_irq();
	}
	else
	{
		if(FIFOCount(Write))
		{
			if(UART_GetFlagStatus(UART1, UART_FLAG_TXEPT) == SET)
			{
				FIFOPop(Write, &tmp);
				UART_SendData(UART1, tmp);
			}
		}
		__enable_irq();
	}
}

void UART1_IRQHandler(void)
{
	uint8_t tmp;
	uint8_t i = 0;
	
	if(UART_GetITStatus(UART1, UART_IT_TXIEN) == SET)
    {
		UART_ClearITPendingBit(UART1, UART_IT_TXIEN);
    	if(FIFOCount(Write))
    	{
	    	FIFOPop(Write, &tmp);
			UART_SendData(UART1, tmp);
	    }

    	while(i < CALL_BACK_MAX)
    	{
    		WriteISR[i]();
    		++i;
    	}
    }
	else if(UART_GetITStatus(UART1, UART_IT_RXIEN) == SET)
    {
		UART_ClearITPendingBit(UART1,UART_IT_RXIEN);
		tmp = UART_ReceiveData(UART1);
    	FIFOPush(Read, &tmp);
		WriteDly = UART_WRITE_WAITTING;

    	while(i < CALL_BACK_MAX)
    	{
    		ReadISR[i]();
    		++i;
    	}
    }
    return;
}





