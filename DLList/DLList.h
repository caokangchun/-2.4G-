#ifndef	__DLLIST_H__
#define	__DLLIST_H__

#include "MM32F103.h"
#include "Common.h"


//����DLList����
typedef struct DLList_* DLList;


//����DLListд���ݻص��������� 
typedef void(* DLListWrite)(void* DataTo, const void* DataFrom);
//����DLList�Ƚ����ݻص��������� 
typedef bool(* DLListEqual)(const void* DataA, const void* DataB);


//DLList����
extern DLList DLListCreate(uint16_t NodeNumber, uint8_t NodeLength);
//DLListд���ݻص���������
extern void DLListCallbackWrite(DLList Thiz, DLListWrite Write);
//DLList�Ƚ����ݻص���������
extern void DLListCallbackEqual(DLList Thiz, DLListEqual Write);
//��DLList����һ���ڵ� 
extern bool DLListNodeAdd(DLList Thiz, uint16_t Position, const void* Data);
//��DLListͷ����һ���ڵ� 
extern bool DLListNodeHeadAdd(DLList Thiz, const void* Data);
//��DLListβ����һ���ڵ� 
extern bool DLListNodeTailAdd(DLList Thiz, const void* Data);
//��DLListɾ��һ���ڵ� 
extern bool DLListNodeDel(DLList Thiz, uint16_t Position);
//��DLListͷɾ��һ���ڵ� 
extern bool DLListNodeHeadDel(DLList Thiz);
//��DLListβɾ��һ���ڵ� 
extern bool DLListNodeTailDel(DLList Thiz);
//��DLListд���޸ģ�һ���ڵ� 
extern bool DLListNodeWrite(DLList Thiz, uint16_t Position, const void* Data);
//��DLListͷд���޸ģ�һ���ڵ� 
extern bool DLListNodeHeadWrite(DLList Thiz, const void* Data);
//��DLListβд���޸ģ�һ���ڵ� 
extern bool DLListNodeTailWrite(DLList Thiz, const void* Data);
//��DLList��һ���ڵ� 
extern bool DLListNodeRead(DLList Thiz, uint16_t Position, void* Data);
//��DLListͷ��һ���ڵ� 
extern bool DLListNodeHeadRead(DLList Thiz, void* Data);
//��DLListβ��һ���ڵ� 
extern bool DLListNodeTailRead(DLList Thiz, void* Data);
//�ƶ�DLList�ڵ�
extern bool DLListNodeMove(DLList Thiz, uint16_t PositionTo, uint16_t PositionFrom);
//�ƶ��ڵ���DLListͷ 
extern bool DLListNodeHeadMove(DLList Thiz, uint16_t PositionFrom);
//�ƶ��ڵ���DLListβ 
extern bool DLListNodeTailMove(DLList Thiz, uint16_t PositionFrom);
//����DLList�����ڵ� 
extern bool DLListNodeSwap(DLList Thiz, uint16_t PositionA, uint16_t PositionB);
//�ڵ㽻����DLListͷ 
extern bool DLListNodeHeadSwap(DLList Thiz, uint16_t Position);
//�ڵ㽻����DLListβ 
extern bool DLListNodeTailSwap(DLList Thiz, uint16_t Position);
//����DLList���нڵ� 
extern bool DLListNodeInvert(DLList Thiz);
//���DLList�����ڵ���� 
extern uint16_t DLListNodeCount(DLList Thiz);
//����DLList�Ϻ�����ͬ���ݵĵ�һ���ڵ�� 
extern int32_t DLListNodeSearch(DLList Thiz, const void* Data);
//�������DLList 
extern bool DLListClear(DLList Thiz);
//����DLList 
extern bool DLListDestroy(DLList* Thiz);


#endif
