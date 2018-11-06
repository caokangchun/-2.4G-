#include <stdlib.h>
#include "Common.h"
#include "gd32f1x0.h"
#include "BSPBeep.h"
#include "ActionConfig.h"
#include "BSP.h"
	
#define CALL_BACK_MAX					8
	
#define TIME1_PSC						71U					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
#define TIME0_PSC						71U					//����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ

#define TIM_CLK  						(72U * 1000000U)	//ʱ��Դ

	
static void BeepGpioConfig(void);				//��������������.
static void BeepTimeSet(uint32_t t);			//������ʱ������.
static void Timer1PwmInit(void);				//��ʱ��1 PWM�����ʼ��.
static void Time0Init(uint32_t Interval);		//��ʱ��0 ��ʼ��.
static void BeepPlayRoutine(void);				//��������������.
static void BSPTime0Routine(void);				//��ʱ��0��������
	
static uint32_t BeepTime = 0;					//����ʱ��
static uint32_t ReloadValue = 0;				//�ؼ���ֵ
static uint8_t	DutyCycle = 50;					//ռ�ձ�
static uint32_t BeepCountTime=0;				//����ֵ


uint16_t Tone2[] = {0,1361,1412,1527,1712,1770,2066};

uint8_t Song_NewSetBeep[]={2,4,6,16};
uint8_t Beat_NewSetBeep[]={50,50,100};

uint8_t Song_NewErrorBeep[]={5,3,1,16};
uint8_t Beat_NewErrorBeep[]={100,100,150};

uint8_t Song_NewSetErrorBeep[]={2,4,6,5,3,1,16};
uint8_t Beat_NewSetErrorBeep[]={50,50,100,100,100,150};

uint8_t Song_NewStartBeep[]={6,0,6,16};
uint8_t Beat_NewStartBeep[]={92,50,82};

static void Null(void)
{
}

static CallBackISR ISR[CALL_BACK_MAX] = 
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

/***************************************************
���ã�	��������ʼ��.
������	��
����:	��
***************************************************/
void BSPBeepInit(void)
{
	
	BeepGpioConfig();
	Timer1PwmInit();

	Time0Init(1000);
	
	BSPTimer0AddFunc(BSPTime0Routine);
}

/***************************************************
���ã�	��ʱ��0��������.
������	��
����:	��
***************************************************/
void BSPTime0Routine(void)
{
	BeepPlayRoutine();
	BSPLEDFlashRoutine();
	
}

/***************************************************
���ã�	��������������.
������	��
����:	��
***************************************************/
static void BeepGpioConfig(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	
	gpio_mode_set(GPIOB,GPIO_MODE_AF,GPIO_PUPD_NONE,GPIO_PIN_3);
	gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_3);
	
	gpio_af_set(GPIOB,GPIO_AF_2,GPIO_PIN_3);//���Ÿ���
}


/***************************************************
���ã�	��ʱ��1 PWM�����ʼ��.
������	��
����:	��
***************************************************/
static void Timer1PwmInit(void)
{
	timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER1);
	timer_deinit(TIMER1);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = TIME1_PSC;//72M/(71+1)=1MHz
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 999;//1ms
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER1, &timer_initpara);
	
	/* CH1 configuration in OC active mode */
    timer_ocintpara.ocpolarity  = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.outputstate = TIMER_CCX_ENABLE;
	
	timer_channel_output_config(TIMER1, TIMER_CH_1, &timer_ocintpara);
	
//	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, 500);//���ò���ֵ
    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_1, TIMER_OC_SHADOW_DISABLE);
	
	 /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER1);
    timer_enable(TIMER1);

}

/***************************************************
���ã�	��ʱ��0 ��ʼ��.
������	Interval-��ʱ���жϼ��ʱ�䣬��λus
����:	��
***************************************************/
static void Time0Init(uint32_t Interval)
{
	timer_parameter_struct timer_initpara;
   
    rcu_periph_clock_enable(RCU_TIMER0);
    timer_deinit(TIMER0);
	
	 /* TIMER0 configuration */
    timer_initpara.prescaler         = TIME0_PSC;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = (Interval * ((TIM_CLK/TIME0_PSC)/1000000)-1);
//	timer_initpara.period            = 999;//1ms*10
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0, &timer_initpara);
	
	nvic_irq_enable(TIMER0_BRK_UP_TRG_COM_IRQn, 1, 1);
	
	/* Auto-reload preload enable */
	timer_auto_reload_shadow_enable(TIMER0);

    /* TIMER0 channel control update interrupt enable */
    timer_interrupt_enable(TIMER0, TIMER_INT_UP);

    /* TIMER0 counter enable */
    timer_enable(TIMER0);
}

