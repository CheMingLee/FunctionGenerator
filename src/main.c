#include "SetupInterrupt.c"
#include "SettingDefine.h"

// Time
XTime g_LED_T1, g_LED_T2;
XTime g_JF8CH1_T1, g_JF8CH1_T2;
XTime g_JF8CH2_T1, g_JF8CH2_T2;

int g_iRLEDCount;
u32 g_setRLED_output;
u32 g_setYLED_output;

// input
float g_fJF8_PWM_Frequency[16];		// Hz
float g_fJF8_PWM_Duty[16];			// 0-100
float g_fJF8_PWM_Delay[16];			// s
float g_fJF7_PWM_Frequency[16];		// Hz
float g_fJF7_PWM_Duty[16];			// 0-100
float g_fJF7_PWM_Delay[16];			// s
int g_iP2_FunctionType[2];
float g_fP2_Anal_Freq[2];			// Hz
float g_fP2_Anal_Amp[2];			// V
float g_fP2_Anal_Ratio[2];			// 0-1
float g_fP2_Anal_Delay[2];			// s

// output
u32 g_uOutSetting[16];
u32 g_uSetJF8Out[16];
u32 g_outputdata_JF8;
u32 g_outputdata_JF7;
u32 g_outputdata_P2[2];

void GetParamsInialization()
{
	int i;
	for (i = 0; i < 16; i++)
	{
		if (i < 2)
		{
			g_iP2_FunctionType[i] = 0;
			g_fP2_Anal_Freq[i] = 0.0;
			g_fP2_Anal_Amp[i] = 0.0;
			g_fP2_Anal_Ratio[i] = 0.0;
			g_fP2_Anal_Delay[i] = 0.0;
		}
		
		g_fJF8_PWM_Frequency[i] = 0.0;
		g_fJF8_PWM_Duty[i] = 0.0;
		g_fJF8_PWM_Delay[i] = 0.0;
		g_fJF7_PWM_Frequency[i] = 0.0;
		g_fJF7_PWM_Duty[i] = 0.0;
		g_fJF7_PWM_Delay[i] = 0.0;

		g_uSetJF8Out[i] = 0;
		g_uOutSetting[i] = 1 << i;
	}
	
	g_outputdata_JF8 = 0;
	g_outputdata_JF7 = 0;
	g_outputdata_P2[0] = 0;
	g_outputdata_P2[1] = 0;

	g_iRLEDCount = 0;
	g_setRLED_output = 0;
	g_setYLED_output = 0;
}

void GetTimeStart()
{
	XTime_GetTime(&g_LED_T1);
	XTime_GetTime(&g_JF8CH1_T1);
	XTime_GetTime(&g_JF8CH2_T1);
}

void GetYLEDtwinkle()
{
	XTime_GetTime(&g_LED_T2);

	if ((g_LED_T2 - g_LED_T1) >= COUNTS_PER_SECOND)
	{
		g_LED_T1 = g_LED_T2;

		g_setYLED_output = 2 - g_setYLED_output;
		Xil_Out32(IO_ADDR_LEDOUT, g_setYLED_output + g_setRLED_output);
	}
}

void GetPWM_JF8CH1()
{
	XTime_GetTime(&g_JF8CH1_T2);

	if (g_fJF8_PWM_Frequency[0] > 0.0)
	{
		if ((g_JF8CH1_T2 - g_JF8CH1_T1) >= ((float)COUNTS_PER_SECOND / (g_fJF8_PWM_Frequency[0]*2.0)))
		{
			g_JF8CH1_T1 = g_JF8CH1_T2;

			g_uSetJF8Out[0] = g_uOutSetting[0] - g_uSetJF8Out[0];
		}
	}
}

void GetPWM_JF8CH2()
{
	XTime_GetTime(&g_JF8CH2_T2);

	if (g_fJF8_PWM_Frequency[1] > 0.0)
	{
		if ((g_JF8CH2_T2 - g_JF8CH2_T1) >= ((float)COUNTS_PER_SECOND / (g_fJF8_PWM_Frequency[1]*2.0)))
		{
			g_JF8CH2_T1 = g_JF8CH2_T2;

			g_uSetJF8Out[1] = g_uOutSetting[1] - g_uSetJF8Out[1];
		}
	}
}

void GetAllOutput()
{
	g_outputdata_JF8 = g_uSetJF8Out[0] + g_uSetJF8Out[1];
	
	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
	Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
	Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
}

int main()
{
	GetParamsInialization();

	GetTimeStart();

	g_fJF8_PWM_Frequency[0] = 100.0;
	g_fJF8_PWM_Frequency[1] = 200.0;

	SetupInterrupt();

	while (1)
	{
		GetYLEDtwinkle();

		GetPWM_JF8CH1();
		GetPWM_JF8CH2();

		GetAllOutput();
	}

	return 0;
}
