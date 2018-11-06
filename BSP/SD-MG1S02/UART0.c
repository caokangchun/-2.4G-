#include <stdlib.h>
#include "MM32F103.h"
#include "Common.h"
#include "FIFO.h"
#include "BSP.h"


#define CALL_BACK_MAX			8
#define UART_WRITE_WAITTING		2


static void Null(void);


static bool WriteFinish = TRUE;
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
void UART0Init(uint32_t BPS)
{
	//��ʼ��USART0�ж����ȼ�
	nvic_irq_enable(USART0_IRQn, 0, 0);

    //��ʼ��USART0�ܽ�	
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_10);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_10);
	gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);
	gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);
	
	//��ʼ��USART0����
	rcu_periph_clock_enable(RCU_USART0);
	usart_baudrate_set(USART0, BPS);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_interrupt_disable(USART0, USART_INT_TC);
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);

    //��ʼ��������
	Read = FIFOCreate(256, sizeof(uint8_t));
	FIFOCallbackWrite(Read, WriteData);
	Write = FIFOCreate(128, sizeof(uint8_t));
	FIFOCallbackWrite(Write, WriteData);
}

//*******************************************************************
//���ã�UART��Ӷ��жϳ���
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���������8��
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
//*******************************************************************
bool UART0AddReadISR(CallBackISR Function)
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
bool UART0AddWriteISR(CallBackISR Function)
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
void UART0Open(void)
{
	usart_enable(USART0);
}

//*******************************************************************
//���ã�UART��
//��������
//���أ���
//*******************************************************************
void UART0Close(void)
{
	FIFOClear(Read);
	FIFOClear(Write);
	usart_disable(USART0);
}

//*******************************************************************
//���ã�UARTд����
//������Data-���͵�����
//���أ���
//*******************************************************************
void UART0Write(uint8_t Data)
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
		if(WriteFinish == TRUE)
		{
			usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
			usart_interrupt_enable(USART0, USART_INT_TC);
			usart_data_transmit(USART0, Data);
			WriteFinish = FALSE;
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
bool UART0AllowRead(void)
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
uint8_t UART0Read(void)
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
void UART0BufClear(void)
{
	FIFOClear(Read);
	FIFOClear(Write);
}

//*******************************************************************
//���ã�UART����������
//��������
//���أ���
//*******************************************************************
void UART0Routine(void)
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
			if(WriteFinish == TRUE)
			{
				FIFOPop(Write, &tmp);
				usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
				usart_interrupt_enable(USART0, USART_INT_TC);
				usart_data_transmit(USART0, tmp);			
				WriteFinish = FALSE;
			}
		}
		__enable_irq();
	}
}

void USART0_IRQHandler(void)
{
	uint8_t tmp;
	uint8_t i = 0;
	
	if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_TC) == SET)
    {
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_TC);
    	if(FIFOCount(Write))
    	{
	    	FIFOPop(Write, &tmp);
			usart_data_transmit(USART0, tmp);
	    }
	    else
	    {
			usart_interrupt_disable(USART0, USART_INT_TC);
	        WriteFinish = TRUE;
    	}

    	while(i < CALL_BACK_MAX)
    	{
    		WriteISR[i]();
    		++i;
    	}
    }
	else if(usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) == SET)
    {
    	WriteDly = UART_WRITE_WAITTING;
    	tmp = usart_data_receive(USART0);

    	FIFOPush(Read, &tmp);    	

    	while(i < CALL_BACK_MAX)
    	{
    		ReadISR[i]();
    		++i;
    	}
    }
    return;
}

void UART0SetBaud(uint32_t BPS)
{
	usart_baudrate_set(USART0, BPS);
	FIFOClear(Read);
	FIFOClear(Write);
}

//��ȡ���������ݳ���
uint32_t UART0GetReadCount(void)
{
	return FIFOCount(Read);
}





