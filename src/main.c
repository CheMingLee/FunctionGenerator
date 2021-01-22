#include "SetupInterrupt.c"
#include "SettingDefine.h"

// Time
XTime g_XT_Start, g_XT_End;
XTime g_XT_JF8_Delay_Start;
// XTime g_LED_T1, g_LED_T2;

// LED
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

// PWM
double g_dJF8_PWM_Ttotal[16];		// s
double g_dJF8_PWM_Ton[16];		// s

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

		g_dJF8_PWM_Ttotal[i] = 0.0;
		g_dJF8_PWM_Ton[i] = 0.0;

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
	// XTime_GetTime(&g_LED_T1);
	XTime_GetTime(&g_XT_Start);
}

// void GetYLEDtwinkle()
// {
// 	XTime_GetTime(&g_LED_T2);

// 	if ((g_LED_T2 - g_LED_T1) >= COUNTS_PER_SECOND)
// 	{
// 		g_LED_T1 = g_LED_T2;

// 		g_setYLED_output = 2 - g_setYLED_output;
// 		Xil_Out32(IO_ADDR_LEDOUT, g_setYLED_output + g_setRLED_output);
// 	}
// }

void GetPWM_JF8(int iCH)
{
	long long lPeriodCnt, lTonCnt, lDelayCnt;

	XTime_GetTime(&g_XT_End);

	lPeriodCnt = g_dJF8_PWM_Ttotal[iCH] * COUNTS_PER_SECOND;
	lTonCnt = g_dJF8_PWM_Ton[iCH] * COUNTS_PER_SECOND;
	lDelayCnt = g_fJF8_PWM_Delay[iCH] * COUNTS_PER_SECOND;

	if ((g_XT_End - g_XT_JF8_Delay_Start) >= lDelayCnt)
	{
		if (g_fJF8_PWM_Frequency[iCH] > 0.0 && g_fJF8_PWM_Duty[iCH] > 0.0)
		{
			if (((g_XT_End - g_XT_Start) % lPeriodCnt) >= lTonCnt)
			{
				g_uSetJF8Out[iCH] = 0;
			}
			else
			{
				g_uSetJF8Out[iCH] = g_uOutSetting[iCH];
			}
		}
		else
		{
			g_uSetJF8Out[iCH] = 0;
		}
	}
	else
	{
		g_uSetJF8Out[iCH] = 0;
	}
	
	if((g_XT_End - g_XT_Start) > (lPeriodCnt * 1000))
	{
		g_XT_Start += lPeriodCnt * 1000;
	}
}

void GetAllOutput()
{	
	int i;
	g_outputdata_JF8 = 0;

	for (i = 0; i < 16; i++)
	{
		g_outputdata_JF8 += g_uSetJF8Out[i];
	}

	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
	Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
	Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
}

int main()
{
	GetParamsInialization();

	GetTimeStart();

	g_fJF8_PWM_Frequency[0] = 33000.0;
	g_fJF8_PWM_Duty[0] = 25.0;
	g_fJF8_PWM_Delay[0] = 10.0;
	g_dJF8_PWM_Ttotal[0] = (double)(1.0 / g_fJF8_PWM_Frequency[0]);
	g_dJF8_PWM_Ton[0] = (double)(g_fJF8_PWM_Duty[0] * 0.01) * g_dJF8_PWM_Ttotal[0];
	
	g_fJF8_PWM_Frequency[2] = 10000.0;
	g_fJF8_PWM_Duty[2] = 45.0;
	g_dJF8_PWM_Ttotal[2] = (double)(1.0 / g_fJF8_PWM_Frequency[2]);
	g_dJF8_PWM_Ton[2] = (double)(g_fJF8_PWM_Duty[2] * 0.01) * g_dJF8_PWM_Ttotal[2];

	g_fJF8_PWM_Frequency[4] = 20000.0;
	g_fJF8_PWM_Duty[4] = 66.0;
	g_dJF8_PWM_Ttotal[4] = (double)(1.0 / g_fJF8_PWM_Frequency[4]);
	g_dJF8_PWM_Ton[4] = (double)(g_fJF8_PWM_Duty[4] * 0.01) * g_dJF8_PWM_Ttotal[4];

	// SetupInterrupt();

	XTime_GetTime(&g_XT_JF8_Delay_Start);

	int i;

	while (1)
	{
		for (i = 0; i < 16; i++)
		{
			GetPWM_JF8(i);
		}

		GetAllOutput();
	}

	return 0;
}
