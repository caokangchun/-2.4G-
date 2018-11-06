
#include "Common.h"
#include "BSPEEPROM.h"


//#define FLASH_PAGE_SIZE             ((uint16_t)0x400)	//1024

#define MM32_FLASH_BASE					0x8000000U
#define FLASH_PAGE_SIZE             	1024U	
#define MM32_PAGE_NUM		        	128U


static uint32_t StartAddress;             	//Stortʹ�õ�FLASH��ʼ��ַ
static uint16_t Size;                     	//EEPROM��С
static bool Switch = FALSE;             	//ʹ�ÿ��أ�����ʼ����ȷ����Ч


//������д��  
static bool EEPROM_Write_NoCheck(uint32_t a,uint16_t *d,uint16_t l);

//*******************************************************************
//���ã�EEPROM��ʼ��
//������FlashSize-оƬFLASH��С����λ�ֽ�
//      EEPROMSize-����EEPROM��С����λ�ֽڣ�����Ϊ2�ı���
//���أ�TRUE-�ɹ�����EEPROM
//      FALSE-ʧ�ܽ���EEPROM
//*******************************************************************
bool BSPEEPROMInit(uint32_t FlashSize, uint16_t EEPROMSize)
{
	
    //ȷ��������СΪż��
    FlashSize &= 0xFFFFFFFE;
    EEPROMSize &= 0xFFFE;

    if(FlashSize < EEPROMSize)
    {
        Switch = FALSE;
        return(FALSE);
    }

    Size = EEPROMSize;

    //����EEPROMʹ��FLASH����ʼҳ���ַ
    if(EEPROMSize % FLASH_PAGE_SIZE)
    {
        StartAddress = 0x8000000 + (FlashSize - FLASH_PAGE_SIZE * (EEPROMSize / FLASH_PAGE_SIZE + 1));
    }
    else
    {
        StartAddress = 0x8000000 + (FlashSize - EEPROMSize);
    }
	
    Switch = TRUE;
    return(TRUE);
}

//���Addressָ���ֵ
//Address:ָ���ĵ�ַ
static bool FLASHErase(uint32_t Address)
{
    uint32_t pageAddress;
    uint32_t tmp;
    uint16_t Data[FLASH_PAGE_SIZE / 2];
    uint8_t Page;
    uint16_t i = 0;
    FLASH_Status status;

    //��ȡָ����ַ����ҳ��FLASH�׵�ַ������������ҳ
    Page = (Address - 0x8000000) / FLASH_PAGE_SIZE;
    pageAddress = 0x8000000 + Page * FLASH_PAGE_SIZE;
    tmp = pageAddress;
    while(i < (FLASH_PAGE_SIZE / 2))
    {
        Data[i] = *((uint32_t*)tmp);
        tmp += 2;
        ++i;
    }

    //���ڴ����Ҫ�޸ĵ�Addressָ���ֵ
    Data[(Address - pageAddress) / 2] = 0xFFFF;

    //ˢ������ҳ
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	status = FLASH_ErasePage(pageAddress); 
    if(status != FLASH_COMPLETE)
    {
		FLASH_Lock();
        return(FALSE);
    }

    //���ݻ�д������Address��ַ�Ŀռ�
    i = 0;
    tmp = pageAddress;
    while(i < (FLASH_PAGE_SIZE / 2))
    {
        if(Data[i] != 0xFFFF)
        {
            FLASH_ProgramHalfWord(tmp, Data[i]);
        }
        tmp += 2;
        ++i;
    }
    FLASH_Lock();
    return(TRUE);
    
}

