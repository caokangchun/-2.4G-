#include "i435e.h"
#include "stdio.h"
#include "i435eKeyValDef.h"

extern void delay_1ms(uint16_t n);

#define RESPONSE_SIZE	256
static char I435eResponse[RESPONSE_SIZE]={0};




const char IM_READY[] = "IM_READY\r\n";
const char IM_CONNECT[] = "IM_CONN:1\r\n";
const char IM_DISCONNECT[] = "IM_DISC:1\r\n";
const char IM_OK[] = "OK\r\n";

const char BOND1[] = "AT+NEWBOND=1\r\n";


static BTState BTStatus=UNKNOW;


static void IntervalRoutine(void)
{
	ResponseParse(I435eRead());
}


//�ַ�ת����ֵ
static void Char2KeyVal(uint8_t chara);


extern BTState GetBTStatus(void)
{
	return BTStatus;
}
	


extern void I435eUartInit(void)
{
	UART2Init(115200);
	BSPTimerIntervalCreate(IntervalRoutine,5);
	UART2Open();
	
}



extern uint8_t I435eRead()
{
	uint8_t i=0;
	while(UART2AllowRead())
	{
		if(RESPONSE_SIZE == i+1)
		{
			return 0;
		}
		
		I435eResponse[i++] = UART2Read();
	}
	return i;
}



//����IM_READY
static bool ParseIM_READY(uint8_t len)
{
	uint8_t i=0,j=0;
	
	for(i=0; i<strlen(IM_READY); i++,j++)
	{
		if(j > len-1)
		{
			return FALSE;
		}
		if(IM_READY[i] != I435eResponse[j])
		{
			i=0-1;	//����ѭ����i++ ��=0, ���Դ˴�0-1
		}
	}

	return TRUE;
}



//����IM_CONNECT
static bool ParseIM_CONNECT(uint8_t len)
{
	uint8_t i=0,j=0;
	
	for(i=0; i<strlen(IM_CONNECT); i++,j++)
	{
		if(j > len-1)
		{
			return FALSE;
		}
		if(IM_CONNECT[i] != I435eResponse[j])
		{
			i=0-1;	
		}
	}

	return TRUE;
}



//����OK
static bool ParseIM_OK(uint8_t len)
{
	uint8_t i=0,j=0;
	
	for(i=0; i<strlen(IM_OK); i++,j++)
	{
		if(j > len-1)
		{
			return FALSE;
		}
		if(IM_OK[i] != I435eResponse[j])
		{
			i=0-1;	
		}
	}

	return TRUE;
}


static volatile bool FlagOk = FALSE;

//������������
extern void ResponseParse(uint8_t ResponseLen)
{
	if(0 == ResponseLen)
	{
		return;
	}
	
	if(ParseIM_READY(ResponseLen))
	{
		BTStatus = READY;
		I435eWrite((char*)BOND1,strlen(BOND1));
	}
	
	if(ParseIM_CONNECT(ResponseLen))
	{
		BTStatus = CONNECT;

	}
	
	if(ParseIM_OK(ResponseLen))
	{
//		BTStatus = OK;
		FlagOk = TRUE;
	}
	
}



extern void I435eWrite(char* dat,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)
	{
		UART2Write(dat[i]);	
	}
}

static void send(char* dat,uint8_t len)
{
	uint8_t i=0;
	for(;i<len;i++)
	{
		UART_SendData(UART2, dat[i]);	
		while(UART_GetFlagStatus(UART2,UART_FLAG_TXEMPTY) != SET);
	}

}



static void ReleaseAllKey(void)
{
//	Char2KeyVal(0);					//ת��
//	
//	__disable_irq();
//		FlagOk = FALSE;				//��λFlagOk
//		send((char*)BTKeyVal,strlen((char*)BTKeyVal));		//����
//	__enable_irq();
//	
//	while(!FlagOk);				//�ȴ�FlagOk 	
	
	
	__disable_irq();
		FlagOk = FALSE;				//��λFlagOk
		send((char*)BTKeyValNon,strlen((char*)BTKeyValNon));		//����
	__enable_irq();
	
	while(!FlagOk);				//�ȴ�FlagOk 		
}


extern void I435eSendContent(char* content,uint8_t len)
{
	uint8_t i=0;
	
	for(;i<len;i++)
	{
		Char2KeyVal(content[i]);	//ת��
		
		__disable_irq();
			FlagOk = FALSE;			//��λFlagOk
			send((char*)BTKeyVal,strlen((char*)BTKeyVal));	//����
		__enable_irq();
		
		while(!FlagOk);				//�ȴ�FlagOk 

//		ReleaseAllKey();			//�ͷ�
	}
	ReleaseAllKey();			//�ͷ�
	
}


//�ַ�ת����ֵ
static void Char2KeyVal(uint8_t chara)
{
	char (*p)[5] = KeyValDef;
	sprintf((char*)BTKeyVal+9,"%s",p[chara]);
	sprintf((char*)BTKeyVal+13,"%c",'\r');
}

	