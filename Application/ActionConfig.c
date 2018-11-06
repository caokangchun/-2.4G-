#include <stdlib.h>
#include <string.h>
#include "MM32F103.h"
#include "Common.h"
#include "FIFO.h"
#include "BSP.h"
#include "ActionConfig.h"
#include "BSPEEPROM.h"
#include "BSPBeep.h"


#define SET_SIZE				sizeof(Dev_Config)
	
#define SET_2(a,b)				((a)<<8|(b))
#define SET_3(a,b,c)			((a)<<16|(b)<<8|(c))
#define SET_4(a,b,c,d)			((a)<<24|(b)<<16|(c)<<8|(d))

Dev_Config ScanSet;
MyString_Typedef MyString;
Setting_Type Setting=NoSet;			//������״̬

//���ֱ�������ǰ��Ӹ�u���������
uint8_t uInsertGroup;				//�����飬�ڼ���		
uint8_t ulastFlag=0;				//���
uint8_t uSettingFlag=0;				//�����б��

static void ActionConfigCodeInit(void);						//ǰ׺��׺��ǰ���ù��ܳ�ʼ��
static void ActionConfigReadDev(void);						//��EEPROM��ȡ��������
static bool ActionConfigSaveDev(void);						//�����EEPROM��������
static uint8_t SetFunction(uint32_t);						//ScanSet�ṹ�����ø��ģ���д��FLASH
static uint8_t CheckSetting(uint8_t *,uint16_t );			//Ԥ�жϴ���
//static void FinalDataProcess(uint8_t *Buf,uint16_t len);	//�������մ���,���õ����������ݰ������ý��д���.

/*
static void TransmitPre(uint8_t code);						//����ǰ׺
static void TransmitSuf(uint8_t code);						//�����׺
static void TransmitPrior(uint8_t code);					//���䱣���ַ�
static void TransmitnNum(uint32_t num,uint8_t n);			//�������֣���ֻ����UserID
static void TransmitInsert(uint8_t num);					//�������
static void TransmitInsertSuffix(uint8_t num);				//��������׺
static void TransmitSuffix2(void);							//�����׺
static void LetterStatusTransform(uint8_t *str,uint16_t n);	//��ĸ��Сд��ת
static void DataMergeOutput(uint8_t *Buf,Beep_Type beep);	//�����ݺϲ�,�����
*/
static void SetBeepSound(void);								//�����������ͬʱ�����FLASH




//���ñ������������󣬽����ݱ����FLASH
void SetBeepSound(void)
{
	SetBeepTone();
	ActionConfigSaveDev();
}

//��Set���ݱ��浽�û���ַ
void UserSetSave(void)
{
	BSPEEPROMWrite(EEPROM_USER_ADDR,(uint16_t *)&ScanSet, SET_SIZE/2);
}

//���û���ַ�����ݷ���Set.
void UserSetInit(void)
{
	BSPEEPROMReadBytes(EEPROM_USER_ADDR, (uint16_t *)&ScanSet,SET_SIZE/2);
	if(BSPEEPROMRead(EEPROM_FIRST_USE)==0xffff)
	{
		ActionConfigDevInit();

		UserSetSave();
	}
}


//ǰ׺��׺��ǰ���ù��ܳ�ʼ��
void ActionConfigCodeInit(void)
{
	ScanSet.UserIDVar 	= 0;
	ScanSet.CodeType 	= OFF;
	ScanSet.UserID		= OFF;
	ScanSet.STX 		= OFF;
	ScanSet.ETX 		= OFF;
	ScanSet.IDPre		= OFF;
	ScanSet.IDSuf 		= OFF;
	ScanSet.HidePre 	= OFF;
	ScanSet.HideSuf 	= OFF;
	ScanSet.KeepPre 	= OFF;
	ScanSet.KeepSuf 	= OFF;
	ScanSet.AddPre 		= OFF;
	ScanSet.AddSuf 		= OFF;
	ScanSet.String_Inversion = OFF;
	ScanSet.Letter_Status = Normal;
	
	ScanSet.Insert = OFF;
	ScanSet.Delete = OFF;
	ScanSet.Replace= OFF;
	Setting=NoSet;
	
	ScanSet.AddPreLen 		= 0;
	ScanSet.AddSufLen 		= 0;
	ScanSet.HidePreLen 		= 0;
	ScanSet.HideSufLen 		= 0;
	ScanSet.KeepPreLen		= 0;
	ScanSet.KeepSufLen		= 0;
	ScanSet.DeleteLen  		= 0;
	ScanSet.ReplaceOldLen	= 0;
	ScanSet.ReplaceNewLen	= 0;
	
	ScanSet.EAN13.uState 	= OFF;
	ScanSet.EAN8.uState		= OFF;
	ScanSet.UPCA.uState 	= OFF;
	ScanSet.UPCE0.uState 	= OFF;
	ScanSet.Code39.uState 	= OFF;
	ScanSet.Code93.uState 	= OFF;
	ScanSet.Code128.uState 	= OFF;
	ScanSet.CodeBar.uState 	= OFF;
	ScanSet.Interleaved2of5.uState 	= OFF;
	ScanSet.Industrial25.uState		= OFF;
	ScanSet.MSI.uState 		= OFF;
	ScanSet.Code11.uState 	= OFF;
	
	memset(ScanSet.AddPreCodeBuf,0,ADD_CODE_NUM);
	memset(ScanSet.AddSufCodeBuf,0,ADD_CODE_NUM);
	memset(ScanSet.InsertAddr,0,INSERT_GROUP);
	memset(ScanSet.InsertLen,0,INSERT_GROUP);
	memset(ScanSet.InsertCodeBuf,0,INSERT_GROUP*INSERT_NUM);
	memset(ScanSet.DeleteCodeBuf,0,DELETE_NUM);
	memset(ScanSet.ReplaceOldCodeBuf,0,REPLACE_NUM);
	memset(ScanSet.ReplaceNewCodeBuf,0,REPLACE_NUM);	
	
//	memcpy(ScanSet.AddPreCodeBuf, "ssd", ADD_CODE_NUM);
//	memcpy(ScanSet.AddSufCodeBuf, "kkkluyfgd", ADD_CODE_NUM);

}

