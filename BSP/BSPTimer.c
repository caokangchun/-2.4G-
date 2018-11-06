#include "MM32F103.h"
#include "BSP.h"


#define CALL_BACK_MAX					10

#define	SYSTICK_TIMER_TYPE_NULL			0
#define	SYSTICK_TIMER_TYPE_INTERVAL		1
#define	SYSTICK_TIMER_TYPE_ONCE			2


typedef struct
{
	CallBackISR ISR;
	uint16_t Scale;
	uint16_t Tick;
	uint8_t Type;
}
CallBack_Program;


static void Null(void);


static CallBack_Program Program[CALL_BACK_MAX] = 
{
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL},
	{Null, 0, 0, SYSTICK_TIMER_TYPE_NULL}
};


static void Null(void)
{
}

//*******************************************************************
//���ã�ϵͳʱ����ʼ�����ڲ��̶�1ms�ж�һ��
//��������
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
//*******************************************************************
bool BSPTimerInit(void)
{	
	if(SysTick_Config(1000 * ((SystemCoreClock / 8) / 1000000)) == 1)
	{
		return(FALSE);
	}	
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	return(TRUE);
}

//*******************************************************************
//���ã��������ִ�еĶ�ʱ�������+-1ms
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���
//		Scale-��ʱ�������������ִ�е�ʱ����䣬��λ1ms
//���أ�1��10-��ʱ�����
//      0-ʧ�� 
//*******************************************************************
uint8_t BSPTimerIntervalCreate(CallBackISR Function, uint16_t Scale)
{
	uint8_t count = 0;
	
    while(count < CALL_BACK_MAX)
    {
		if(Program[count].Type == SYSTICK_TIMER_TYPE_NULL)
		{
			Program[count].ISR = Function;
			Program[count].Scale = Scale;
			Program[count].Tick = Scale;
			Program[count].Type = SYSTICK_TIMER_TYPE_INTERVAL;
			return(count + 1);
		}
        ++count;
    }
	return(0);
}

//*******************************************************************
//���ã�ɾ�����ִ�еĶ�ʱ��
//������TimerNum-��ʱ�����
//���أ���
//*******************************************************************
void BSPTimerIntervalDestroy(uint8_t TimerNum)
{
	--TimerNum;
	BSPSystemIntClose();
	Program[TimerNum].ISR = Null;
	Program[TimerNum].Scale = 0;
	Program[TimerNum].Tick = 0;
	Program[TimerNum].Type = SYSTICK_TIMER_TYPE_NULL;
	BSPSystemIntOpen();
}

//*******************************************************************
//���ã�����ִ��һ�εĶ�ʱ�������+-1ms
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���
//		Scale-��ʱ�������������ִ�еĵȴ�ʱ�䣬��λ1ms
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
//*******************************************************************
bool BSPTimerOnceCreate(CallBackISR Function, uint16_t Scale)
{
	uint8_t count = 0;
	
    while(count < CALL_BACK_MAX)
    {
		if(Program[count].Type == SYSTICK_TIMER_TYPE_NULL)
		{
			Program[count].ISR = Function;
			Program[count].Scale = Scale;
			Program[count].Tick = Scale;
			Program[count].Type = SYSTICK_TIMER_TYPE_ONCE;
			return(TRUE);
		}
        ++count;
    }
	return(FALSE);
}

//*******************************************************************
//���ã��ȴ���ʽ��ʱ
//������Scale-��ʱ������������ʱʱ�䣬���ֵ500����Сֵ50����λ1us
//���أ���
//*******************************************************************
void BSPTimerDelay(uint16_t Scale)
{
	uint32_t currentT;
	uint32_t scaleT;
	uint32_t startT = SysTick->VAL;
	uint32_t tmp;
	
	if(Scale > 500)
	{
		Scale = 500;
	}
	else if(Scale < 50)
	{
		Scale = 50;
	}
	
	scaleT = (uint32_t)Scale * 10000;
	scaleT /= (0.083333333 * 10000);
	
	if(startT > scaleT)
	{
		while(1)
		{
			currentT = SysTick->VAL;
			if(startT - currentT > scaleT)
			{
				return;
			}
		}
	}
	else
	{
		tmp = (12000 - 1) - (scaleT - startT);
		while(1)
		{
			currentT = SysTick->VAL;
			if(currentT > startT && currentT < tmp)
			{
				return;
			}
		}
	}
}

void SysTick_Handler(void)
{
	uint8_t i = 0;
	
	while(i < CALL_BACK_MAX)
	{
		if(Program[i].Type == SYSTICK_TIMER_TYPE_INTERVAL)
		{
			if(--Program[i].Tick == 0)
			{
				Program[i].ISR();
				Program[i].Tick = Program[i].Scale;
			}
		}
		else if(Program[i].Type == SYSTICK_TIMER_TYPE_ONCE)
		{
			if(--Program[i].Tick == 0)
			{
				Program[i].ISR();
				Program[i].ISR = Null;
				Program[i].Scale = 0;
				Program[i].Tick = 0;
				Program[i].Type = SYSTICK_TIMER_TYPE_NULL;
			}
		}
		++i;
	}
}

