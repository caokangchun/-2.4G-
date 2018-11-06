#ifndef __BSP2_4G_H__
#define __BSP2_4G_H__	1


#include "BSP.h"
#include "includes.h"


//����ͨѶ��ʼ��
extern void BSP2_4GInit(void);

//����ͨѶ��������
extern bool BSP2_4GSendData(uint8_t *PData, uint8_t len);

extern bool BSP2_4GEnterRx(void);



//����ͨѶ��
extern bool BSP2_4GOpen(void);
//����ͨѶ�ر�
extern bool BSP2_4GClose(void);
//����ͨѶ��λ
extern bool BSP2_4GReset(void);
//����ͨѶ��������
extern bool BSP2_4GConnet(void);
//����ͨѶ�Ͽ�����
extern bool BSP2_4GDisconnet(void);
//����ͨѶ����״̬��ѯ
extern bool BSP2_4GOnline(void);

//����ͨѶ��ѯ���ջ�����������
extern uint16_t BSP2_4GBufCount(void);
//����ͨѶ��ս��ջ���������
extern void BSP2_4GBufClear(uint8_t* Data, uint16_t Count);
//����ͨѶ��������һ�ֽ�
extern uint8_t BSP2_4GReadByte(void);
//����ͨѶ�������ݶ��ֽ�
extern void BSP2_4GReadBytes(uint8_t* Data, uint16_t Count);
//����ͨѶ��ӷ�����������жϷ���
extern bool BSP2_4GAddWriteISR(CallBackISR Program);
//����ͨѶ��ӽ�����������жϷ���
extern bool BSP2_4GAddReadISR(CallBackISR Program);
//����ͨѶѭ������
extern void BSP2_4GRoutine(void);

#endif