//�������ܳ�ʼ��
void ActionConfigDevInit(void)
{
	memset(&ScanSet,0,SET_SIZE);
	
	ScanSet.BeepSound= ON;
	ScanSet.StartSound = ON;
	ScanSet.BeepVol= BEEP_VOL_BIG;
	ScanSet.BeepLenght= BEEP_TONE_CENTRE;
		
	ScanSet.ScanMode= _MODE_MANUAL;						//ɨ��ģʽ
	ScanSet.ScanTail= SUF_TAIL_CR;						//ɨ���׺������������׺��
	ScanSet.ScanInterval= INTERVAL_0MS;					//����ɨ����
	ScanSet.UploadMode=Normal_Mode;						//�ϴ�ģʽ
	ScanSet.InventoryData=NULL;							//�̵�����
	ScanSet.Tx_Mode = USB_Mode;							//�ӿ�ģʽ
		
	ScanSet.BaudRate= BaudRate_115200;					//���ڲ�����
	ScanSet.DataBit= DATABIT_8BIT;						//��������λ
	ScanSet.StopBit= STOPBIT_1BIT;						//����ֹͣλ
	ScanSet.ParityBit = PARITYBIT_NONE;					//����У��λ
		
	ScanSet.Collimation= NORMAL;						//��׼����
	ScanSet.Lighting = NORMAL;							//����������
	ScanSet.KeyBoardType = America;
	
	ActionConfigCodeInit();
	
	ActionConfigSaveDev();
}



//set����
bool ActionConfigSaveDev(void)
{
	return(BSPEEPROMWrite(EEPROM_DEV_ADDR,(uint16_t *)&ScanSet, SET_SIZE/2));
}
	
//set��ʼ��
void ActionConfigSetInit(void)
{
	uint16_t addrEnable=1;
	
	BSPEEPROMInit(FLASH_PAGE_SIZE * GD150_FLASH_SIZE, 1*FLASH_PAGE_SIZE);
	
	//��ȡ�û����ã�δ���ù��ͽ��г�ʼ������
	if(BSPEEPROMRead(EEPROM_FIRST_USE)==0xffff)
	{
		ActionConfigDevInit();
		
		//ʹ��д1�����Ѿ����ù�	
		BSPEEPROMWrite(EEPROM_FIRST_USE, &addrEnable, 1);
	}
	else
	{
		ActionConfigReadDev();
	}
	
}


//��ȡ��������
void ActionConfigReadDev(void)
{
	BSPEEPROMReadBytes(EEPROM_DEV_ADDR, (uint16_t *)&ScanSet, SET_SIZE/2);
}


//�����봦��
uint8_t ActionConfigCodeProcess(uint8_t *BuffCode, uint16_t len)
{
	uint16_t i;
	
	if(BuffCode[0]==0x80)
	{
		len=len-1;
	}
//	else
//	{
//		BuffCode--;
//	}
	
	if(CheckSetting(BuffCode, len))	return 1;		//����Ԥ����,�ж��Ƿ���������,�����,��������,��������˳��������
	
	if(BuffCode[1]=='X'&&BuffCode[2]=='M'&&len<=6)   //����Ƿ�������������
	{
		uint32_t T=0;
		
		for(i=3;i<=len;i++)
		{
			T<<=8;
			T|=BuffCode[i];
		}
		
		if(SetFunction(T))	//ѡ���������¼������޸���Ӧ������
		{
			ActionConfigSaveDev();	//�޸���ɱ����FLASH.
			return 1;
		}
	}
	
	return 0;
}

#if 0
//��׺���
void TransmitSuffix2(void)
{
	if(ScanSet.String_Inversion==OFF)
	{
		switch(ScanSet.ScanTail)
		{
			case SUF_TAIL_NULL:	
			{
				break;
			}
			case SUF_TAIL_CR:		
			{				
				TransmitSuf('\r');
				break;
			}
			case SUF_TAIL_CRLF:	
			{
				TransmitSuf('\r');
				TransmitSuf('\n');
				break;
			}
			case SUF_TAIL_TAB:		
			{
				TransmitSuf('\t');
				break;
			}
			default :	break;
		}
	}
	else
	{
		switch(ScanSet.ScanTail)
		{
			case SUF_TAIL_NULL:	
			{
				break;
			}
			case SUF_TAIL_CR:		
			{
				TransmitPre('\r');
				break;
			}
			case SUF_TAIL_CRLF:	
			{
				TransmitPre('\r');
				TransmitPre('\n');
				break;
			}
			case SUF_TAIL_TAB:	
			{				
				TransmitPre('\t');
				break;
			}
			default :	break;
		}		
	}
}


//����ǰ׺
void TransmitPre(uint8_t code)
{
	if(MyString.PreNum<PRE_CODE_NUM)	
	{
		MyString.PreCodeBuf[MyString.PreNum++]=code;
	}
}

//�����׺
void TransmitSuf(uint8_t code)
{
	if(MyString.SufNum<SUF_CODE_NUM)
	{
		MyString.SufCodeBuf[MyString.SufNum++]=code;
	}
}

//���䱣��
void TransmitPrior(uint8_t code)
{
	if(MyString.PriorNum<PRIOR_CODE_NUM)	
	{
		MyString.PriorCodeBuf[MyString.PriorNum++]=code;
	}
}

//#ifndef	NEWSET
void TransmitnNum(uint32_t num,uint8_t n)
{
	uint8_t i=0,a[5]={0};
	
	while(1)
	{
		a[i]=num%10;
		num/=10;
		if(num>0)	
		{
			i++;
		}
		else
		{
			break;
		}
	}
	
	for(i=n;i>0;i--)
	{
		TransmitPre(a[i-1]+0x30);
	}
}

//�������
void TransmitInsert(uint8_t num)
{
	uint8_t n,m;
	
	for(n=0;n<INSERT_GROUP;n++)
	{
		if(ScanSet.InsertAddr[n]==num)
		{
			for(m=0;m<ScanSet.InsertLen[n];m++)
			{
				TransmitPrior(ScanSet.InsertCodeBuf[n][m]);
			}
		}
	}
}

//��������׺
void TransmitInsertSuffix(uint8_t num)
{
	uint8_t i,j,temp,n=0,a[INSERT_GROUP];
	
	for(i=0;n<INSERT_GROUP;i++)
	{
		if(ScanSet.InsertAddr[i]>=num)
		{
			a[n]=ScanSet.InsertAddr[i];
			n++;
		}
	}
	
	if(n==0)	return;
	
	for(i=0;i<n;i++)
	{
		for(j=0;j<n-i-1;j++)
		{
			if(a[j]>a[j+1])
			{
				temp=a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
			}
		}
	}
	
	for(i=0;i<n;i++)
	{
		TransmitInsert(a[i]);
	}
}
//#endif

//�ַ���ת
void LetterStatusTransform(uint8_t *str,uint16_t n)
{
	uint16_t i;
	
	switch(ScanSet.Letter_Status)
	{
		case Normal:		
		{
			break;
		}
		case Capital:			
		{
			for(i=0;i<n;i++)
			{
				if(*str>=0x61&&*str<=0x7A)
				{
					*str-=0x20;													
				}
				str++;
			}
			break;
		}
		case Lowercase:		
		{	
			for(i=0;i<n;i++)
			{
				if(*str>=0x41&&*str<=0x5A)
				{
					*str+=0x20;
				}
				str++;
			}
			break;
		}
		case Inversion:		
		{	
			for(i=0;i<n;i++)
			{
				if(*str>=0x61&&*str<=0x7A)
				{
					*str-=0x20;
				}
				else if(*str>=0x41&&*str<=0x5A)
				{
					*str+=0x20;
				}
				str++;
			}
			break;
		}									
		default: break;
	}
}
#endif

