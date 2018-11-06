#include "MM32F103.h"
#include "Common.h"
#include "BSPWatchDog.h"




static void PVU_CheckStatus(void);
static void RVU_CheckStatus(void);


/*******************************************************************
���ã����Ź���ʼ����λʱ�䣬����ʼ����
������
	MS409,
	MS819,
	MS1638,
	MS3276,
	MS6553,
	MS13107,
	MS26214,
���أ���
*******************************************************************/
extern void BSPWatchDogInit(WDTInterval time)
{
    /*�����ڲ�����ʱ��,�ȴ�ʱ�Ӿ���*/
    RCC_LSICmd(ENABLE);
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY)==RESET);
    
    /*����ʱ��Ԥ��Ƶ*/	
    PVU_CheckStatus();
    IWDG_WriteAccessCmd(0x5555);
	switch(time)
	{
		case MS409:
			IWDG_SetPrescaler(IWDG_Prescaler_4);
			break;
		
		case MS819:
			IWDG_SetPrescaler(IWDG_Prescaler_8);
			break;

		case MS1638:
			IWDG_SetPrescaler(IWDG_Prescaler_16);
			break;
		
		case MS3276:
			IWDG_SetPrescaler(IWDG_Prescaler_32);
			break;
		
		case MS6553:
			IWDG_SetPrescaler(IWDG_Prescaler_64);
			break;		
		
		case MS13107:
			IWDG_SetPrescaler(IWDG_Prescaler_128);
			break;
		
		case MS26214:
			IWDG_SetPrescaler(IWDG_Prescaler_256);
			break;
		
		default: break;
	}
    
    
    /*�������ؼĴ���ֵ*/	
    RVU_CheckStatus();
    IWDG_WriteAccessCmd(0x5555);
    IWDG_SetReload(0xfff);	
    
    /*װ�ز�ʹ�ܼ�����*/	
    IWDG_ReloadCounter();
    IWDG_Enable();	
}


/*******************************************************************
���ã�ι������
��������
���أ���
*******************************************************************/
void BSPWatchDogReset(void)
{
	IWDG_ReloadCounter();
}




/********************************************************************************************************
**������Ϣ ��PVU_CheckStatus(void)                       
**�������� �����������Ź�Ԥ��Ƶλ״̬
**������� ����
**������� ����
********************************************************************************************************/
static void PVU_CheckStatus(void)
{
    while(1)
    {
        /*���Ԥ��Ƶλ״̬,ΪRESET�ſɸı�Ԥ��Ƶֵ*/
        if(IWDG_GetFlagStatus(IWDG_FLAG_PVU)==RESET)                                                       
        {
            break;
        }
    }
}
/********************************************************************************************************
**������Ϣ ��RVU_CheckStatus(void)                  
**�������� �����������Ź����ر�־
**������� ����
**������� ����
********************************************************************************************************/
static void RVU_CheckStatus(void)
{
    while(1)
    {
        /*������ر�־״̬*/
        if(IWDG_GetFlagStatus(IWDG_FLAG_RVU)==RESET)  
        {
            break;
        }
    }
}
