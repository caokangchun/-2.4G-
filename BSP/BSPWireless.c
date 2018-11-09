#include <stdlib.h>
#include "MM32F103.h"
#include "Common.h"
#include "BSP.h"
#include "FIFO.h"
#include "DLList.h"
#include "wireless.h"
#include "ConfigAddr.h"

CMD_UPLOAD pktCmdUpload={0xfe,0xef,0,0};	//��ʼ���ϴ������



static Wireless_Type WirelessType = TYPE_2_4G;	//Ĭ��2.4G



unsigned char crc_high_first(unsigned char *ptr, unsigned char len)
{
    unsigned char i; 
    unsigned char crc=0x00; /* ����ĳ�ʼcrcֵ */ 

    while(len--)
    {
        crc ^= *ptr++;  /* ÿ��������Ҫ������������,������ָ����һ���� */  
        for (i=8; i>0; --i)   /* ������μ�����������һ���ֽ�crcһ�� */  
        { 
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc = (crc << 1);
        }
    }

    return (crc); 
}


BOOL checkData(unsigned char *ptr, UINT16 len)
{
	uint8_t crc;
	
	crc = crc_high_first(ptr,len-1);
	if(crc == ptr[len-1])
	{
		return TRUE;
	}

	return FALSE;
}



/***************���������ص�ǰ��������****************/
Wireless_Type BSPGetWireleeType(void)
{
	if(BT_LINK_IN) // BT connected
	{
		WirelessType = TYPE_BlueT;
	}
	else
	{
		WirelessType = TYPE_2_4G;
	}
	return WirelessType;
}




/**********2.4G,������ʼ�����Լ����ûص�����**********/
void BSPWirelessInit(void)
{
	RingBuffer_Init(&uart_ring, uart_buffer, RING_SIZE);
	//�����������
	BSPTimerIntervalCreate(RF_Timer,1);
	BSPTimerIntervalCreate(BT_Timer,1);
	
	rf_init();
	BT_Initial();
//	I435eUartInit();
}




/***********************************
����ͨѶ��������
������
	uint8_t* ָ��
	uint16_t ����
���أ�
	TRUE	�ɹ�
	FALSE	ʧ��
***********************************/
bool BSPWirelessWriteBytes(const uint8_t* Data, uint16_t Count)
{
	__disable_irq();
	if ((Count + 2) <= (uart_ring.buffer_size - RingBuffer_Data_Len(&uart_ring)))	//����δ��
	{
		RingBuffer_Write(&uart_ring, (uint8_t*)&Count, 2);		//д�볤��
		RingBuffer_Write(&uart_ring, (uint8_t*)Data, Count);	//д������	
	}
	__enable_irq();
	
	return Wireless_Main();
}







/***********************************
������	2.4G �շ����
������	void
���أ�
	TRUE	�ɹ�
	FALSE	ʧ��
***********************************/
bool BSPWirelseeRequestMatch(void)
{
	bool b;
	rf_host.idleout = 0;
	rf_host.rx_init = TRUE;
	
    uart.buff[0] = 8;//-= 2;
    uart.index = uart.buff[0];	

	if ( rf_host.rx_init )
	{
		b = RF_PairForRx();
		if(b)
		{
			GetMAC2_4G();
			BSPEEPROMWrite(CONFIG_2_4MAC, (uint16_t*)GetMAC2_4G(), 2);		
		}
	}	

	BSPWatchDogReset();
	uart.data_ok = 0;	
	return b;
}



bool SendCmd2_4G(CMD_UPLOAD* p);
//char TestString[] = "caoKangChun@@";
char TestString[] = "guangzhousanmakejiyouxiangongsi\r";

//����ͨѶѭ������
void BSPWirelessRoutine(void)
{
	pktCmdUpload.sid = getPkt_sid();		//��ȡsid

	pktCmdUpload.crc = crc_high_first((uint8_t*)&pktCmdUpload,sizeof(CMD_UPLOAD)-1);	//����crc
	
	SendCmd2_4G(&pktCmdUpload);
//	UART2Routine();
//	
//	if(CONNECT == GetBTStatus())
//	{
//		I435eSendContent(TestString,strlen(TestString));	
//	}
}




//����ͨѶ������
void BSPWirelessConfig(void)
{

}	

//����ͨѶ��
bool BSPWirelessOpen(void)
{
	return(TRUE);
}
	
//����ͨѶ�ر�
bool BSPWirelessClose(void)
{
	return(TRUE);
}

//����ͨѶ��λ
bool BSPWirelessReset(void)
{
	return(TRUE);
}

//����ͨѶ��������
bool BSPWirelessConnet(void)
{
	return(TRUE);
}

//����ͨѶ�Ͽ�����
bool BSPWirelessDisconnet(void)
{
	return(TRUE);
}	

//����ͨѶ����״̬��ѯ
bool BSPWirelessOnline(void)
{
	return(TRUE);
}



//����ͨѶ��ѯ���ջ����þ���
uint16_t BSPWirelessBufCount(void)
{
	return(TRUE);
}

//����ͨѶ��ս��ջ���������
void BSPWirelessBufClear(uint8_t* Data, uint16_t Count)
{
	
}

//����ͨѶ��������һ�ֽ�
uint8_t BSPWirelessReadByte(void)
{
	return(TRUE);
}	

//����ͨѶ�������ݶ��ֽ�
void BSPWirelessReadBytes(uint8_t* Data, uint16_t Count)
{
}

//����ͨѶ��ӷ�����������жϷ���
bool BSPWirelessAddWriteISR(CallBackISR Program)
{
	return(TRUE);
}

//����ͨѶ��ӽ�����������жϷ���
extern bool BSPWirelessAddReadISR(CallBackISR Program)
{
	return(TRUE);
}



