#include "SetupInterrupt.c"
#include "SettingDefine.h"

// input
extern float g_dJF8_PWM_Frequency[16]; // Hz
extern float g_dJF8_PWM_Duty[16]; // 0-100
extern float g_dJF8_PWM_Delay[16]; // s

// output
u32 g_outputdata_JF8;

int main()
{
	g_outputdata_JF8 = 0;

	SetupInterrupt();
	
	while (1)
	{
		Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	}

	return 0;
}
