#include "BSP2_4G.h"




//����ͨѶ��ʼ��
extern void BSP2_4GInit(void)
{
	/*д��̶�ֵ�����ڳ���2.4G����*/
	xn297l.pipe_index=0x17;
	xn297l.mac[0] = 0x87;xn297l.mac[1] = 0x39;xn297l.mac[2] = 0x6d;xn297l.mac[3] = 0xac;
	/****************************/
	
	rf_init();
}


