#ifndef	__SCANACTION_H__
#define	__SCANACTION_H__


#include <stdint.h>
#include "Common.h"
#include "BSP.h"


/**
//EEPROM�ڴ��ַԤ����
**/

#define EEPROM_FIRST_USE				0x01			//0x800fc00

#define EEPROM_DEV_ADDR					0x03
#define EEPROM_USER_ADDR				(EEPROM_DEV_ADDR+SET_SIZE)



#define ADD_CODE_NUM					15											//�������15λǰ׺
#define PRE_CODE_NUM					(ADD_CODE_NUM+6+9)         					//���á�2λԤ��λ + 1λSTX + 3λ�û�ID + 15λǰ׺+9λԤ����,
#define SUF_CODE_NUM    				(ADD_CODE_NUM+6+9)         					//���á�15λ��׺ + 1λETX + 2λ������ + 3λԤ��λ+9λԤ����,
#define INSERT_GROUP					8											//��������
#define INSERT_NUM						10											//�ɲ��볤��
#define DELETE_NUM						10											//��ɾ������
#define REPLACE_NUM						10											//���滻����
#define PRIOR_CODE_NUM					(ADD_CODE_NUM+INSERT_GROUP*INSERT_NUM)		//95


//����״̬
typedef enum
{	
	OFF,	
	ON	
} 
Status_type; 

//����״̬
typedef enum
{	
	NoSet,				//��
	Hid_Pre,			//ǰ����
	Hid_Suf,			//������
	Keep_Pre,			//ǰ����
	Keep_Suf,			//����
	Add_Pre,			//���ǰ׺
	Add_Suf,			//��Ӻ�׺
			
	Insert_Group,		//������
	InsertCodeBuf,		//�����ַ�
	Limit_Lower,		//��С����
	Limit_Upper,		//��󳤶�
	User_ID,			//�û�ID
	Delete,				//ɾ��
			
	Replace_Old,		//�滻_��
	Replace_New,		//�滻_��
	Sleep_Time,			//����ʱ��
	Inventory_Time		//
	
}
Setting_Type;	

//������С
typedef enum
{	
	BEEP_VOL_SMALL = 10,	
	BEEP_VOL_BIG = 50,	
}
BeepVol_type;	

//����
typedef enum
{	
	BEEP_TONE_LONG = 1000,	
	BEEP_TONE_CENTRE = 2000,	
	BEEP_TONE_SHORT = 3000,
}
Beep_Change;	

//������
typedef enum
{	
	Normal_Beep,		//����
	Setting_Beep		//����
}	
Beep_Type;	

//������
typedef enum
{	
	BaudRate_1200=1200,	
	BaudRate_2400=2400,	
	BaudRate_4800=4800,	
	BaudRate_9600=9600,
	BaudRate_19200=19200,	
	BaudRate_38400=38400,
	BaudRate_57600=57600,	
	BaudRate_115200=115200
} 
BaudRate_Type;	

//����
typedef enum
{	
	America,				//����Ӣ��
	France,					//��������
	German,					//�¹�����
	France_CAN,				//���ô���
	Dutch,					//������
	Spanish,				//��������
	Portuguese_BR,			//�������������
	Italian,				//�������
	Portuguese,				//��������
	Turkish_F,				//������F
	Turkish_Q,				//������Q
	Japanese,				//�ձ�
	Russian					//����˹
} 
Language_Type;	

//��׼_��������
typedef enum
{	
	NONE = 0,				//����׼
	NORMAL, 				//��ͨ
	ALWAYS					//����
}	
Sight_LightSet;	

//��ĸ״̬
typedef enum
{	
	Normal,					//����
	Capital,				//��д
	Lowercase,				//Сд
	Inversion				//��ת
}	
LetterStatus_Type;	

//��׺������
typedef enum
{
	SUF_TAIL_CR = 0,	//�س�
	SUF_TAIL_CRLF,		//�س�����
	SUF_TAIL_TAB,		//TAB
	SUF_TAIL_NULL		//��
}
Suf_Tail;	

//����ɨ����ʱ��
typedef enum
{
	INTERVAL_0MS=0,
	INTERVAL_500MS=500,
	INTERVAL_1000MS=1000,
	INTERVAL_1500MS=1500,
	INTERVAL_2000MS=2000
}
Interval_type;

//����λ
typedef enum
{
	DATABIT_7BIT=7,
	DATABIT_8BIT=8
}
DataBit_type;	

//ֹͣλ
typedef enum
{
	STOPBIT_1BIT=1,
	STOPBIT_2BIT=2
}
StopBit_type;	

//��żУ��ģʽ
typedef enum
{
	PARITYBIT_NONE = 0,		//��У��
	PARITYBIT_ODD,			//��У��
	PARITYBIT_EVEN			//żУ��
}
ParityBit_type;

//ʶ��ģʽ
typedef enum
{
	_MODE_MANUAL = 0,		//�ֶ�ģʽ
	_MODE_INDUCTION,		//��Ӧģʽ
	_MODE_CONTINUAL, 		//����ģʽ
	_MODE_COMMAND			//�����ģʽ
}
ScanMode_type;

//�ϴ�ģʽ
typedef enum
{
	Normal_Mode,			//��ͨģʽ
	Inventory_Mode			//�̵�ģʽ
}
UploadMode_type;

//�ӿ�
typedef enum
{
	Usart_Mode,				//����
	USB_Mode,				//USB
	USB_VCP_Mode,			//USB���⴮��
	Wireless_Mode			//����
}
TxMode_Type;	

typedef struct 
{
	uint8_t uState;					//����״̬
	uint8_t uStartStopSymbol;		//�Ƿ�����ֹ��
	uint8_t uMiniLen;				//��̳���
	uint8_t uMaxLen;				//�����
}
SetCode_type;

