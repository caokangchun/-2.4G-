#ifndef __CONFIGADDR_H__
#define __CONFIGADDR_H__	1



#define MCU_FLASH_SIZE	(128*1024)

#define EEPROM_SIZE		(1*1024)


/*����#define���ڶ���������eeprom�е�ƫ�Ƶ�ַ*/



#define CONFIG_NoConfig										(0)

#define CONFIG_2_4MAC										(CONFIG_NoConfig+2)		//+1������

#define CONFIG_ScanMode										(CONFIG_2_4MAC+1)		//+2������

#define CONFIG_Keyboard										(CONFIG_ScanMode+1)

#define CONFIG_BeepEn										(CONFIG_Keyboard+1)

#define CONFIG_Sleep										(CONFIG_BeepEn+1)

#define CONFIG_SuccessCount									(CONFIG_Sleep+2)






#endif