//��������������.�޸�Ƶ��
void BSPBeepLenght(uint32_t Rate)
{

	if(Rate > TIM_CLK / (TIME1_PSC+1))//��Ƶ�ʴ������Ƶ��ʱ����ֻ�ܵ������Ƶ��
	{
		Rate = TIM_CLK / (TIME1_PSC+1);
	}
	else if(Rate < 1)//��СΪ1Hz
	{
		Rate = 1;
	}	
	
	//Ƶ��=TIM_CLK/{(ARR+1)*(PSC+1)} ==> arr=(TIM_CLK/(PSC+1)/Ƶ��)-1;
	ReloadValue = (TIM_CLK / (TIME1_PSC+1) / Rate) - 1;
}

//��������������.�޸�ռ�ձ�
void BSPBeepVol(uint16_t DutyRatio)
{
	DutyCycle = DutyRatio;
	
	if(DutyCycle > 100)//ռ�ձ����100%
	{
		DutyCycle = 100;
	}
	
//	DutyCycle = DutyRatio;//ռ�ձ�
}

/***************************************************
���ã�	����������.
������	Time-ʱ��
����:	��
***************************************************/
void BSPBeepPlay(uint32_t Time)
{
	BSPBeepLenght(ScanSet.BeepLenght);
	BSPBeepVol(ScanSet.BeepVol);
	
	BeepTimeSet(Time);
}

//����������������.
void BeepSoundTone(uint8_t *Song, uint8_t *Beat)
{
	uint8_t i=0;
	while(Song[i]<16)
	{
		if(!BeepCountTime)
		{
			BSPBeepLenght(Tone2[Song[i]]);
			BSPBeepVol(ScanSet.BeepVol);
			BeepTimeSet(Beat[i]);
			i++;
		}
	}
}

//����ǰ׺��׺�ȵ�ʱ�ķ�����
void SetBeepTone(void)
{
	BeepSoundTone(Song_NewSetBeep, Beat_NewSetBeep);
}

//���ó�����ʾ��
void ErrorBeepSound(Beep_Type beep)
{
	if(!beep)
	{
		BeepSoundTone(Song_NewErrorBeep,Beat_NewErrorBeep);
	}
	else
	{
		BeepSoundTone(Song_NewSetErrorBeep,Beat_NewSetErrorBeep);
	}
}

/***************************************************
���ã�	������ʱ������.
������	Time-ʱ��
����:	��
***************************************************/
static void BeepTimeSet(uint32_t Time)
{
	BeepTime = Time;
}

/***************************************************
���ã�	������ʱ����.
������	��
����:	BeepTime
***************************************************/
static uint32_t BeepTimeGet(void)
{
	return(BeepTime);
}

/***************************************************
���ã�	��������������.
������	��
����:	��
***************************************************/
static void BeepPlayRoutine(void)
{
	static uint8_t beepState = TRUE;
	
	if(0 == BeepCountTime--)//time--; if(time == 0)
	{
		BeepCountTime = BeepTimeGet();
		beepState = ScanSet.BeepSound;//��ȡ������״̬
		if(BeepCountTime && beepState)//����ʱ��ͷ�����״̬
		{
			timer_enable(TIMER1);
			timer_autoreload_value_config(TIMER1, ReloadValue);//���ö�ʱ���Զ����ؼĴ���ֵ
			timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, ReloadValue*DutyCycle/100);//���ò���ֵ
			
			BeepTimeSet(0);//������ȡ֮��,�ͽ�Time = 0;������ֻ��һ����.
		}
		else
		{
			timer_disable(TIMER1);
		}
	}
}


/*******************************************************************
//���ã���ʱ������жϳ���
//������Function-��ӵ���ʱ���ж�ʱִ�еĳ���������8��
//���أ�TRUE-�ɹ�
//      FALSE-ʧ�� 
*******************************************************************/
bool BSPTimer0AddFunc(CallBackISR Function)
{
    static uint8_t count = 0;

    if(count < CALL_BACK_MAX)
    {
        ISR[count++] = Function;
        return(TRUE);
    }
	return(FALSE);
}


void TIMER0_BRK_UP_TRG_COM_IRQHandler(void)
{
	if(timer_interrupt_flag_get(TIMER0 ,TIMER_INT_FLAG_UP))
	{		
		uint8_t i = 0;
	
		while(i < CALL_BACK_MAX)
		{
			ISR[i++]();
		}
		
		timer_interrupt_flag_clear(TIMER0,TIMER_INT_FLAG_UP);	//clear
	}
}


