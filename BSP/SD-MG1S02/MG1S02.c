#include <stdlib.h>
#include <string.h>
#include "MM32F103.h"
#include "Common.h"
#include "FIFO.h"
#include "DLList.h"
#include "BSP.h"
#include "UART0.h"
#include "MG1S02.h"


#define		STATE_RECEIVING				0
#define		STATE_EXECUTE_COMMAND		1
#define		STATE_IDLE					2

#define Data_Buffer_Size 2400
#define Len_Buffer_Size 128

typedef struct
{
	uint8_t WorkState;
}
Module_State;

static Module_State ModuleState;

static FIFO DataBuffer;		//���ݻ���
static FIFO LenBuffer;		//��������

static uint32_t ReceiveTimer = 0;	//����������ʱ
static uint8_t RxBuffer[266];		//���ݽ��ջ���
static uint8_t HeadLen = 0;			//ͷ������
static uint16_t RxDataLen = 0;		//���ս������ݳ���
static uint16_t LenCount = 0;

static uint32_t BlockingTime = 0;	//����ʱ��
static uint16_t AckDataLen = 0; 	//��־λ�������ص����ݳ���
static uint8_t* FlagBuffer;
static uint8_t CRCBuffer[2];
static uint16_t AckLenCount = 0;

static uint32_t ScanTime = 0;	//ɨ��ʱ��

//״̬�Ĵ��ֽ�
//bit6-7	���ݽ���״̬
//bit0-1	�ظ�����״̬
//bit2		���ͱ�־λ��������ȴ���Ӧ״̬
//bit3		ɨ�蹤��״̬
static uint8_t StateByte = 0;

void MG1S02Routine(void);
static void MG1S02CheakBaud(void);
static void MG1S02DefaultConfiguration(void);
bool MG1S02SendCommand(uint16_t FlagAddr, uint8_t FlagLen, uint8_t BitsLen, uint8_t BitsOffset, uint16_t Data ,bool NeedToRead);

static void WriteData(void* DataTo, const void* DataFrom)
{
	*((uint8_t*)DataTo) = *((uint8_t*)DataFrom);
}
static void WriteLen(void* DataTo, const void* DataFrom)
{
	*((uint16_t*)DataTo) = *((uint16_t*)DataFrom);
}

//*******************************************************************+
//���ã�CRCУ�� 
//������Ptr-У����ַ���
//		Len-У�鳤��
//���أ���
//*******************************************************************
static uint16_t CRCCalByBit(uint8_t *Ptr, uint8_t Len)
{
	uint32_t crc = 0;
	uint8_t i;

	while(Len-- != 0)
	{
		for(i = 0x80; i != 0; i /= 2)
		{
			crc *= 2;
			if((crc&0x10000) !=0) //��һλ CRC �� 2 ������λ�� 1������� 0x11021
				crc ^= 0x11021;
			if((*Ptr&i) != 0) //�����λ�� 1����ô CRC = ��һλ�� CRC + ��λ/CRC_CCITT
				crc ^= 0x1021;
		}
		Ptr++;
	}
	return crc;
}

//*******************************************************************
//���ã�ɨ��ģ���ʼ�� 
//��������
//���أ���
//*******************************************************************
void MG1S02Init(void)
{	
	ModuleState.WorkState = STATE_IDLE;
	
	UART0Init(115200);
	UART0Open();
	BSPTimerAddISR(UART0Routine);
	BSPTimerAddISR(MG1S02Routine);
	
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB,GPIO_MODE_OUTPUT,GPIO_PUPD_NONE,GPIO_PIN_1);
	gpio_output_options_set(GPIOB,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_1);
	gpio_bit_set(GPIOB,GPIO_PIN_1); 
	
	MG1S02CheakBaud();
	MG1S02DefaultConfiguration();
	
	//��ʼ��������
	DataBuffer = FIFOCreate(Data_Buffer_Size, sizeof(uint8_t));
	FIFOCallbackWrite(DataBuffer, WriteData);
	LenBuffer = FIFOCreate(Len_Buffer_Size, sizeof(uint16_t));
	FIFOCallbackWrite(LenBuffer, WriteLen);
	
	ModuleState.WorkState = STATE_RECEIVING;
}

