#include <stdlib.h>
#include "Common.h"
#include "FIFO.h"
#include "BSP.h"
#include "3800B.h"


//ɨ���ʼ��
void BSPCodeScannerInit(void)
{
	CM3800BInit();
}

//ɨ���
void BSPCodeScannerOpen(void)
{
	CM3800BOpen();
}

//ɨ��ر�
void BSPCodeScannerClose(void)
{
	CM3800BClose();
}

void BSPCodeScannerScanStart(void)
{
	CM3800BScanStart();
}

void BSPCodeScannerScanStop(void)
{
	CM3800BScanStop();
}

bool BSPCodeScannerAllowRead(void)
{
	return(CM3800BAllowRead());
}

uint8_t BSPCodeScannerRead(void)
{
	return(CM3800BRead());
}

void BSPCodeScannerRoutine(void)
{
	CM3800BRoutine();
}


