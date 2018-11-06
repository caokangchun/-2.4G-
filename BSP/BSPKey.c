#include "MM32F103.h"
#include "Common.h"
#include "FIFO.h"
#include "BSP.h"


#define KEY_DOWN_TIMES_SHORT_MATCH      3
#define KEY_DOWN_TIMES_LONG_MATCH       800

/*���Ŷ���*/
#define KEY_PIN							GPIO_Pin_1
#define KEY_GPIO_PORT					GPIOB
#define KEY_GPIO_CLK					RCC_APB2Periph_GPIOB

#define KeyVal()						GPIO_ReadInputDataBit(KEY_GPIO_PORT, KEY_PIN)


typedef struct
{
    uint8_t State;
    uint32_t Times;
}
Key_Attrib;


static FIFO FIFOKeySignal;
static bool RunFlag;
static Key_Mode KeyMode;


static void WriteData(void* DataTo, const void* DataFrom)
{
    *((Key_Signal*)DataTo) = *((Key_Signal*)DataFrom);
}

//*******************************************************************
//���ã�������ʼ��
//��������
//���أ�TRUE-�ɹ���ʼ��
//      FALSE-ʧ�ܳ�ʼ��
//*******************************************************************
bool BSPKeyInit(void)
{
	GPIO_InitTypeDef GPIOInitStructure;
	uint32_t time = 0;
	uint16_t release;

	RCC_APB2PeriphClockCmd(KEY_GPIO_CLK, ENABLE);
	GPIOInitStructure.GPIO_Pin  =  KEY_PIN;
	GPIOInitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(KEY_GPIO_PORT, &GPIOInitStructure);
	
    FIFOKeySignal = FIFOCreate(1, sizeof(Key_Signal));
    FIFOCallbackWrite(FIFOKeySignal, WriteData);

	RunFlag = FALSE;

	if(KeyVal() == SET)
	{
		return(TRUE);
	}
	
	while(1)
	{
		if(++time > 60000000)
		{
			return(FALSE);
		}
		if(KeyVal() == SET)
		{
			release = 1000;
			while(--release);
			if(KeyVal() == SET)
			{
				return(TRUE);
			}
		}
	}
}

//����ģʽ����
void BSPKeyModeSet(Key_Mode Mode)
{
	KeyMode = Mode;
}

//*******************************************************************
//���ã�����ʹ��
//��������
//���أ���
//*******************************************************************
void BSPKeyEnable(void)
{
    RunFlag = TRUE;
}

//*******************************************************************
//���ã���������
//��������
//���أ���
//*******************************************************************
void BSPKeyDisable(void)
{
    RunFlag = FALSE;
}

//*******************************************************************
//���ã���ѯ�����Ƿ������
//��������
//���أ�TRUE-�а������Զ���
//      FALSE-�ް������Զ���
//*******************************************************************
bool BSPKeyAllowRead(void)
{
    if(FIFOCount(FIFOKeySignal) == 0)
	{
		return(FALSE);
	}
	return(TRUE);
}

//*******************************************************************
//���ã���ȡ������ֵ
//��������
//���أ�������ֵ
//*******************************************************************
Key_Signal BSPKeyRead(void)
{
    Key_Signal keySignal;
	
	keySignal.Type = BSP_KEY_TYPE_NONE;

	FIFOPop(FIFOKeySignal, &keySignal);
	return(keySignal);
}

static Key_Type KeyScanRelease(uint8_t KeyLevel, Key_Attrib* KeyAttr)
{
    static Key_Type keyType;

    switch(KeyAttr->State)
    {
        case 0:
        {
            if(KeyLevel == 0)
            {
				keyType = BSP_KEY_TYPE_NONE;
                KeyAttr->Times = 0;
                KeyAttr->State = 1;
            }
            break;
        }
        case 1:
        {
            if(KeyLevel == 0)
            {
                ++KeyAttr->Times;
                if(KeyAttr->Times == KEY_DOWN_TIMES_SHORT_MATCH)
                {
                    keyType = BSP_KEY_TYPE_SHORT;
                    KeyAttr->State = 2;
                }
            }
            else
            {
                KeyAttr->State = 0;
            }
            break;
        }
        case 2:
        {            
            if(KeyAttr->Times > KEY_DOWN_TIMES_LONG_MATCH)
            {
				return(BSP_KEY_TYPE_LONG);
            }
            else
            {
                ++KeyAttr->Times;
            }
            if(KeyLevel == 1)
            {
                KeyAttr->State = 0;
                return(keyType);
            }
            break;
        }
    }
    return(BSP_KEY_TYPE_NONE);
}

static Key_Type KeyScanPress(uint8_t KeyLevel)
{
	static uint32_t keytime = 0;
	
	if(KeyLevel == 0)
	{
		keytime++;
		if(keytime > KEY_DOWN_TIMES_LONG_MATCH)
		{
			return(BSP_KEY_TYPE_NONE);
		}
		else if(keytime > KEY_DOWN_TIMES_SHORT_MATCH)
		{
			return(BSP_KEY_TYPE_SHORT);
		}
	}
	else
	{	
		if(keytime > KEY_DOWN_TIMES_LONG_MATCH)
		{
			return(BSP_KEY_TYPE_LONG);
		}
		
		keytime = 0;
		return(BSP_KEY_TYPE_NONE);
	}
	
	return(BSP_KEY_TYPE_NONE);
}

//*******************************************************************
//���ã���������������
//��������
//���أ���
//*******************************************************************
void BSPKeyRoutine(void)
{
	static Key_Attrib keyAttr[] = 
	{
	    {0, 0},
	};
	static uint8_t value;
	Key_Signal keySignal;

	if(RunFlag == FALSE)
	{
	    return;
	}

	value = KeyVal();
	
	if(KeyMode == BSP_KEY_MODE_RELEASE)
	{
		keySignal.Type = KeyScanRelease(value, &keyAttr[0]);
	}
	else
	{
		keySignal.Type = KeyScanPress(value);
	}
	
	if(keySignal.Type != BSP_KEY_TYPE_NONE)
	{
	    keySignal.Value = 1;
	    FIFOPush(FIFOKeySignal, &keySignal);
	}
}