//*******************************************************************
//���ã���ʼɨ��
//��������
//���أ���
//*******************************************************************
static void MG1S02ScanStart(void)
{
		ScanTime = 0;
		StateByte |= 0x08;
		gpio_bit_reset(GPIOB,GPIO_PIN_1);	
}

//*******************************************************************
//���ã�ֹͣɨ��
//��������
//���أ���
//*******************************************************************
static void MG1S02ScanStop(void)
{
	ScanTime = 0;
	StateByte &= 0xF7;
	gpio_bit_set(GPIOB,GPIO_PIN_1);
}

//*******************************************************************
//���ã�ɨ�谴����Ӧ
//��������
//���أ���
//*******************************************************************
void MG1S02Scan(void)
{
	if(gpio_output_bit_get(GPIOB,GPIO_PIN_1) == SET)
	{
		MG1S02ScanStart();
	}
	else
	{
		MG1S02ScanStop();
	}	
}

//*******************************************************************
//���ã����մ��ڻ����һ���ֽ�
//��������
//���أ�TRUE-������һ����������
//      FALSE-����δ���
//*******************************************************************
static bool MG1S02ReceiveData(void)
{
	uint8_t tmp;
	uint32_t readLen = 0;
	
	uint8_t RxDataState = StateByte>>6;
	
	readLen = UART0GetReadCount();		//���ڻ�������ݳ���
			
	switch(RxDataState)
	{
		case 0:
		{
			while(readLen--)
			{
				tmp = UART0Read();
				if(tmp == 0x03)		//��ʼ
				{
					RxBuffer[0] = tmp;
					HeadLen = 1;		//�ѽ���ͷ����Э�飩����
					ReceiveTimer = 0;
					RxDataState = 1;	//��ʼ����
					break;
				}
			}
			break;
		}
			
		case 1:
		{
			while(readLen--)
			{
				tmp = UART0Read();
				RxBuffer[HeadLen++] = tmp;
				if(HeadLen == 3)  //��ȡǰ�����ַ�
				{
					RxDataLen = RxBuffer[1];	//��ȡ�������ݳ���
					RxDataLen <<=8;
					RxDataLen |= RxBuffer[2];
					
					if(RxDataLen>(Data_Buffer_Size-FIFOCount(DataBuffer)))
					{
						HeadLen = 0;
						RxDataLen = 0;
						ReceiveTimer = 0;
						StateByte &= 0x3F;
						MG1S02ScanStop();
						return(FALSE);
					}
					
					LenCount = 0;	 //��ʼ����
					RxDataState = 2; //����������ݽ���״̬
					break;
				}	
			}
			break;
		}
			
		case 2:
		{
			while(readLen--)
			{
				tmp = UART0Read();
				if(LenCount<RxDataLen)	
				{
					FIFOPush(DataBuffer, &tmp); 
					LenCount++;
				}
				if(LenCount>=RxDataLen)	//�ѻ�ȡȫ������
				{
					FIFOPush(LenBuffer,&RxDataLen);
					HeadLen = 0;
					RxDataLen = 0;
					LenCount = 0;
					ReceiveTimer = 0;
					StateByte &= 0x3F;
					MG1S02ScanStop();
					return(TRUE);
				}
			}
			break;
		}
	}
	
	if(RxDataState!=0)
	{
		ReceiveTimer++;
		if(ReceiveTimer>=100)
		{
			if(RxDataState==2)
			{
				FIFOSpecifyClear(DataBuffer, LenCount);
			}
			RxDataState = 0;
		}
	}
	
	StateByte = (StateByte&0x3F) | (RxDataState<<6);
	
	return(FALSE);
}

//*******************************************************************
//���ã��ж��Ƿ��н����������
//��������
//���أ�TRUE-�ɶ�
//      FALSE-���ɶ�
//*******************************************************************
bool MG1S02AllowRead(void)
{

	if(FIFOCount(LenBuffer))
	{
		return(TRUE);
	}
	
	return(FALSE);
}

