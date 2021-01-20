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
	// XTime XT_start, XT_end;
	// float fRunTime = 0.0;
	// float fT_PWM_on, fT_PWM_off, fT_PWM_total;
	// unsigned long ulRunTime = 0;
	// float fCount = 0.0;
	// u32 u32OutStatus;
	
	g_outputdata_JF8 = 0;
	g_outputdata_JF7 = 0;
	g_outputdata_P2[0] = 0;
	g_outputdata_P2[1] = 0;

	// g_fJF8_PWM_Frequency[0] = 0.0;

	SetupInterrupt();
	
	// XTime_GetTime(&XT_start);

	while (1)
	{
		Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
		Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
		Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
		Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);

		// if (g_fJF8_PWM_Frequency[0] > 0.0)
		// {
		// 	fT_PWM_total = 1.0 / g_fJF8_PWM_Frequency[0] * UNIT_TIME;
		// 	fT_PWM_on = g_fJF8_PWM_Duty[0] / g_fJF8_PWM_Frequency[0] * 0.01 * UNIT_TIME;

		// 	XTime_GetTime(&XT_end);
		// 	fRunTime = (float)(XT_end - XT_start) / COUNTS_PER_SECOND * UNIT_TIME;
		// 	ulRunTime = round(fRunTime);

		// 	fCount = fmod((float)ulRunTime, fT_PWM_total);

		// 	if (fCount <= 0.1)
		// 	{
		// 		u32OutStatus = Xil_In32(IO_ADDR_OUTPUT_STATUS);
		// 		g_outputdata_JF8 = GetPWM_CH_OnOff(true, u32OutStatus, 0);
		// 	}
		// 	else if (fCount >= fT_PWM_on)
		// 	{
		// 		u32OutStatus = Xil_In32(IO_ADDR_OUTPUT_STATUS);
		// 		g_outputdata_JF8 = GetPWM_CH_OnOff(false, u32OutStatus, 0);
		// 	}
		// }
	}

	return 0;
}
