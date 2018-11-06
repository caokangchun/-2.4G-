#include <stdlib.h>
#include "Common.h"
#include "MM32F103.h"


//*******************************************************************
//���ã�ϵͳ��ʼ��
//��������
//���أ���
//*******************************************************************
void BSPSystemInit(void)
{	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);							//��������ʱ�ӣ����һ��Ҫ��
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);						//ʧ��JTAG��ʹ��SW
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

//*******************************************************************
//���ã����жϴ�
//��������
//���أ���
//*******************************************************************
void BSPSystemIntOpen(void)
{
	__enable_irq();
}

//*******************************************************************
//���ã����жϹر�
//��������
//���أ���
//*******************************************************************
void BSPSystemIntClose(void)
{
	__disable_irq();
}

//*******************************************************************
//���ã�ϵͳ��λ
//��������
//���أ���
//*******************************************************************
void BSPSystemReset(void)
{
	NVIC_SystemReset();
}


