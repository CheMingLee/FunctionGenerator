#include "SetupInterrupt.c"
#include "SettingDefine.h"

// input
float g_fJF8_PWM_Frequency[16]; // Hz
float g_fJF8_PWM_Duty[16]; // 0-100
float g_fJF8_PWM_Delay[16]; // s
float g_fJF7_PWM_Frequency[16]; // Hz
float g_fJF7_PWM_Duty[16]; // 0-100
float g_fJF7_PWM_Delay[16]; // s
int g_iP2_FunctionType[2];
float g_fP2_Anal_Freq[2]; // Hz
float g_fP2_Anal_Amp[2]; // V
float g_fP2_Anal_Ratio[2]; // 0-1
float g_fP2_Anal_Delay[2]; // s

// output
u32 g_outputdata_JF8;
u32 g_outputdata_JF7;
u32 g_outputdata_P2[2];

int main()
{
	g_outputdata_JF8 = 0;
	g_outputdata_JF7 = 0;
	g_outputdata_P2[0] = 0;
	g_outputdata_P2[1] = 0;

	SetupInterrupt();
	
	while (1)
	{
		Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
		Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
		Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
		Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
	}

	return 0;
}

// XTime start, end;
// double timePWM;

// XTime_GetTime(&start);
// GetPWM();
// XTime_GetTime(&end);
// timePWM = (double)(end - start) / COUNTS_PER_SECOND * UNIT_TIME;