//*******************************************************************
//���ã����ձ�־λ�����Ļظ�
//��������
//���أ�TRUE-�����ɹ�
//      FALSE-����ʧ��
//*******************************************************************
static bool MG1S02ReceiveAck(void)
{
	uint8_t tmp;
	uint16_t crc; 
	uint32_t readLen = 0;
	uint8_t RxAckState = StateByte&0x03;
	
	while(UART0AllowRead() == TRUE)
	{
		while((StateByte>>6)!=0)
		{
			MG1S02ReceiveData();	//û��������Ƚ���
		}
		
		readLen = UART0GetReadCount();		//���ڻ�������ݳ���
		
		switch(RxAckState)
		{
			case 2:
			{
				while(readLen--)
				{
					tmp = UART0Read();
					RxBuffer[AckLenCount++] = tmp;
					if(AckLenCount >= (AckDataLen+4))
					{
						RxAckState = 3;
						break;
					}
				}
				break;
			}
			
			case 0:
			{
				while(readLen--)
				{
					tmp = UART0Read();
					if(tmp == 0x02)
					{
						RxBuffer[0] = tmp;	
						AckLenCount = 1;
						RxAckState = 1;
						break;
					}
				}
				break;
			}
			
			case 1:
			{
				while(readLen--)
				{
					tmp = UART0Read();
					RxBuffer[AckLenCount++] = tmp;
					if(AckLenCount == 4)
					{
						AckDataLen = tmp;
						if(AckDataLen == 0)
						{
							AckDataLen = 256;
						}
						RxAckState = 2;
						break;
					}
				}
				break;
			}
			
			case 3:
			{
				while(readLen--)
				{
					tmp = UART0Read();
					CRCBuffer[AckLenCount-(AckDataLen+4)] = UART0Read();
					AckLenCount++;
					if(AckLenCount >= (AckDataLen+6))
					{
						crc = CRCCalByBit(&RxBuffer[2],AckDataLen+2);
					
						if(0x00 != RxBuffer[1])		//ͷ�����򷵻ش���
						{
							return(FALSE);
						}		
						if(0x00 != RxBuffer[2])
						{
							return(FALSE);	
						}
						if(AckDataLen != RxBuffer[3])
						{
							return(FALSE);
						}
						if(CRCBuffer[0] == (uint8_t)(crc>>8) && CRCBuffer[1] == (uint8_t)crc)
						{
							FlagBuffer = &RxBuffer[4];
							return(TRUE);
						}
						return(FALSE);
					}
				}
				break;
			}
			
		}
	}
	
	StateByte = (StateByte&0xFC) | RxAckState;
	
	return(FALSE);
}

//*******************************************************************
//���ã�д��־λ
//������Addr-��־λ��ַ
//		Len-����д��־λ����������Ϊ256��0��ʾ��
//		Data-д���־λ����
//���أ�TRUE-д�ɹ�
//      FALSE-дʧ��
//*******************************************************************
static bool MG1S02WriteFlag(uint16_t Addr, uint8_t Len, const uint8_t* Data) 
{
	uint8_t i;
	uint16_t crc;
	uint16_t dataLen;
	uint8_t buf[8+256];
	
	dataLen = Len;
	if(Len == 0)
	{
		dataLen = 256;
	}
	
	buf[0] = 0x7E;	//ͷ
	buf[1] = 0x00;
	
	buf[2] = 0x08;	//д����
	
	buf[3] = dataLen;		//д���ݳ���
	
	buf[4] = (uint8_t)(Addr>>8);	//��ַ
	buf[5] = (uint8_t)Addr;

	for(i=0;i<dataLen;i++)
	{
		buf[6+i] = Data[i];		//����
	}

	crc = CRCCalByBit(&buf[2],4+dataLen);		//crcУ��

	buf[6+dataLen] = (uint8_t)(crc>>8);
	buf[7+dataLen] =(uint8_t)crc;

	for(i=0;i<(8+dataLen);i++)		//8���̶�����+���ݳ���len
	{
		UART0Write(buf[i]);		//���ڷ���
	}
	
	StateByte |= 0x04;
	StateByte &= 0xFC;	
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=(200+dataLen))
	{
		if(MG1S02ReceiveAck() == TRUE)
		{
			StateByte &= 0xFB;
			return(TRUE);
		}
	}
	
	StateByte &= 0xFB;
	return(FALSE);
}

