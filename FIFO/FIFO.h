#ifndef	__FIFO_H__
#define	__FIFO_H__


#include "Common.h"


//����FIFO����
typedef struct FIFO_* FIFO;


//����FIFOд���ݻص���������
typedef void(* FIFOWrite)(void* DataTo, const void* DataFrom);


//FIFO����
extern FIFO FIFOCreate(uint16_t DataNumber, uint8_t DataLength);
//FIFOд���ݻص���������
extern void FIFOCallbackWrite(FIFO Thiz, FIFOWrite Write);
//FIFOѹ������
extern bool FIFOPush(FIFO Thiz, const void* Data);
//FIFO��������
extern bool FIFOPop(FIFO Thiz, void* Data);
//���FIFO�������ݸ���
extern uint16_t FIFOCount(FIFO Thiz);
//���FIFOָ����������
extern bool FIFOSpecifyClear(FIFO Thiz, uint16_t Count);
//�������FIFO����
extern bool FIFOClear(FIFO Thiz);
//����FIFO
extern bool FIFODestroy(FIFO* Thiz);


#endif