typedef struct 
{
	//������
	Status_type	BeepSound;							//���������� 1:�� 0����
	Status_type StartSound;							//��������	1:�� 0����
	BeepVol_type BeepVol;							//���������� 
	Beep_Change BeepLenght;							//����������	
	
	ScanMode_type ScanMode;							//ɨ��ģʽ
	Suf_Tail ScanTail;								//ɨ���׺������������׺��
	Interval_type ScanInterval;						//����ɨ���� //Interval = ScanInterval*100;
	UploadMode_type UploadMode;						//�ϴ�ģʽ
	uint8_t InventoryData;							//�̵�����
	TxMode_Type Tx_Mode;							//�ӿ�ģʽ
	
	BaudRate_Type BaudRate;							//���ڲ�����
	DataBit_type DataBit;							//��������λ	Ĭ��8λ
	StopBit_type StopBit;							//����ֹͣλ	Ĭ��1λ
	ParityBit_type ParityBit;						//����У��λ	Ĭ����
	
	
	Sight_LightSet Collimation;						//��׼����	Ĭ����ͨ
	Sight_LightSet Lighting;						//����������	Ĭ����ͨ
	
	
	Language_Type KeyBoardType;						//��������
	
	uint8_t UserIDVar;								//�û�ID��ֵ������Ϊ����Χ00~99.
	Status_type CodeType;							//��ʾ�������Ϳ�����ر�
	Status_type UserID;								//UserID����
	Status_type IDPre;								//ǰID����
	Status_type IDSuf;								//��ID����
	Status_type STX;								//STX����
	Status_type ETX;								//ETX����
	Status_type HidePre;							//ǰ���ؿ���
	Status_type HideSuf;							//�����ؿ���
	Status_type KeepPre;							//ǰ��������
	Status_type KeepSuf;							//��������
	Status_type AddPre;								//ǰ׺��ӿ���
	Status_type AddSuf;								//��׺��ӿ���
	Status_type String_Inversion;					//�ַ����򿪹�
	LetterStatus_Type Letter_Status;				//��ĸ��Сд״̬

	Status_type Insert;								//���뿪��
	Status_type Delete;								//ɾ������
	Status_type Replace;							//�滻����
	
	uint8_t LimitLowerLen;							//��������
	uint16_t LimitUpperLen;							//��������
	uint8_t AddPreLen;								//���ǰ׺����
	uint8_t AddSufLen;								//��Ӻ�׺����
	uint8_t HidePreLen;								//ǰ���س���
	uint8_t HideSufLen;								//�����س���
	uint16_t KeepPreLen;							//ǰ��������
	uint16_t KeepSufLen;							//��������
	uint8_t DeleteLen;								//ɾ������
	uint8_t ReplaceOldLen;							//�滻ǰ����
	uint8_t ReplaceNewLen;							//�滻�󳤶�
	
	uint8_t AddPreCodeBuf[ADD_CODE_NUM];             //ǰ׺Ҫ��ӵ��ַ����ݣ����15���ַ���
	uint8_t AddSufCodeBuf[ADD_CODE_NUM];             //��׺Ҫ��ӵ��ַ����ݣ����15���ַ���
	uint8_t InsertAddr[INSERT_GROUP];                //�ӵڼ�λ����
	uint8_t InsertLen[INSERT_GROUP];                 //�����뼸λ
	uint8_t InsertCodeBuf[INSERT_GROUP][INSERT_NUM]; //Ҫ������ַ����ݣ���������飬ÿ����ຬ10���ַ���
	uint8_t DeleteCodeBuf[DELETE_NUM];               //Ҫɾ�����ַ����ݣ����10���ַ���
	uint8_t ReplaceOldCodeBuf[REPLACE_NUM];          //���ǰ���ַ����ݣ����10���ַ���
	uint8_t ReplaceNewCodeBuf[REPLACE_NUM];	         //�������ַ����ݣ����10���ַ���
	
	
	
	SetCode_type EAN13;
	SetCode_type EAN8;
	SetCode_type UPCA;
	SetCode_type UPCE0;
	SetCode_type UPCE1;
	
	SetCode_type Code128;
	SetCode_type Code39;
	SetCode_type Code93;
	SetCode_type CodeBar;
	SetCode_type QR;
	SetCode_type Interleaved2of5;
	SetCode_type Industrial25;
	SetCode_type Matrix2of5;
	SetCode_type MSI;
	SetCode_type Code11;	

	
}Dev_Config;


typedef struct
{
	uint8_t PreNum;									//ǰ׺����
	uint8_t SufNum; 								//��׺����
	uint8_t PriorNum;								//���������
	uint16_t uNum;     								//������
	uint16_t uStart;            					//������ʼλ��
	uint8_t PreCodeBuf[PRE_CODE_NUM];               //���á�2λԤ��λ + 1λSTX + 3λ�û�ID + 15λǰ׺��,��106λ
	uint8_t PriorCodeBuf[PRIOR_CODE_NUM];           //����ɨ������������ַ���ǰ99λ�Լ��м�Ĳ����ַ�,��180λ
	uint8_t SufCodeBuf[SUF_CODE_NUM];	            //���á�15λ��׺ + 1λETX + 2λ������ + 3λԤ��λ��,��106λ


}MyString_Typedef;


extern Dev_Config ScanSet;


//�������ó�ʼ����δ��ʼ�����ͽ��г������ã����ù��Ͷ�ȡ
extern void ActionConfigSetInit(void);
//�������ó�ʼ�������ڻָ�������������
extern void ActionConfigDevInit(void);
//�����봦��
extern uint8_t ActionConfigCodeProcess(uint8_t *BuffCode, uint16_t len);




#endif