//*******************************************************************
//���ã�����־λ 
//������Addr-��־λ��ַ
//		Len-��������־λ�Ĵ���
//���أ�TRUE-���ɹ�
//      FALSE-��ʧ��
//*******************************************************************
static bool MG1S02ReadFlag(uint16_t Addr, uint8_t Len)
{	
	uint8_t i;
	uint16_t crc;
	uint8_t buf[9];
	
	buf[0] = 0x7E;	//ͷ
	buf[1] = 0x00;
	
	buf[2] = 0x07;	//������
	
	buf[3] = 0x01;		
	
	buf[4] = (uint8_t)(Addr>>8);	//��ַ
	buf[5] = (uint8_t)Addr;

	buf[6] = Len;		//Ҫ������ȡ�ı�־λ��λ��

	crc = CRCCalByBit(&buf[2],5);		//crcУ��

	buf[7] = (uint8_t)(crc>>8);
	buf[8] =(uint8_t)crc;

	for(i=0;i<9;i++)			//8���̶�����+���ݳ���len
	{
		UART0Write(buf[i]);		//���ڷ���
	}

	StateByte |= 0x04;
	StateByte &= 0xFC;
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=(200+Len))
	{
		if(MG1S02ReceiveAck() == TRUE)
		{
			StateByte &= 0xFB;
			return(TRUE);
		}
	}
	
	StateByte &= 0xFB;
	return(FALSE);
}

