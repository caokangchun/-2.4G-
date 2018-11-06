#ifndef	__MG1S02_H__
#define	__MG1S02_H__


#include "Common.h"
#include "BSP.h"


//ʶ��ģʽ
typedef enum
{
	MODE_MANUAL = 0,		//�ֶ�ģʽ
	MODE_INDUCTION,			//��Ӧģʽ
	MODE_CONTINUAL, 		//����ģʽ
	MODE_COMMAND			//�����ģʽ
}
Scan_Mode;

//��׼
typedef enum
{
	COLLIMATE_NONE = 0,		//����׼
	COLLIMATE_NORMAL, 		//��ͨ
	COLLIMATE_ALWAYS		//����
}
Collimate_Mode;

//����
typedef enum
{
	LIGHT_NONE = 0,		//������
	LIGHT_NORMAL, 		//��ͨ
	LIGHT_ALWAYS		//����
}
Light_Mode;

//��Դ������ģʽ��Ĭ�ϵ�ƽ
typedef enum
{
	IDLE_HIGH_LEVEL = 0,	//���������иߵ�ƽ��æµ�͵�ƽ
	IDLE_LOW_LEVEL			//���������е͵�ƽ��æµ�ߵ�ƽ
}
Default_Level;

//���ݱ����ʽ
typedef enum
{
	ENCODING_FORMAT_GBK = 0,	
	ENCODING_FORMAT_UNICODE,	//���������Ч
	ENCODING_FORMAT_AUTO,		//����������Ч
	ENCODING_FORMAT_UTF8
}
Encoding_Format;

//�������ģʽ
typedef enum
{
	OUTPUT_MODE_UART = 0,		//�������
	OUTPUT_MODE_USB_PC = 1,		//USB PC����
	OUTPUT_MODE_USB_UART = 3	//USB ���⴮��
}
Output_Mode;

//��żУ��ģʽ
typedef enum
{
	PARITY_NONE = 0,	//��У��
	PARITY_ODD,			//��У��
	PARITY_EVEN			//żУ��
}
Parity_Mode;

//���뷶Χ
typedef enum
{
	DECODE_SCOPE_FULL = 0,		//ȫ������
	DECODE_SCOPE_CENTRE_ONLY	//����������
}
Decode_Scope;

//�����ܿ���
typedef enum
{
	BARCODE_CONTROL_DISABLE = 0,		//��ֹʶ����������
	BARCODE_CONTROL_ENABLE_ALL = 1,		//����ʶ����������
	BARCODE_CONTROL_ENABLE_DEFAULT = 2	//��Ĭ�Ͽ�ʶ������
}
Barcode_Control;

//������
typedef enum
{
	TAIL_CR = 0,	//�س�
	TAIL_CRLF,		//�س�����
	TAIL_TAB,		//TAB
	TAIL_NULL		//��
}
Tail_Type;

//��������
typedef enum
{
	KEYBOARD_AMERICA = 0,	//����
	KEYBOARD_CZECH,			//�ݿ�
	KEYBOARD_FRANCE,		//����
	KEYBOARD_GERMANY,		//�¹�
	KEYBOARD_HUNGARY,		//������
	KEYBOARD_ITALY,			//�����
	KEYBOARD_JAPAN,			//�ձ�
	KEYBOARD_SPAIN,			//������
	KEYBOARD_TURKEY_Q,		//������Q
	KEYBOARD_TURKEY_F		//������F
}
Keyboard_Type;

//CodeID�б�
typedef enum
{
	CODEID_EAN_13 = 0x91,
	CODEID_EAN_8,
	CODEID_UPC_A,
	CODEID_UPC_E0,
	CODEID_UPC_E1,
	CODEID_CODE_128,
	CODEID_CODE_39,
	CODEID_CODE_93,
	CODEID_CODEBAR,
	CODEID_INTERLEAVED_2_OF_5,
	CODEID_INDUSTRIAL_2_OF_5,
	CODEID_MATTRIX_2_OF_5,
	CODEID_CODE_11,
	CODEID_MIS_PLESSEY,
	CODEID_RSS_14,			
	CODEID_RSS_LIMITED,	
	CODEID_RSS_EXPANDED,
	CODEID_QR_CODE,
	CODEID_DATA_MATTRIX,
	CODEID_PDF_417
}
CodeID_Addr;

//�ַ���ȡ
typedef enum
{
	INTERCEPT_ALL = 0,				//��������Data�ַ�
	INTERCEPT_FIRST,				//������ǰM��Data�ַ�
	INTERCEPT_LAST,					//�����ͺ�N��Data�ַ�
	INTERCEPT_WITHOUT_FIRST_LAST	//������ǰM+��N��Data�ַ�
}
Character_Interception;


extern void MG1S02Init(void);		//��ʼ��
extern void MG1S02Scan(void);		//ɨ��
extern void MG1S02Open(void);
extern void MG1S02Close(void);
extern bool MG1S02AllowRead(void);	//���������
extern void MG1S02ReadData(uint8_t* Data, uint16_t* DataLen);	//��ȡһ��ɨ������
extern bool MG1S02SaveFlag(void);		//�����־λ����ģ���flash 
extern bool MG1S02FlagFactorySetting(void);  	//��־λ�ָ�����������
extern bool MG1S02SetBaud(uint32_t Baud);		//���ò�����
extern bool MG1S02GetBaud(uint32_t* Baud);		//��ò�����
extern bool MG1S02SendCommand(uint16_t FlagAddr, uint8_t FlagLen, uint8_t BitsLen, uint8_t BitsOffset, uint16_t Data ,bool NeedToRead);		//���ʹ�������
extern bool MG1S02GetSetting(uint16_t FlagAddr, uint8_t FlagLen, uint8_t BitsLen, uint8_t BitsOffset, uint16_t* Value);			//��ȡָ����־λֵ
extern bool MG1S02SetCharacterValue(uint32_t Addr, const char *Str, uint16_t Len);		//�����ַ�ֵ
extern bool MG1S02GetCharacterValue(uint32_t Addr, char *Str, uint16_t Len);		//��ȡ�ַ�ֵ
extern bool MG1S02GetVersion(uint32_t Addr, char* Version);		//��ȡ�汾��
extern bool MG1S02GetSoftwareDate(char* Date);		//��ȡ�������
extern bool MG1S02GetProductModel(char* Model);		//��ȡ��Ʒ�ͺ�

#endif