#if 0
//*******************************************************************
//���ã�EEPROMд����.2���ֽڣ����֣�
//������Address-д���EEPROM��ַ.��:0x300
//      Data-д�������
//���أ�TRUE-�ɹ�д��
//      FALSE-ʧ��д��
//*******************************************************************
bool BSPEEPROMWrite_old(uint16_t Address, uint16_t Data)
{
    FLASH_Status status;
    uint32_t flashAddress;
    uint16_t tmp;

    if(Switch == FALSE)
    {
        return(FALSE);
    }

    if(Address >= Size / 2)
    {
        return(FALSE);
    }

    flashAddress = StartAddress + (Address * 2);

    //�ж������ܷ�д��
    tmp = *((uint32_t*)flashAddress);
    if(tmp != 0xFFFF)
    {
        if(FLASHErase(flashAddress) == TRUE)
        {
            FLASH_Unlock();
            FLASH_ProgramHalfWord(flashAddress, Data);
            FLASH_Lock();
            return(TRUE);
        }
		return(FALSE);
    }
    else
    {
        FLASH_Unlock();
        fmc_flag_clear(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
        status = FLASH_ProgramHalfWord(flashAddress, Data);
        FLASH_Lock();
        if(status == FLASH_COMPLETE)
        {
            return(TRUE);
        }
        return(FALSE);
    }
}


//*******************************************************************
//��EEPROM��ַ��ʼд��ָ�����ȵ�����
//ReadAddr:��ʼ��ַ.������Ե�ַ.
//DataBuf:����ָ��
//DataLen:����(16λ)��.����
//*******************************************************************
bool BSPEEPROMWriteBytes_old(uint16_t WriteAddr, uint16_t *DataBuf, uint16_t DataLen)
{
	uint16_t i;
	uint8_t re=1;
	
	for(i=0; i<DataLen; i++)
	{
		re &= BSPEEPROMWrite(WriteAddr, DataBuf[i]);
		WriteAddr ++;//��ַ����.
		
	}
	if(re)
	{
		return(TRUE);
	}		
	else
	{
		return(FALSE);
	}		
	
}

#endif


//*******************************************************************
//������д��
//WriteAddr:��ʼ��ַ
//DataBuf:����ָ��
//DataLen:����(16λ)��   
//*******************************************************************
static bool EEPROM_Write_NoCheck(uint32_t WriteAddr, uint16_t *DataBuf, uint16_t DataLen)   
{ 			 		
	FLASH_Status status;
	uint16_t i;
	
	
	for(i=0; i<DataLen; i++)
	{
		status = FLASH_ProgramHalfWord(WriteAddr, DataBuf[i]);
	    WriteAddr += 2;//��ַ����2.
	}  
	
	if(status == FLASH_COMPLETE)
	{
		return(TRUE);
	}
	return(FALSE);
} 

//*******************************************************************
//���ã�EEPROMд����.2���ֽڣ����֣�
//������WriteAddr-д���EEPROM��ַ.��:0x300
//      DataBuf-����ָ��
//		DataLen:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
//���أ�TRUE-�ɹ�д��
//      FALSE-ʧ��д��
//*******************************************************************
bool BSPEEPROMWrite(uint32_t WriteAddr, uint16_t *DataBuf, uint16_t DataLen)
{
	uint32_t pagePos;	   					//ҳ��ַ
	uint16_t pageOff;	   					//ҳ��ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t pageRemain; 					//ҳ��ʣ���ַ(16λ�ּ���)	   
 	uint16_t i;    
	bool  wStatus = TRUE;						//д���Ƿ�ɹ�
	
	uint32_t offAddr;   					//ȥ��0X08000000��ĵ�ַ
	uint32_t operateAddress;				//������ַ
	uint32_t flashAddress;					//Flash��ַ,���Ե�ַ.
	uint16_t FlashBuf[FLASH_PAGE_SIZE/2];	//Flash���ݴ�Ż�����
	
	flashAddress = StartAddress + (WriteAddr * 2);
	
	if((flashAddress >= (MM32_FLASH_BASE + FLASH_PAGE_SIZE * MM32_PAGE_NUM)))
	{
		return(FALSE);//�Ƿ���ַ
	}
	
	FLASH_Unlock();									//����
	offAddr= flashAddress - MM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	pagePos = offAddr / FLASH_PAGE_SIZE;			//ҳ��ַ,�ڼ�ҳ  0~64 for GD32F150G8
	pageOff = (offAddr % FLASH_PAGE_SIZE) / 2;		//��ҳ�ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	pageRemain = FLASH_PAGE_SIZE / 2 - pageOff;		//ҳʣ��ռ��С   
	
	if(DataLen <= pageRemain)
	{
		pageRemain = DataLen;						//�����ڸ�ҳ��Χ
	}
	
	while(1) 
	{	
		operateAddress = pagePos * FLASH_PAGE_SIZE + MM32_FLASH_BASE;
		BSPEEPROMReadBytes(operateAddress - StartAddress, FlashBuf,FLASH_PAGE_SIZE/2);//��������ҳ������
		for(i=0; i<pageRemain; i++)//У������
		{
			if(FlashBuf[pageOff+i] != 0XFFFF)break;//��Ҫ����  	  
		}
		
		if(i < pageRemain)//��Ҫ����
		{
			FLASH_ErasePage(operateAddress);//�������ҳ
			for(i=0; i<pageRemain; i++)//����
			{
				FlashBuf[i+pageOff] = DataBuf[i];	  
			}
			wStatus &= EEPROM_Write_NoCheck(operateAddress, FlashBuf, FLASH_PAGE_SIZE/2);//д������ҳ  
		}
		else
		{
			wStatus &= EEPROM_Write_NoCheck(flashAddress, DataBuf, pageRemain);//д�Ѿ������˵�,ֱ��д��ҳʣ������. 
		}		
		
		if(DataLen == pageRemain)
		{
			break;//д�������
		}
		else//д��δ����
		{
			pagePos++;						//ҳ��ַ��1
			pageOff=0;						//ƫ��λ��Ϊ0 	 
		   	DataBuf 	 += pageRemain;  	//ָ��ƫ��
			flashAddress += pageRemain;		//д��ַƫ��	   
		   	DataLen 	 -= pageRemain;		//�ֽ�(16λ)���ݼ�
			if(DataLen > (FLASH_PAGE_SIZE/2))
			{
				pageRemain = FLASH_PAGE_SIZE/2;//��һ��ҳ����д����
			}
			else 
			{
				pageRemain = DataLen;//��һ��ҳ����д����
			}
		}	 
	};	
	FLASH_Lock();//����
	
	return (wStatus);//����д״̬.1�ɹ� 0ʧ��
}






//*******************************************************************
//���ã�EEPROM������.2���ֽڣ����֣�
//������Address-������EEPROM��ַ
//���أ�����������.
//*******************************************************************
uint16_t BSPEEPROMRead(uint16_t Address)
{
    uint16_t tmp;
    uint32_t flashAddress;

	if(Switch == FALSE)
    {
        return(0xFFFF);
    }
	
	if(Address >= Size/2)
    {
        return(0xFFFF);
    }
	
	
    flashAddress = StartAddress + (Address * 2);//��2���ֽڶ�ȡ.	
	
    tmp = *((uint32_t*)flashAddress);
    return(tmp);
}

//*******************************************************************
//��EEPROM��ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ.������Ե�ַ.
//DataBuf:����ָ��
//DataLen:����(16λ)��.����
//*******************************************************************
void BSPEEPROMReadBytes(uint32_t ReadAddr, uint16_t *DataBuf, uint16_t DataLen)
{
	uint16_t i;
	
//	if(ReadAddr > StartAddress)//���ԭ����Ϊ��BSPEEPROMWrite()��ȡ����ʱ��������Ǿ��Ե�ַ.
//	{
//		ReadAddr = ReadAddr - StartAddress;
//	}
	
	for(i=0;i<DataLen;i++)
	{
		DataBuf[i]=BSPEEPROMRead(ReadAddr);//��ȡ2���ֽ�.
//		ReadAddr+=2;//ƫ��2���ֽ�.	
		ReadAddr++;//ƫ��.	
	}
}

//*******************************************************************
//���ã�EEPROM��ʽ��
//��������
//���أ�TRUE-�ɹ�
//      FALSE-ʧ��
//*******************************************************************
bool BSPEEPROMFormat(void)
{
    uint32_t addr = StartAddress;
    uint32_t i = 0;
    FLASH_Status status;

    if(Switch == FALSE)
    {
        return(FALSE);
    }

    while(i < Size)
    {
        FLASH_Unlock();
        status = FLASH_ErasePage(addr);
        if(status != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return(FALSE);
        }
        addr += FLASH_PAGE_SIZE;
        i += FLASH_PAGE_SIZE;
    }
    FLASH_Lock();
    return(TRUE);
}

//*******************************************************************
//���ã�EEPROMɾ�����
//������AddrStart-��ʼ��ַ
//      AddrEnd-������ַ    
//���أ�TRUE-�ɹ�
//      FALSE-ʧ��
//*******************************************************************
bool BSPEEPROMDelete(uint16_t AddrStart, uint16_t AddrEnd)
{
    uint32_t flashFromAddress;
    uint32_t flashToAddress;
    FLASH_Status status;

    if(Switch == FALSE)
    {
        return(FALSE);
    }

    if(AddrStart >= Size / 2 || AddrEnd >= Size / 2)
    {
        return(FALSE);
    }

    if(AddrEnd <= AddrStart)
    {
        return(FALSE);
    }

    flashFromAddress = StartAddress + (AddrStart * 2);
    flashToAddress = StartAddress + (AddrEnd * 2);

    while(flashFromAddress <=  flashToAddress)
    {
        if((flashFromAddress - 0x8000000) % FLASH_PAGE_SIZE)
        {
            if(FLASHErase(flashFromAddress) == FALSE)
            {
                return(FALSE);
            }
            flashFromAddress += 2;
        }
        else
        {
            if(flashToAddress - flashFromAddress >= FLASH_PAGE_SIZE - 2)
            {
                FLASH_Unlock();
                status = FLASH_ErasePage(flashFromAddress);
                FLASH_Lock();
                if(status != FLASH_COMPLETE)
                {
                    return(FALSE);
                }
                flashFromAddress += FLASH_PAGE_SIZE;
            }
            else
            {
                if(FLASHErase(flashFromAddress) == FALSE)
                {
                    return(FALSE);
                }
                flashFromAddress += 2;
            }
        }
    }
    return(TRUE);
}