//*******************************************************************
//���ã�У��ģ���봮�ڵĲ����� 
//��������
//���أ���
//*******************************************************************
static void MG1S02CheakBaud(void)	
{
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	uint16_t value;
	uint8_t baudFlag = 0;
	
	while(1)
	{
		UART0Close();
		switch(baudFlag)
		{
			case 0:
			{
				UART0SetBaud(115200);
				break;
			}
			case 1:
			{
				UART0SetBaud(9600);
				break;
			}
			case 2:
			{
				UART0SetBaud(57600);
				break;
			}
			case 3:
			{
				UART0SetBaud(38400);
				break;
			}
			case 4:
			{
				UART0SetBaud(19200);
				break;
			}
			case 5:
			{
				UART0SetBaud(14400);
				break;
			}
			case 6:
			{
				UART0SetBaud(4800);
				break;
			}
			case 7:
			{
				UART0SetBaud(1200);
				break;
			}
		}
		UART0Open();
		
		if(MG1S02ReadFlag(0x002A,2) == TRUE)
		{
			value = ((FlagBuffer[1]<<8) | FlagBuffer[0]) & 0x1FFF;	//ȡbit12-0
			switch(value)
			{
				case 0x001A:
				{
					if(baudFlag == 0)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x0139:
				{
					if(baudFlag == 1)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x0034:
				{
					if(baudFlag == 2)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x004E:
				{
					if(baudFlag == 3)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x009C:
				{
					if(baudFlag == 4)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x00D0:
				{
					if(baudFlag == 5)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x0271:
				{
					if(baudFlag == 6)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}
				case 0x09C4:
				{
					if(baudFlag == 7)
					{
						ModuleState.WorkState = STATE_RECEIVING;
						return ;
					}
					break;
				}

			}
		}
		
		baudFlag++;
		if(baudFlag>=8)
		{
			baudFlag = 0;
		}
	}
}

//*******************************************************************
//���ã�ǿ������ָ������ 
//��������
//���أ���
//*******************************************************************
static void MG1S02DefaultConfiguration(void)
{
	uint8_t state = 0;
	uint8_t data;
	uint8_t tmp;
	
	while(1)
	{
		switch(state)
		{
			case 0:
			{
				if(MG1S02ReadFlag(0x000D,1) == TRUE)	//�����UTF8���������
				{
					data = FlagBuffer[0];
					tmp = data<<4;
					if(0xC0 == tmp)
					{
						state = 2;
					}
					else
					{
						data &= 0xF0;
						data |= 0x0C;
						state = 1;
					}		
				}
				break;
			}
			case 1:
			{
				if(MG1S02WriteFlag(0x000D,1,&data) == TRUE)	//д���UTF8���������
				{
					state = 2;
				}
				break;
			}
			case 2:
			{
				if(MG1S02ReadFlag(0x0060,1) == TRUE)	//�����ڴ�Э��
				{
					data = FlagBuffer[0];
					if((data>>7) == 1)
					{
						return;
					}
					else
					{
						data |= 0x80;
						state = 3;
					}
				}
				break;
			}
			case 3:
			{
				if(MG1S02WriteFlag(0x0060,1,&data) == TRUE)	//д���ڴ�Э��
				{
					return;
				}
				break;
			}
		}
	}
}


//*******************************************************************
//���ã���������ͷ�������жϻ��ѣ�
//��������
//���أ���
//*******************************************************************
void MG1S02Open(void)	//����
{
	uint8_t i;
	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	uint8_t buf[9]={0x7E,0x00,0x08,0x01,0x00,0xD9,0x00,0xDB,0x26};

	for(i=0;i<9;i++)
	{
		UART0Write(buf[i]);		//���ڷ���
	}
	
	StateByte |= 0x04;	
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=50)
	{
		
	}
	
	StateByte &= 0xFB;;
	ModuleState.WorkState = STATE_RECEIVING;
}

//*******************************************************************
//���ã��ر�����ͷ�����˯��1.8mA��
//��������
//���أ���
//*******************************************************************
void MG1S02Close(void)	
{
	uint8_t i;
	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	uint8_t buf[9]={0x7E,0x00,0x08,0x01,0x00,0xD9,0xA5,0x3E,0x69};

	for(i=0;i<9;i++)
	{
		UART0Write(buf[i]);		//���ڷ���
	}
	
	StateByte |= 0x04;	
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=50)
	{
		
	}
	
	StateByte &= 0xFB;
	ModuleState.WorkState = STATE_RECEIVING;
}

//*******************************************************************
//���ã������־λ��flash 
//��������
//���أ�TRUE-����ɹ�
//      FALSE-����ʧ��
//*******************************************************************
bool MG1S02SaveFlag(void)
{
	uint8_t i;
	uint8_t buf[9];
	
	buf[0] = 0x7E;		//ͷ
	buf[1] = 0x00;	
	
	buf[2] = 0x09;		//����
	
	buf[3] = 0x01;		//����
	
	buf[4] = 0x00;		//��ַ
	buf[5] = 0x00;

	buf[6] = 0x00;		//datas

	buf[7] = 0xDE;		//crcУ��
	buf[8] = 0xC8;

	for(i=0;i<9;i++)			//8���̶�����+���ݳ���len
	{
		UART0Write(buf[i]);		//���ڷ���
	}
	
	StateByte |= 0x04;
	StateByte &= 0xFC;
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=100)
	{
		if(MG1S02ReceiveAck() == TRUE)
		{
			StateByte &= 0xFB;
			return(TRUE);
		}
	}
	
	StateByte &= 0xFB;
	return(FALSE);
}

//*******************************************************************
//���ã���־λ�ָ�����������
//��������
//���أ�TRUE-�ָ��ɹ�
//      FALSE-�ָ�ʧ��
//*******************************************************************
bool MG1S02FlagFactorySetting(void) 
{
	uint8_t i;
	uint16_t crc;
	uint8_t buf[9];
	
	buf[0] = 0x7e;		//ͷ
	buf[1] = 0x00;	
	
	buf[2] = 0x09;		//����
	
	buf[3] = 0x01;		//д���ݳ���
	
	buf[4] = 0x00;		//��ַ
	buf[5] = 0x00;

	buf[6] = 0xFF;		//datas

	crc = CRCCalByBit(&buf[2],5);		//crcУ��

	buf[7] = (uint8_t)(crc>>8);
	buf[8] = (uint8_t)crc;

	for(i=0;i<9;i++)			//8���̶�����+���ݳ���len
	{
		UART0Write(buf[i]);		//���ڷ���
	}
	
	StateByte |= 0x04;
	StateByte &= 0xFC;
	BlockingTime = 0;	//��������
	
	while(BlockingTime<=100)
	{
		if(MG1S02ReceiveAck() == TRUE)
		{
			StateByte &= 0xFB;
			return(TRUE);
		}
	}
	
	StateByte &= 0xFB;
	return(FALSE);
}

//*******************************************************************
//���ã����ò�����
//������Baud-������
//���أ�TRUE-���óɹ�
//      FALSE-����ʧ��
//*******************************************************************
bool MG1S02SetBaud(uint32_t Baud)
{
	uint8_t data[2];
	uint16_t val;
	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	switch (Baud)
	{
		case 1200:
		{
			val=0x09c4;
			break;
		}			
		case 4800:
		{
			val=0x0271;
			break;
		}		
		case 9600:
		{
			val=0x0139;
			break;
		}		
		case 14400:
		{
			val=0x00d0;
			break;	
		}	
		case 19200:
		{
			val=0x009c;
			break;
		}		
		case 38400:
		{
			val=0x004e;
			break;
		}
		case 57600:
		{
			val=0x0034;
			break;	
		}			
		case 115200:
		{
			val=0x001a;
			break;
		}		
		default: val=0x001a;	//115200
	}
	data[0] = val;
	data[1] = val>>8;
	
	if(MG1S02WriteFlag(0x002A,2,data) == TRUE)
	{
		UART0SetBaud(Baud);	//��д�ɹ�����Ҫ���³�ʼ������
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã���ò�����
//����: Baud-�����ʵ�ֵ��ָ��
//���أ�TRUE-��ȡ�ɹ�
//      FALSE-��ȡʧ��
//*******************************************************************
bool MG1S02GetBaud(uint32_t* Baud)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	*Baud = 0;
	uint16_t buf = 0;
	
	if(MG1S02ReadFlag(0x002A,2) == TRUE)
	{
		buf = FlagBuffer[1];
		buf = buf<<8 | FlagBuffer[0];
		
		switch (buf)
		{
			case 0x09c4:
			{
				*Baud=1200;
				break;
			}			
			case 0x0271:
			{	
				*Baud=4800;
				break;
			}		
			case 0x0139:
			{
				*Baud=9600;
				break;
			}		
			case 0x00d0:
			{
				*Baud=14400;
				break;	
			}	
			case 0x009c:
			{
				*Baud=19200;
				break;
			}		
			case 0x004e:
			{
				*Baud=38400;
				break;
			}
			case 0x0034:
			{
				*Baud=57600;
				break;	
			}			
			case 0x001a:
			{
				*Baud=115200;
				break;
			}		
			default:
			{
				ModuleState.WorkState = STATE_RECEIVING;
				return(FALSE);
			}
		}
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã������ַ�ֵ
//����:	Str-�����޸ĵ��ַ���
//		Addr-�ַ���ַ
//		Len-Ҫ���õ��ַ�����
//���أ�TRUE-�޸ĳɹ�
//      FALSE-�޸�ʧ��
//*******************************************************************
bool MG1S02SetCharacterValue(uint32_t Addr, const char *Str, uint16_t Len)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	if(Len > 15)
	{
		Len = 15;
	}

	if(MG1S02WriteFlag(Addr,Len,(uint8_t*)Str) == TRUE)		//�޸��ַ�
	{
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã�����ַ�ֵ
//����:	Str-��õ��ַ�����ֵָ��
//		Addr-�ַ���ַ
//		Len-Ҫ�����ַ�����
//���أ�TRUE-���ɹ�
//      FALSE-��ʧ��
//*******************************************************************
bool MG1S02GetCharacterValue(uint32_t Addr, char *Str, uint16_t Len)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	uint8_t i;
	
	if(Len > 15)
	{
		Len = 15;
	}

	if(MG1S02ReadFlag(Addr,Len) == TRUE)
	{
		for(i=0;i<Len;i++)
		{
			Str[i] = FlagBuffer[i];
		}
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}


//*******************************************************************
//���ã���ȡ�汾��
//����:	Addr-Ӳ���汾������汾��ַ
//		Version-�汾���ַ���ָ��
//���أ�TRUE-���ɹ�
//      FALSE-��ʧ��
//*******************************************************************
bool MG1S02GetVersion(uint32_t Addr, char* Version)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	uint8_t tmp;
	
	if(MG1S02ReadFlag(Addr,1) == TRUE)
	{
		tmp = FlagBuffer[0];
		Version[0] = 'V';
		Version[1] = tmp/100+0x30;
		Version[2] = '.';
		Version[3] = (tmp/10)%10+0x30;
		Version[4] = tmp%10+0x30;
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}


//*******************************************************************
//���ã���ȡ�������
//����:	Date-��������ַ���ָ��
//���أ�TRUE-���ɹ�
//      FALSE-��ʧ��
//*******************************************************************
bool MG1S02GetSoftwareDate(char* Date)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	if(MG1S02ReadFlag(0x00E3,3) == TRUE)
	{
		//year
		Date[0] = 0x32;
		Date[1] = FlagBuffer[0]/100+0x30;
		Date[2] = (FlagBuffer[0]/10)%10+0x30;
		Date[3] = FlagBuffer[0]%10+0x30;
		Date[4] = '.';
		
		//month
		Date[5] = FlagBuffer[1]/10+0x30;
		Date[6] = FlagBuffer[1]%10+0x30;
		Date[7] = '.';
		
		//day
		Date[8] = FlagBuffer[2]/10+0x30;
		Date[9] = FlagBuffer[2]%10+0x30;
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã���ȡ��Ʒ�ͺ�
//����:	Model-��Ʒ�ͺ��ַ���ָ��
//���أ�TRUE-���ɹ�
//      FALSE-��ʧ��
//*******************************************************************
bool MG1S02GetProductModel(char* Model)
{	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	if(MG1S02ReadFlag(0x00E0,3) == TRUE)
	{
		if(FlagBuffer[0] == 0x02)
		{
			strcpy(Model,"SD_MG1S02");
		}	
		else if(FlagBuffer[0] == 0x03)
		{
			strcpy(Model,"SD_MG1S03");
		}
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}


//*******************************************************************
//���ã����ʹ�������
//����: FlagAddr-��صı�־λ��ַ
//		FlagLen-�����޸ĵı�־λ����
//		BitsLen-ָ��������ռ�ֽڵ�λ����
//		BitsOffset-ָ���������ڵ��ֽ�λƫ��
//		Data-�����޸ĵ�����
//		NeedToRead-�Ƿ���Ҫ�ȶ�
//���أ�TRUE-���óɹ�
//      FALSE-����ʧ��
//*******************************************************************
bool MG1S02SendCommand(uint16_t FlagAddr, uint8_t FlagLen, uint8_t BitsLen, uint8_t BitsOffset, uint16_t Data ,bool NeedToRead)
{
	uint8_t value[2];	//�����Ͽ����趨256�ֽڣ���Ŀǰ���������ı�־λ��󲻳���16λ������2�ֽ��㹻
	uint8_t state = 0;
	uint16_t i;
	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	while(state!=3)
	{
		switch(state)
		{
			case 0:
			{
				if(NeedToRead == TRUE)
				{
					state = 1;
				}
				else
				{
					value[0] = Data;
					value[1] = Data>>8;
					state =  2;
				}
				break;
			}
			
			case 1:
			{
				if(MG1S02ReadFlag(FlagAddr,FlagLen) == TRUE)
				{
					if(FlagLen == 2)
					{
						uint16_t buf = 0;
						uint16_t tmp = 0;
						buf = FlagBuffer[0]|(FlagBuffer[1]<<8);
						for(i=0;i<FlagLen;i++)
						{
							tmp |= (1<<(i+BitsOffset));
						}
						Data = (Data<<BitsOffset)&tmp;
						buf &= ~tmp;
						buf |= Data;
						value[0] = buf>>8;
						value[1] = buf;
					}
					else
					{
						uint8_t tmp = 0;
						value[0] = FlagBuffer[0];
						for(i=0;i<BitsLen;i++)
						{
							tmp |= (1<<(i+BitsOffset));
						}
						Data = (Data<<BitsOffset)&tmp;
						value[0] &= ~tmp;
						value[0] |= Data;
					}
					state = 2;
				}
				else
				{
					state = 3;
				}
				break;
			}
			
			case 2:
			{
				if(MG1S02WriteFlag(FlagAddr,FlagLen,value) == TRUE)
				{
					ModuleState.WorkState = STATE_RECEIVING;
					return(TRUE);
				}
				else
				{
					state = 3;
				}
				break;
			}
			
			case 3:
			{
				break;
			}
		}
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã���ȡ����
//����: FlagAddr-��صı�־λ��ַ
//		FlagLen-�����޸ĵı�־λ����
//		BitsLen-ָ��������ռ�ֽڵ�λ����
//		BitsOffset-ָ���������ڵ��ֽ�λƫ��
//		Value-�����ڱ�־λ�е�ֵ
//���أ�TRUE-��ȡ�ɹ�
//      FALSE-��ȡʧ��
//*******************************************************************
bool MG1S02GetSetting(uint16_t FlagAddr, uint8_t FlagLen, uint8_t BitsLen, uint8_t BitsOffset, uint16_t* Value)
{
	*Value = 0;
	
	ModuleState.WorkState = STATE_EXECUTE_COMMAND;
	
	if(MG1S02ReadFlag(FlagAddr,FlagLen) == TRUE)
	{
		if(FlagLen == 2)
		{
			uint16_t buf = 0;
			buf = (FlagBuffer[0]<<8) | FlagBuffer[1];
			buf <<= 16 - (BitsOffset+BitsLen);
			buf >>= 16 - BitsLen;
			*Value = buf;
		}
		else
		{
			uint8_t buf = 0;
			buf = FlagBuffer[0];
			buf <<= 8 - (BitsOffset+BitsLen);
			buf >>= 8 - BitsLen;
			*Value = buf;
		}
		
		ModuleState.WorkState = STATE_RECEIVING;
		return(TRUE);
	}
	
	ModuleState.WorkState = STATE_RECEIVING;
	return(FALSE);
}

//*******************************************************************
//���ã���ȡһ���ѽ�����ɵ�ɨ�����ݣ�������
//����: Data-�ṩ����ģ���������ݵ�ָ��
//		DataLen-�ṩ����ģ�����õ����ݳ���ָ��
//���أ�	��
//*******************************************************************
void MG1S02ReadData(uint8_t* Data, uint16_t* DataLen)
{
	uint16_t len;
	uint16_t i;
	
	FIFOPop(LenBuffer,&len);
	*DataLen = len;
	
	for(i=0;i<len;i++)
	{
		FIFOPop(DataBuffer,&Data[i]);
	}
}

//*******************************************************************
//���ã�ģ�������������Ҫ����1����Ķ�ʱ��ִ�У�
//����: ��
//���أ���
//*******************************************************************
void MG1S02Routine(void)
{
	if((StateByte>>3)&1)
	{
		ScanTime++;
		if(ScanTime > 1000)
		{
			ScanTime = 0;
			StateByte &= 0xF7;
			gpio_bit_set(GPIOB,GPIO_PIN_1);	
		}
	}
	
	switch(ModuleState.WorkState)
	{ 
		case STATE_RECEIVING:
		{
			while(UART0AllowRead() == TRUE)
			{
				MG1S02ReceiveData();
			}
			break;
		}
		
		case STATE_EXECUTE_COMMAND:
		{
			MG1S02ScanStop();
			if((StateByte>>2)&1)
			{
				BlockingTime++;
			}
			break;
		}
		
		case STATE_IDLE:
		{
			break;
		}
	}
}