//����Ԥ����,�ж��Ƿ���������,�����,�����ַ�������,
//���ɨ����������,���˳��������.
uint8_t CheckSetting(uint8_t BuffCode[],uint16_t len)
{
	uint8_t flag=0,j=1;
	
	if(Setting!=NoSet)            //������״̬
	{
//		key_flag=1;
		
		if(BuffCode[1]=='X'&&BuffCode[2]=='M'&&len<=7)           //��������������
		{	
			Setting=NoSet;
			uSettingFlag=0;
			return 0;
		}
		
		if(len==3&&BuffCode[1]=='$')                              //���������ַ�����ʽΪ��$ + ��λ��ASCII��ʮ�����ơ�
		{
			uint16_t i,a[2];
			
			for(i=0;i<2;i++)
			{
				if(BuffCode[i+2]>='0'&&BuffCode[i+2]<='9')
				{
					a[i]=BuffCode[i+2]-0x30;
				}
				else if(BuffCode[i+2]>='A'&&BuffCode[i+2]<='F')
				{
					a[i]=BuffCode[i+2]-55;
				}
				else                                 //�������ǳ����ַ���Χ��ֵ
				{
					Setting=NoSet;              
					uSettingFlag=0;
					return 0;
				}
			}
			
			BuffCode[1]=a[0]<<4|a[1];      //��λ��ASCII��ʮ������
			len=j;
			flag=1;                         //1������������ַ���,0�������������ͨ��
		}

		if(Setting==Add_Pre)              //״̬Ϊ�����ǰ׺
		{
			if(flag)
			{
				if(ScanSet.AddPreLen<ADD_CODE_NUM)
				{
					SetBeepSound();
					uSettingFlag=1;
					ScanSet.AddPreCodeBuf[ScanSet.AddPreLen++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				ScanSet.AddPre=ON;
				return 1;
			}
			else
			{
				if(uSettingFlag)                 //�Ѽӹ�һ���ַ����ַ�
				{
					Setting=NoSet;
					uSettingFlag=0;
					return 0;
				}
				
				if(len-j+1<=ADD_CODE_NUM)				
				{
					memset(ScanSet.AddPreCodeBuf,0,ADD_CODE_NUM);
					for(ScanSet.AddPreLen=0;ScanSet.AddPreLen<len+1-j;ScanSet.AddPreLen++)					
					{
						ScanSet.AddPreCodeBuf[ScanSet.AddPreLen]=BuffCode[ScanSet.AddPreLen+j];
					}
					ScanSet.AddPre=ON;
					SetBeepSound();					
				}
				else
				{
					ErrorBeepSound(Setting_Beep);					
				}

				Setting=NoSet;				
				return 1;
			}
		}
		else if(Setting==Add_Suf)
		{
			if(flag)
			{
				if(ScanSet.AddSufLen<ADD_CODE_NUM)
				{
					SetBeepSound();
					uSettingFlag=1;
					ScanSet.AddSufCodeBuf[ScanSet.AddSufLen++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				ScanSet.AddSuf=ON;
				return 1;
			}
			else
			{
				if(uSettingFlag)
				{
					Setting=NoSet;
					uSettingFlag=0;
					return 0;
				}

				if(len-j+1<=ADD_CODE_NUM)				
				{				
					memset(ScanSet.AddSufCodeBuf,0,ADD_CODE_NUM);
					for(ScanSet.AddSufLen=0;ScanSet.AddSufLen<len+1-j;ScanSet.AddSufLen++)					
					{
						ScanSet.AddSufCodeBuf[ScanSet.AddSufLen]=BuffCode[ScanSet.AddSufLen+j];
					}		
					
					ScanSet.AddSuf=ON;
					SetBeepSound();
				}
				else
				{
					ErrorBeepSound(Setting_Beep);						
				}
				
				Setting=NoSet;				
				return 1;				
			}
		}
		else if(Setting==InsertCodeBuf)
		{
			if(flag)
			{
				if(ScanSet.InsertLen[uInsertGroup]<INSERT_NUM)
				{
					SetBeepSound();
					ScanSet.Insert=ON;
					uSettingFlag=1;
					ScanSet.InsertCodeBuf[uInsertGroup][ScanSet.InsertLen[uInsertGroup]++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				return 1;
			}
			else
			{
				if(uSettingFlag)
				{
					ScanSet.Insert=ON;
					uSettingFlag=0;
					Setting=NoSet;
					return 0;
				}
				
				if(len-j+1<=INSERT_NUM)
				{
					uint16_t i;
					
					memset(ScanSet.InsertCodeBuf[uInsertGroup],0,INSERT_NUM);
					ScanSet.InsertLen[uInsertGroup]=len-j+1;
					for(i=0;i<ScanSet.InsertLen[uInsertGroup];i++)
					{
						ScanSet.InsertCodeBuf[uInsertGroup][i]=BuffCode[i+j];
					}
					
					ScanSet.Insert=ON;
					SetBeepSound();
				}
				else
				{
					ErrorBeepSound(Setting_Beep);
				}
				
				Setting=NoSet;
				return 1;
			}
		}
		else if(Setting==User_ID)
		{
			if((BuffCode[len]>='0'&&BuffCode[len]<='9')&&(j==len))			
			{
				if(uSettingFlag==0)
				{
					ScanSet.UserIDVar=BuffCode[len]-0x30;
					uSettingFlag=1;
					SetBeepSound();
				}
				else
				{
					ScanSet.UserIDVar=ScanSet.UserIDVar*10+BuffCode[len]-0x30;
					ScanSet.UserID=ON;
					uSettingFlag=0;
					Setting=NoSet;
					SetBeepSound();
				}
				
				return 1;
			}
			else
			{
				uSettingFlag=0;
				Setting=NoSet;
				return 0;
			}
		}
		else if(Setting==Delete)
		{
			if(flag)
			{
				if(ScanSet.DeleteLen<DELETE_NUM)
				{
					SetBeepSound();
					uSettingFlag=1;
					ScanSet.DeleteCodeBuf[ScanSet.DeleteLen++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				ScanSet.Delete=ON;
				return 1;
			}
			else
			{
				if(uSettingFlag)
				{
					Setting=NoSet;
					uSettingFlag=0;
					return 0;
				}
				
				if(len-j+1<=DELETE_NUM)
				{
					memset(ScanSet.DeleteCodeBuf,0,DELETE_NUM);
					for(ScanSet.DeleteLen=0;ScanSet.DeleteLen<len+1-j&&ScanSet.DeleteLen<DELETE_NUM;ScanSet.DeleteLen++)
					{
						ScanSet.DeleteCodeBuf[ScanSet.DeleteLen]=BuffCode[ScanSet.DeleteLen+j];
					}
					
					ScanSet.Delete=ON;
					SetBeepSound();
				}
				else
				{
				  ErrorBeepSound(Setting_Beep);
				}
				
				Setting=NoSet;
				return 1;					
			}
		}
		else if(Setting==Replace_Old)
		{
			if(flag)
			{
				if(ScanSet.ReplaceOldLen<REPLACE_NUM)
				{
					SetBeepSound();
					uSettingFlag=1;
					ScanSet.ReplaceOldCodeBuf[ScanSet.ReplaceOldLen++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				if(ScanSet.ReplaceNewLen>0)	ScanSet.Replace=ON;
				return 1;
			}
			else
			{
				if(uSettingFlag)
				{
					Setting=NoSet;
					uSettingFlag=0;
					return 0;
				}

				if(len-j+1<=REPLACE_NUM)
				{
					memset(ScanSet.ReplaceOldCodeBuf,0,REPLACE_NUM);
					for(ScanSet.ReplaceOldLen=0;ScanSet.ReplaceOldLen<len+1-j&&ScanSet.ReplaceOldLen<REPLACE_NUM;ScanSet.ReplaceOldLen++)
					{
						ScanSet.ReplaceOldCodeBuf[ScanSet.ReplaceOldLen]=BuffCode[ScanSet.ReplaceOldLen+j];
					}
					
					if(ScanSet.ReplaceNewLen>0)	ScanSet.Replace=ON;
					SetBeepSound();
				}
				else
				{
					ErrorBeepSound(Setting_Beep);
				}
				
				Setting=NoSet;
				return 1;					
			}
		}
		else if(Setting==Replace_New)
		{
			if(flag)
			{
				if(ScanSet.ReplaceNewLen<REPLACE_NUM)
				{
					SetBeepSound();
					uSettingFlag=1;
					ScanSet.ReplaceNewCodeBuf[ScanSet.ReplaceNewLen++]=BuffCode[j];
					ActionConfigSaveDev();
				}
				else
				{
					Setting=NoSet;
					uSettingFlag=0;
					ErrorBeepSound(Setting_Beep);
				}
				
				if(ScanSet.ReplaceOldLen>0)	ScanSet.Replace=ON;
				return 1;
			}
			else
			{
				if(uSettingFlag)
				{
					Setting=NoSet;
					uSettingFlag=0;
					return 0;
				}

				if(len-j+1<=REPLACE_NUM)
				{				
					memset(ScanSet.ReplaceNewCodeBuf,0,REPLACE_NUM);
					for(ScanSet.ReplaceNewLen=0;ScanSet.ReplaceNewLen<len+1-j&&ScanSet.ReplaceNewLen<REPLACE_NUM;ScanSet.ReplaceNewLen++)
					{
						ScanSet.ReplaceNewCodeBuf[ScanSet.ReplaceNewLen]=BuffCode[ScanSet.ReplaceNewLen+j];
					}
					
					if(ScanSet.ReplaceOldLen>0)	ScanSet.Replace=ON;
					SetBeepSound();
				}
				else
				{
					ErrorBeepSound(Setting_Beep);
				}
				
				Setting=NoSet;
				return 1;							
			}
		}
//		else if(Setting==Sleep_Time)
//		{
//			Setting=NoSet;
//			return 0;
//		}
		
		if(len==j&&BuffCode[len]>='0'&&BuffCode[len]<='9')                       //���ַ���Ĳ���
		{
			if(Setting==Hid_Pre||Setting==Hid_Suf)                             //ǰ/����������
			{
				if(Setting==Hid_Pre)
				{
					if(uSettingFlag==0)	ScanSet.HidePreLen=BuffCode[len]-0x30;
					else
					{
						ScanSet.HidePreLen=ScanSet.HidePreLen*10+BuffCode[len]-0x30;
						ScanSet.HidePre=ON;          //ǰ���ع�����Ч
					}
				}
				else
				{
					if(uSettingFlag==0)	ScanSet.HideSufLen=BuffCode[len]-0x30;
					else
					{
						ScanSet.HideSufLen=ScanSet.HideSufLen*10+(BuffCode[len]-0x30);
						ScanSet.HideSuf=ON;          //�����ع�����Ч
					}                          
				}
				
				if(uSettingFlag==0)          
				{
//					Beep();
					SetBeepSound();
					uSettingFlag=1;            //��ʾ�����˵�1λ�ַ�������
				}
				else                         //��ʾ�����˵�2λ�ַ������֣����α������ܣ����⣩
				{                            
					ScanSet.KeepPre=OFF;
					ScanSet.KeepPreLen=0;
					ScanSet.KeepSuf=OFF;
					ScanSet.KeepSufLen=0;
					Setting=NoSet;
					uSettingFlag=0;
					SetBeepSound();
				}
			}
			else if(Setting==Keep_Pre||Setting==Keep_Suf)                      //ǰ/��������
			{
				if(Setting==Keep_Pre)
				{
					if(uSettingFlag==0)	ScanSet.KeepPreLen=BuffCode[len]-0x30;
					else
					{
						ScanSet.KeepPreLen=ScanSet.KeepPreLen*10+BuffCode[len]-0x30;
						ScanSet.KeepPre=ON;         //ǰ����������Ч
					}
				}
				else
				{
					if(uSettingFlag==0)	ScanSet.KeepSufLen=BuffCode[len]-0x30;
					else
					{
						ScanSet.KeepSufLen=ScanSet.KeepSufLen*10+(BuffCode[len]-0x30);
						ScanSet.KeepSuf=ON;         //����������Ч
					}
				}
				
				if(uSettingFlag==0)
				{
//					Beep();
					SetBeepSound();
					uSettingFlag=1;            //��ʾ�����˵�1λ�ַ�������
				}
				else
				{
					ScanSet.HidePre=OFF;           //��ʾ�����˵�2λ�ַ������֣��������ع��ܣ����⣩
					ScanSet.HidePreLen=0;
					ScanSet.HideSuf=OFF;
					ScanSet.HideSufLen=0;
					Setting=NoSet;
					uSettingFlag=0;
					SetBeepSound();
				}
			}
			else if(Setting==Insert_Group)                                 //��������
			{
				if(uSettingFlag==0)                                          //�������ǵ�һλ�ַ�������
				{
					ScanSet.InsertAddr[uInsertGroup]=BuffCode[len]-0x30;
					uSettingFlag=1;
					SetBeepSound();
				}
				else                                                         //�������ǵڶ�λ�ַ�������
				{
					ScanSet.InsertAddr[uInsertGroup]*=10;
					ScanSet.InsertAddr[uInsertGroup]+=BuffCode[len]-0x30;
					Setting=InsertCodeBuf;
					ScanSet.InsertLen[uInsertGroup]=0;                              //�����ַ�����ͳ������
					memset(ScanSet.InsertCodeBuf[Insert_Group],0,INSERT_NUM);        //��������������
					uSettingFlag=0;
					SetBeepSound();
				}
			}
			else if(Setting==Limit_Lower)
			{
				if(uSettingFlag==0)
				{
					ScanSet.LimitLowerLen=BuffCode[len]-0x30;
					uSettingFlag=1;
					SetBeepSound();
				}
				else
				{
					ScanSet.LimitLowerLen*=10;
					ScanSet.LimitLowerLen+=BuffCode[len]-0x30;
					Setting=NoSet;
					uSettingFlag=0;
					SetBeepSound();
				}
			}
			else if(Setting==Limit_Upper)
			{
				if(uSettingFlag==0)
				{
					ScanSet.LimitUpperLen=BuffCode[len]-0x30;
					uSettingFlag=1;
					SetBeepSound();
				}
				else
				{
					ScanSet.LimitUpperLen*=10;
					ScanSet.LimitUpperLen+=BuffCode[len]-0x30;
					Setting=NoSet;
					uSettingFlag=0;
					SetBeepSound();
				}
			}
//			else if(Setting==Inventory_Time)
//			{
//				if(uSettingFlag==0)
//				{
//					Beep();
//					ScanSet.Inventory_Time=BuffCode[len]-0x30;
//					uSettingFlag=1;
//				}
//				else
//				{
//					ScanSet.Inventory_Time*=10;
//					ScanSet.Inventory_Time+=BuffCode[len]-0x30;
//					Setting=NoSet;
//					uSettingFlag=0;
//					SetBeepSound();
//				}
//			}
			
			return 1;
		}
		else if(uSettingFlag)
		{
			if(Setting==Hid_Pre||Setting==Hid_Suf)	//����ǰ���ػ������
			{
				if(Setting==Hid_Pre)
				{
					ScanSet.HidePre=ON;
				}
				else
				{
					ScanSet.HideSuf=ON;
				}
				
				ScanSet.KeepPre=OFF;
				ScanSet.KeepPreLen=0;
				ScanSet.KeepSuf=OFF;
				ScanSet.KeepSufLen=0;
			}
			else if(Setting==Keep_Pre||Setting==Keep_Suf)	//����ǰ���������
			{
				if(Setting==Keep_Pre)
				{
					ScanSet.KeepPre=ON;
				}
				else
				{
					ScanSet.KeepSuf=ON;
				}
				
				ScanSet.HidePre=OFF;
				ScanSet.HidePreLen=0;
				ScanSet.HideSuf=OFF;
				ScanSet.HideSufLen=0;
			}
			else if(Setting==Insert_Group)	//��������
			{
				Setting=InsertCodeBuf;
				ScanSet.InsertLen[uInsertGroup]=0;
				memset(ScanSet.InsertCodeBuf[Insert_Group],0,INSERT_NUM);
				uSettingFlag=0;
				
				return CheckSetting(BuffCode,len);
			}
			
			ActionConfigSaveDev();
		}
		
		Setting=NoSet;
		uSettingFlag=0;
		
	}
	return 0;
	

}






uint8_t SetFunction(uint32_t temp)
{	
	switch(temp)
	{
		case SET_2('B','B'):	//�汾��Ϣ
		{
			
			SetBeepSound();
			break;
		}
		case SET_2('C','C'):	//��������
		{	
			ActionConfigDevInit();
			SetBeepSound();
			break;
		}			
		case SET_2('Y','H'):	//�û����ó�ʼ��
		{
			UserSetInit();
			SetBeepSound();	
			break;
		}
		case SET_4('Y','H','B','C'):	//�û����ñ���
		{
			UserSetSave();
			SetBeepSound();	
			break;
		}
		
		//����
		case SET_4('K','J','S','0'):	
		{
			ScanSet.StartSound=OFF;
			SetBeepSound();
			break;																	
		}
		case SET_4('K','J','S','1'):	
		{
			ScanSet.StartSound=ON;
			SetBeepSound();
			break;	
		}
		case SET_3('S','Y','0'):	
		{
			ScanSet.BeepSound=OFF;
////			SendCommandToCmos(COMMAND_MUTE_ON,GetReturn);
//			BSPCodeScannerReadSuccessBeepControl(DISABLE);
			SetBeepSound();
			break;
		}
		case SET_3('S','Y','1'):	
		{
			ScanSet.BeepSound=ON;
////			SendCommandToCmos(COMMAND_MUTE_OFF,GetReturn);
//			BSPCodeScannerReadSuccessBeepControl(ENABLE);
			SetBeepSound();
			break;
		}
		case SET_3('S','Y','D'):	
		{
			ScanSet.BeepVol=BEEP_VOL_BIG;
			SetBeepSound();
			break;
		}
		case SET_3('S','Y','X'):		
		{
			ScanSet.BeepVol=BEEP_VOL_SMALL;
			SetBeepSound();
			break;		
		}
		case SET_3('Y','C','1'):
		{
			ScanSet.BeepLenght=BEEP_TONE_SHORT;
			SetBeepSound();
			break;
		}
		case SET_3('Y','C','2'):
		{
			ScanSet.BeepLenght=BEEP_TONE_CENTRE;
			SetBeepSound();
			break;
		}
		case SET_3('Y','C','3'):
		{
			ScanSet.BeepLenght=BEEP_TONE_LONG;
			SetBeepSound();
			break;
		}
		
//ɨ��ģʽ
		case SET_3('M','S','1'):
		{			
			ScanSet.ScanMode=_MODE_MANUAL;                                  //������ģʽ
//			BSPCodeScannerSetScanMode(MODE_MANUAL);
			SetBeepSound();
			break;		
		}
		case SET_3('M','S','5'):
		{		
			ScanSet.ScanMode=_MODE_CONTINUAL;								
//			BSPCodeScannerSetScanMode(MODE_CONTINUAL);				//����ģʽ
//			BSPCodeScannerSetSingleReadDuration(0);					//���õ��ζ���ʱ��Ϊ���޳�
//			BSPCodeScannerSetSameBarcodeReadDelayDuration(0x01);	//����ͬ����ʱ��Ϊ0.1��
			SetBeepSound();
			break;
		}
		case SET_3('M','S','7'):
		{
			
			ScanSet.ScanMode=_MODE_INDUCTION;                            	//��Ӧģʽ
//			BSPCodeScannerSetScanMode(MODE_INDUCTION);
//			BSPCodeScannerSetSingleReadDuration(0);			//���õ��ζ���ʱ��Ϊ���޳�
//			BSPCodeScannerGetSensitivity1(0);				//����������Ϊ0(���)
//			BSPCodeScannerSetSameBarcodeReadDelayDuration(50);//����ͬ����ʱ��				
			SetBeepSound();		
			break;
		}
		//��׺����������	
		case SET_3('H','Z','0'):	
		{
			ScanSet.ScanTail=SUF_TAIL_NULL;
			SetBeepSound();
			break;
		}
		case SET_3('H','Z','1'):	
		{
			ScanSet.ScanTail=SUF_TAIL_CR;
			SetBeepSound();
			break;
		}
		case SET_3('H','Z','3'):	
		{
			ScanSet.ScanTail=SUF_TAIL_CRLF;
			SetBeepSound();
			break;
		}
		case SET_3('H','Z','4'):	
		{
			ScanSet.ScanTail=SUF_TAIL_TAB;
			SetBeepSound();
			break;
		}
		
//�ӿ�ģʽ
		case SET_3('J','K','1')://����δ��		
		{
//			uart_init();	
			
			ScanSet.Tx_Mode=Usart_Mode;
			SetBeepSound();		
			break;
		}	
		case SET_3('J','K','2'):			
		{		
			ScanSet.Tx_Mode=USB_Mode;	
			SetBeepSound();		
//			usb_init();																	
			break;											
		}
		//PS2ģʽ������ģʽδ��
//		case SET_3('J','K','4'):			
//		{
//			if((ScanSet.Tx_Mode==Usart_Mode||ScanSet.Tx_Mode==USB_VCP_Mode)&&ScanSet.ScanTail==SUF_TAIL_CRLF)	Set.Suffix=Suffix_LF;
//			
//			ScanSet.Tx_Mode=PS2_Mode;	
//			SetBeepSound();
//			break;		
//		}
//		case SET_3('J','K','0'):			
//		{
//			ScanSet.Tx_Mode=Wireless_Mode;
//			SetBeepSound();
//			break;
//		}
		
		
		//***************//
		//���ڲ���		
		case SET_4('B','T','L','1'):
		{
			ScanSet.BaudRate=BaudRate_1200;	
			SetBeepSound();
			break;
		}
		case SET_4('B','T','L','2'):	
		{
			ScanSet.BaudRate=BaudRate_2400;	
			SetBeepSound();
			break;
		}																
		case SET_4('B','T','L','3'):	
		{
			ScanSet.BaudRate=BaudRate_4800;	
			SetBeepSound();
			break;
		}
		case SET_4('B','T','L','4'):	
		{
			ScanSet.BaudRate=BaudRate_9600;
			SetBeepSound();
			break;
		}
		case SET_4('B','T','L','5'):	
		{
			ScanSet.BaudRate=BaudRate_19200;
			SetBeepSound();					
			break;
		}
		case SET_4('B','T','L','6'):	
		{
			ScanSet.BaudRate=BaudRate_38400;
			SetBeepSound();
			break;
		}
		case SET_4('B','T','L','7'):	
		{
			ScanSet.BaudRate=BaudRate_57600;
			SetBeepSound();
			break;			
		}
		case SET_4('B','T','L','8'):	
		{
			ScanSet.BaudRate=BaudRate_115200;
			SetBeepSound();
			break;
		}

		//����λ
		case SET_4('S','J','W','1'):	
		{
			ScanSet.DataBit=DATABIT_7BIT;
			SetBeepSound();
			break;
		}
		case SET_4('S','J','W','2'):	
		{
			ScanSet.DataBit=DATABIT_8BIT;
			SetBeepSound();
			break;
		}
		//ֹͣλ
		case SET_4('T','Z','W','1'):	
		{
			ScanSet.StopBit=STOPBIT_1BIT;
			SetBeepSound();
			break;
		}
		case SET_4('T','Z','W','2'):	
		{
			ScanSet.StopBit=STOPBIT_2BIT;
			SetBeepSound();
			break;
		}
		//У��λ
		case SET_4('J','Y','W','0'):	
		{
			ScanSet.ParityBit=PARITYBIT_NONE;
			SetBeepSound();
			break;
		}
		case SET_4('J','Y','W','1'):	
		{
			ScanSet.ParityBit=PARITYBIT_ODD;
			SetBeepSound();
			break;
		}
		case SET_4('J','Y','W','2'):	
		{	
			ScanSet.ParityBit=PARITYBIT_EVEN;
			SetBeepSound();
			break;
		}
		//*************//


		//����ɨ����
		
		
		//�ϴ�ģʽ
		case SET_3('P','D','0'):
		{
			ScanSet.UploadMode=Normal_Mode;
			SetBeepSound();
			break;
		}
		case SET_3('P','D','1'):
		{
			ScanSet.UploadMode=Inventory_Mode;
			SetBeepSound();
			break;
		}

		case SET_3('P','D','S'):			
		{
//			str="number:";
//			MyString.uNum=0;
//			Transmit_String(str);
//			Transmit_Num(Inventory_n);
//			TransmitSuffix();
//			DataMergeOutput();
			SetBeepSound();
			break;
		}
		case SET_3('P','D','L'):			
		{
//			str="size:";
//			MyString.uNum=0;
//			Transmit_String(str);
//			Transmit_Num(Inventory_m);
//			TransmitSuffix();
//			DataMergeOutput();
			SetBeepSound();
			break;
		}
		case SET_3('P','D','Y'):			
		{
//			str="remainder:";
//			MyString.uNum=0;
//			Transmit_String(str);
//			Transmit_Num(INVENTORY_SIZE-Inventory_m);
//			TransmitSuffix();
//			DataMergeOutput();
			SetBeepSound();
			break;
		}
		case SET_4('P','D','S','C'):	
		{
//			LAS(0);
//			Transmit_Inventory();
//			LAS(1);		
			SetBeepSound();
			break;
		}
		case SET_4('P','D','Q','C'):	
		{
//			LAS(0);		
//			Inventory_Delete();
//			SetBeepSound_NoSave();		
//			LAS(1);		
			SetBeepSound();
			break;
		}
		case SET_4('P','D','C','X'):	
		{
//			Inventory_Repeal();
//			SetBeepSound_NoSave();	
			SetBeepSound();			
			break;		
		}
		

		//��׼����
		case SET_3('M','Z','1'):	
		{
//			BSPCodeScannerSetCollimation(COLLIMATE_NORMAL);		//��ͨ
			ScanSet.Collimation=NORMAL;
			SetBeepSound();
			break;
		}
		case SET_3('M','Z','2'):	
		{
//			BSPCodeScannerSetCollimation(COLLIMATE_ALWAYS);		//����
			ScanSet.Collimation=ALWAYS;
			SetBeepSound();
			break;
		}
		case SET_3('M','Z','0'):	
		{
//			BSPCodeScannerSetCollimation(COLLIMATE_NONE);		//����׼
			ScanSet.Collimation=NONE;
			SetBeepSound();
			break;
		}
		case SET_3('Z','M','1'):	
		{
//			BSPCodeScannerSetLight(LIGHT_NORMAL);				//��ͨ
			ScanSet.Lighting=NORMAL;
			SetBeepSound();
			break;
		}
		case SET_3('Z','M','2'):	
		{
//			BSPCodeScannerSetLight(LIGHT_ALWAYS);				//����
			ScanSet.Lighting=ALWAYS;
			SetBeepSound();
			break;
		}
		case SET_3('Z','M','0'):	
		{
//			BSPCodeScannerSetLight(LIGHT_NONE);					//����׼
			ScanSet.Lighting=NONE;
			SetBeepSound();
			break;
		}

		
//��������
			
		case SET_2('M','G'):					
		{
			ScanSet.KeyBoardType=America;
			SetBeepSound();
			break;
		}
		case SET_2('F','G'):					
		{
			ScanSet.KeyBoardType=France;
			SetBeepSound();
			break;
		}
		case SET_2('D','G'):					
		{
			ScanSet.KeyBoardType=German;
			SetBeepSound();
			break;	
		}
		case SET_2('F','J'):					
		{
			ScanSet.KeyBoardType=France_CAN;
			SetBeepSound();
			break;	
		}
		case SET_2('H','L'):					
		{
			ScanSet.KeyBoardType=Dutch;
			SetBeepSound();
			break;
		}														
		case SET_2('X','G'):				
		{	
			ScanSet.KeyBoardType=Spanish;
			SetBeepSound();
			break;	
		}
		case SET_2('P','B'):				
		{	
			ScanSet.KeyBoardType=Portuguese_BR;
			SetBeepSound();
			break;
		}															
		case SET_2('Y','D'):					
		{	
			ScanSet.KeyBoardType=Italian;
			SetBeepSound();
			break;																	
		}
		case SET_2('P','T'):				
		{
			ScanSet.KeyBoardType=Portuguese;
			SetBeepSound();
			break;
		}
		case SET_2('T','F'):				
		{
			ScanSet.KeyBoardType=Turkish_F;
			SetBeepSound();
			break;
		}														
		case SET_2('T','Q'):				
		{
			ScanSet.KeyBoardType=Turkish_Q;
			SetBeepSound();
			break;																	
		}
		case SET_2('R','B'):				
		{
			ScanSet.KeyBoardType=Japanese;
			SetBeepSound();
			break;
		}
		case SET_2('E','L'):					
		{	
			ScanSet.KeyBoardType=Russian;
			SetBeepSound();
			break;
		}
		
		


//�û�ID
		case SET_3('U','I','D'):		
		{
			Setting=User_ID;
			SetBeepSound();
			break;
		}
		case SET_4('U','I','D','0'):	
		{	
			ScanSet.UserID=OFF;
			SetBeepSound();
			break;
		}
		case SET_4('U','I','D','1'):	
		{
			ScanSet.UserID=ON;
			SetBeepSound();
			break;	
		}
		//ǰ��ID����//*****//
		case SET_4('I','D','Z','0'):
		{
			ScanSet.IDPre=OFF;
			SetBeepSound();
			break;
		}
		case SET_4('I','D','Z','1'):
		{
			ScanSet.IDPre=ON;
			SetBeepSound();
			break;
		}
		case SET_4('I','D','Y','0'):
		{
			ScanSet.IDSuf=OFF;
			SetBeepSound();
			break;
		}
		case SET_4('I','D','Y','1'):
		{
			ScanSet.IDSuf=ON;
			SetBeepSound();	
			break;	
		}
		//********//
		//�ַ�����
		case SET_4('S','T','X','0'):
		{
			ScanSet.STX=OFF;
			SetBeepSound();
			break;
		}
		case SET_4('S','T','X','1'):
		{
			ScanSet.STX=ON;
			SetBeepSound();
			break;
		}
		case SET_4('E','T','X','0'):
		{
			ScanSet.ETX=OFF;
			SetBeepSound();
			break;
		}
		case SET_4('E','T','X','1'):
		{
			ScanSet.ETX=ON;
			SetBeepSound();
			break;		
		}
		//��ʾ����״̬//******//
		case SET_3('M','X','0'):
		{
			ScanSet.CodeType=OFF;
			SetBeepSound();
			break;
		}
		case SET_3('M','X','1'):
		{										
			ScanSet.CodeType=ON;
			SetBeepSound();
			break;
		}
//����
		case SET_4('Y','C','Z','0'):
		{	
			ScanSet.HidePre=OFF;
			ScanSet.HidePreLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('Y','C','Z','1'):
		{
			Setting=Hid_Pre;
			SetBeepSound();
			break;
		}
		case SET_4('Y','C','Y','0'):
		{	
			ScanSet.HideSuf=OFF;
			ScanSet.HideSufLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('Y','C','Y','1'):
		{
			Setting=Hid_Suf;
			SetBeepSound();
			break;
		}
		//����
		case SET_4('B','L','Z','0'):
		{
			ScanSet.KeepPre=OFF;
			ScanSet.KeepPreLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('B','L','Z','1'):	
		{
			Setting=Keep_Pre;
			SetBeepSound();
			break;
		}
		case SET_4('B','L','Y','0'):
		{
			ScanSet.KeepSuf=OFF;
			ScanSet.KeepSufLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('B','L','Y','1'):	
		{
			Setting=Keep_Suf;
			SetBeepSound();
			break;
		}
		//���
		case SET_4('T','J','Z','0'):	
		{	
			ScanSet.AddPre=OFF;
			memset(ScanSet.AddPreCodeBuf,0,ADD_CODE_NUM);
			ScanSet.AddPreLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('T','J','Z','1'):	
		{
			Setting=Add_Pre;
			SetBeepSound();
			break;
		}
		case SET_4('T','J','Y','0'):
		{
			ScanSet.AddSuf=OFF;
			memset(ScanSet.AddSufCodeBuf,0,ADD_CODE_NUM);
			ScanSet.AddSufLen=0;
			SetBeepSound();
			break;
		}
		case SET_4('T','J','Y','1'):	
		{
			Setting=Add_Suf;
			SetBeepSound();
			break;
		}
		
		//����
		case SET_3('C','R','0'):	
		{
			ScanSet.Insert=OFF;
			memset(ScanSet.InsertAddr,0,INSERT_GROUP);
			memset(ScanSet.InsertLen,0,INSERT_GROUP);
			memset(ScanSet.InsertCodeBuf,0,INSERT_GROUP*INSERT_NUM);
			SetBeepSound();
			break;
		}
		case SET_3('C','R','1'):	
		{
			uInsertGroup=0;
			ScanSet.InsertAddr[0]=0;
			Setting=Insert_Group;
			SetBeepSound();
			break;
		}
		case SET_3('C','R','2'):	
		{
			uInsertGroup=1;
			ScanSet.InsertAddr[1]=0;
			Setting=Insert_Group;
			SetBeepSound();
			break;
		}
		case SET_3('C','R','3'):	
		{
			uInsertGroup=2;
			ScanSet.InsertAddr[2]=0;																	
			Setting=Insert_Group;
			SetBeepSound();
			break;
		}
		case SET_3('C','R','4'):	
		{
			SetBeepSound();
			uInsertGroup=3;
			ScanSet.InsertAddr[3]=0;																	
			Setting=Insert_Group;
			break;
		}
		case SET_3('C','R','5'):	
		{
			SetBeepSound();
			uInsertGroup=4;
			ScanSet.InsertAddr[4]=0;
			Setting=Insert_Group;
			break;
		}															
		case SET_3('C','R','6'):	
		{
			SetBeepSound();
			uInsertGroup=5;
			ScanSet.InsertAddr[5]=0;
			Setting=Insert_Group;
			break;
		}															
		case SET_3('C','R','7'):	
		{
			SetBeepSound();
			uInsertGroup=6;
			ScanSet.InsertAddr[6]=0;
			Setting=Insert_Group;
			break;
		}
		case SET_3('C','R','8'):	
		{
			SetBeepSound();
			uInsertGroup=7;
			ScanSet.InsertAddr[7]=0;
			Setting=Insert_Group;
			break;
		}														
		//ɾ��
		case SET_3('S','C','0'):
		{
			ScanSet.Delete=OFF;
			ScanSet.DeleteLen=0;
			memset(ScanSet.DeleteCodeBuf,0,DELETE_NUM);
			SetBeepSound();
			break;
		}
		case SET_3('S','C','1'):	
		{
			SetBeepSound();
			ScanSet.DeleteLen=0;
			Setting=Delete;
			break;
		}
		//�滻
		case SET_3('T','H','0'):
		{
			ScanSet.Replace=OFF;
			ScanSet.ReplaceOldLen=0;
			ScanSet.ReplaceNewLen=0;
			ScanSet.Replace=OFF;
			memset(ScanSet.ReplaceOldCodeBuf,0,REPLACE_NUM);
			memset(ScanSet.ReplaceNewCodeBuf,0,REPLACE_NUM);
			SetBeepSound();
			break;
		}
		case SET_3('T','H','Q'):	
		{
			SetBeepSound();
			ScanSet.ReplaceOldLen=0;
			Setting=Replace_Old;
			break;
		}
		case SET_3('T','H','H'):	
		{
			SetBeepSound();
			ScanSet.ReplaceNewLen=0;
			Setting=Replace_New;
			break;																	
		}
		//�ָ�����
		case SET_4('H','F','T','M'):
		{
			ActionConfigCodeInit();
			SetBeepSound();
			break;
		}
		
		//��Сд		
		case SET_4('Z','M','D','X'):
		{
			ScanSet.Letter_Status=Capital;
			SetBeepSound();
			break;
		}
		case SET_4('Z','M','X','X'):
		{	
			ScanSet.Letter_Status=Lowercase;
			SetBeepSound();
			break;
		}	
		case SET_4('Z','M','F','Z'):
		{
			ScanSet.Letter_Status=Inversion;
			SetBeepSound();
			break;
		}
		case SET_4('Z','M','H','F'):
		{
			ScanSet.Letter_Status=Normal;
			SetBeepSound();
			break;
		}
		//����
		case	SET_3('D','X','0'):		
		{
			ScanSet.String_Inversion=OFF;
			SetBeepSound();
			break;
		}
		case	SET_3('D','X','1'):	
		{
			ScanSet.String_Inversion=ON;
			SetBeepSound();
			break;
		}
/*
//Ir�Զ���Ӧ
		case SET_3('Z','G','0'):	
		{
			ScanSet.Reaction=OFF;
			SetBeepSound();
			break;
		}
		case SET_3('Z','G','1'):	
		{
			ScanSet.Reaction=ON;
			SetBeepSound();
			break;
		}
//������ʱ����
		case SET_4('A','J','C','0'):	
		{
			ScanSet.Scan_Delay=ScanDelay_1s;
			SD_ParaInit();
			SetBeepSound();
			break;																	
		}
		case SET_4('A','J','C','1'):	
		{
			ScanSet.Scan_Delay=ScanDelay_3s;
			SD_ParaInit();		
			SetBeepSound();
			break;	
		}
		case SET_4('A','J','C','2'):	
		{
			ScanSet.Scan_Delay=ScanDelay_5s;
			SD_ParaInit();		
			SetBeepSound();
			break;
		}
		case SET_4('A','J','C','3'):	
		{
			ScanSet.Scan_Delay=ScanDelay_Inf;
//			SD_ParaInit();		
			SetBeepSound();
			break;		
		}
*/
		//EAN13
		case SET_3('d','0','1'):
		{
			ScanSet.EAN13.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('d','0','0'):
		{
			ScanSet.EAN13.uState = OFF;
			SetBeepSound();
			break;
		}
		//EAN8
		case SET_3('g','0','1'):
		{
			ScanSet.EAN8.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('g','0','0'):
		{
			ScanSet.EAN8.uState = OFF;
			SetBeepSound();
			break;
		}
		//UPCA
		case SET_3('c','0','1'):
		{
			ScanSet.UPCA.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('c','0','0'):
		{
			ScanSet.UPCA.uState = OFF;
			SetBeepSound();
			break;
		}
		//UPCE0
		case SET_3('h','0','1'):
		{
			ScanSet.UPCE0.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('h','0','0'):
		{
			ScanSet.UPCE0.uState = OFF;
			SetBeepSound();
			break;
		}
		//Code39
		case SET_3('b','0','1'):
		{
			ScanSet.Code39.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('b','0','0'):
		{
			ScanSet.Code39.uState = OFF;
			SetBeepSound();
			break;
		}
		//Code93
		case SET_3('y','0','1'):
		{
			ScanSet.Code93.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('y','0','0'):
		{
			ScanSet.Code93.uState = OFF;
			SetBeepSound();
			break;
		}
		//Code128
		case SET_3('j','0','1'):
		{
			ScanSet.Code128.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('j','0','0'):
		{
			ScanSet.Code128.uState = OFF;
			SetBeepSound();
			break;
		}
		//CodeBar
		case SET_3('a','0','1'):
		{	
			ScanSet.CodeBar.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('a','0','0'):
		{
			ScanSet.CodeBar.uState = OFF;
			SetBeepSound();
			break;
		}
		//Interleaved2of5
		case SET_3('e','0','1'):
		{
			ScanSet.Interleaved2of5.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('e','0','0'):
		{
			ScanSet.Interleaved2of5.uState = OFF;
			SetBeepSound();
			break;
		}
		//Industrial25
		case SET_3('i','0','1'):
		{
			ScanSet.Industrial25.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('i','0','0'):
		{
			ScanSet.Industrial25.uState = OFF;
			SetBeepSound();
			break;
		}
		//MSI
		case SET_3('m','0','1'):
		{
			ScanSet.MSI.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('m','0','0'):
		{
			ScanSet.MSI.uState = OFF;
			SetBeepSound();
			break;
		}
		//Code11
		case SET_3('z','0','1'):
		{
			ScanSet.Code11.uState = ON;
			SetBeepSound();
			break;
		}
		case SET_3('z','0','0'):
		{
			ScanSet.Code11.uState = OFF;
			SetBeepSound();
			break;
		}
		


		default :		return 0;
	
		
	}
	
	return	1;
}




