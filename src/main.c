#include "SettingDefine.h"

/*************************************************************************/

// SetupInterrupt.c
void SetupInterrupt(void);
void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
void TickHandler(void *CallBackRef);

// GenerateFunction.c
void GetAppCmd();
void SetLED();
float* GetPWM_Params();
void SetPWM_JF8();
void SetPWM_JF7();
float* GetAnal_Params();
void SetAnal_Function(int iCH);
void SetAnalog_1();
void SetAnalog_2();
bool CheckFlag();
void SetFlagInZero();
void SetFlagOutOne();

/*************************************************************************/

// interrupt
XTtcPs Timer;
XScuGic Intc; 							//GIC

// Time
XTime g_XT_Start, g_XT_End;
XTime g_XT_JF8_Delay_Start, g_XT_JF7_Delay_Start;
XTime g_XT_P2Ch1_Delay_Start, g_XT_P2Ch2_Delay_Start;
XTime g_XT_Analog_Delay_Start;

// input
float g_fJF8_PWM_Frequency[16];			// Hz
float g_fJF8_PWM_Duty[16];				// 0-100
float g_fJF8_PWM_Delay[16];				// s
float g_fJF7_PWM_Frequency[16];			// Hz
float g_fJF7_PWM_Duty[16];				// 0-100
float g_fJF7_PWM_Delay[16];				// s
int g_iP2_FunctionType[2];
float g_fP2_Anal_Freq[2];				// Hz
float g_fP2_Anal_Amp[2];				// V
float g_fP2_Anal_Ratio[2];				// 0-1
float g_fP2_Anal_Delay[2];				// s

// PWM
double g_dJF8_PWM_Ttotal[16];			// s
double g_dJF8_PWM_Ton[16];				// s
double g_dJF7_PWM_Ttotal[16];			// s
double g_dJF7_PWM_Ton[16];				// s

// Analog
double g_dAnal_Period[2];				// s
double g_dAnal_Omega[2];				// rad/s

// output
u32 g_uOutSetting[16];
u32 g_uSetJF8Out[16];
u32 g_uSetJF7Out[16];
u32 g_outputdata_JF8;
u32 g_outputdata_JF7;
u32 g_outputdata_P2[2];

/*************************************************************************/

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

			g_dAnal_Period[i] = 0.0;
			g_dAnal_Omega[i] = 0.0;

			g_outputdata_P2[i] = 0;
		}
		
		g_fJF8_PWM_Frequency[i] = 0.0;
		g_fJF8_PWM_Duty[i] = 0.0;
		g_fJF8_PWM_Delay[i] = 0.0;
		g_fJF7_PWM_Frequency[i] = 0.0;
		g_fJF7_PWM_Duty[i] = 0.0;
		g_fJF7_PWM_Delay[i] = 0.0;

		g_dJF8_PWM_Ttotal[i] = 0.0;
		g_dJF8_PWM_Ton[i] = 0.0;
		g_dJF7_PWM_Ttotal[i] = 0.0;
		g_dJF7_PWM_Ton[i] = 0.0;

		g_uSetJF8Out[i] = 0;
		g_uSetJF7Out[i] = 0;
		g_uOutSetting[i] = 1 << i;
	}

	XTime_GetTime(&g_XT_Start);
}

/*************************************************************************/

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

void GetPWM_JF7(int iCH)
{
	long long lPeriodCnt, lTonCnt, lDelayCnt;

	XTime_GetTime(&g_XT_End);

	lPeriodCnt = g_dJF7_PWM_Ttotal[iCH] * COUNTS_PER_SECOND;
	lTonCnt = g_dJF7_PWM_Ton[iCH] * COUNTS_PER_SECOND;
	lDelayCnt = g_fJF7_PWM_Delay[iCH] * COUNTS_PER_SECOND;

	if ((g_XT_End - g_XT_JF7_Delay_Start) >= lDelayCnt)
	{
		if (g_fJF7_PWM_Frequency[iCH] > 0.0 && g_fJF7_PWM_Duty[iCH] > 0.0)
		{
			if (((g_XT_End - g_XT_Start) % lPeriodCnt) >= lTonCnt)
			{
				g_uSetJF7Out[iCH] = 0;
			}
			else
			{
				g_uSetJF7Out[iCH] = g_uOutSetting[iCH];
			}
		}
		else
		{
			g_uSetJF7Out[iCH] = 0;
		}
	}
	else
	{
		g_uSetJF7Out[iCH] = 0;
	}
	
	if((g_XT_End - g_XT_Start) > (lPeriodCnt * 1000))
	{
		g_XT_Start += lPeriodCnt * 1000;
	}
}

/*************************************************************************/

void GetAnal_Sine(int iCH)
{
	long long lPeriodCnt, lDelayCnt;
	double dTimeSeconds;
	u32 uOutData;

	if (iCH == 0)
	{
		g_XT_Analog_Delay_Start = g_XT_P2Ch1_Delay_Start;
	}
	else if (iCH == 1)
	{
		g_XT_Analog_Delay_Start = g_XT_P2Ch2_Delay_Start;
	}
	
	if (iCH == 0 || iCH == 1)
	{
		XTime_GetTime(&g_XT_End);

		lPeriodCnt = g_dAnal_Period[iCH] * COUNTS_PER_SECOND;
		lDelayCnt = g_fP2_Anal_Delay[iCH] * COUNTS_PER_SECOND;

		if ((g_XT_End - g_XT_Analog_Delay_Start) >= lDelayCnt)
		{
			if (g_fP2_Anal_Freq[iCH] > 0.0 && g_fP2_Anal_Amp[iCH] > 0.0)
			{
				dTimeSeconds = (double)(g_XT_End - g_XT_Start) / (double)COUNTS_PER_SECOND;
				uOutData = round(65535.0 / 11.0 * (g_fP2_Anal_Amp[iCH] / 2.0 * sin(g_dAnal_Omega[iCH] * dTimeSeconds))) + (65535.0 / 11.0 * (g_fP2_Anal_Amp[iCH] / 2.0));
			}
			else
			{
				uOutData = 0;
			}
		}
		else
		{
			uOutData = 0;
		}

		if((g_XT_End - g_XT_Start) > (lPeriodCnt * 1000))
		{
			g_XT_Start += lPeriodCnt * 1000;
		}

		g_outputdata_P2[iCH] = uOutData;	
	}
}

void GetAnal_Sawtooth(int iCH)
{
	long long lPeriodCnt, lRatioPeriodCnt, lDownRatioPeriodCnt, lDelayCnt, lTimeCnt;
	u32 uOutData;

	if (iCH == 0)
	{
		g_XT_Analog_Delay_Start = g_XT_P2Ch1_Delay_Start;
	}
	else if (iCH == 1)
	{
		g_XT_Analog_Delay_Start = g_XT_P2Ch2_Delay_Start;
	}
	
	if (iCH == 0 || iCH == 1)
	{
		XTime_GetTime(&g_XT_End);

		lPeriodCnt = g_dAnal_Period[iCH] * COUNTS_PER_SECOND;
		lRatioPeriodCnt = g_fP2_Anal_Ratio[iCH] * lPeriodCnt;
		lDownRatioPeriodCnt = lPeriodCnt - lRatioPeriodCnt;
		lDelayCnt = g_fP2_Anal_Delay[iCH] * COUNTS_PER_SECOND;

		if ((g_XT_End - g_XT_Analog_Delay_Start) >= lDelayCnt)
		{
			if (g_fP2_Anal_Freq[iCH] > 0.0 && g_fP2_Anal_Amp[iCH] > 0.0)
			{
				lTimeCnt = (g_XT_End - g_XT_Start) % lPeriodCnt;
				
				if (lTimeCnt <= lRatioPeriodCnt)
				{
					uOutData = round(65535.0 / 11.0 * g_fP2_Anal_Amp[iCH] * lTimeCnt / lRatioPeriodCnt);
				}
				else
				{
					uOutData = round(65535.0 / 11.0 * (g_fP2_Anal_Amp[iCH] / lDownRatioPeriodCnt) * (lPeriodCnt - lTimeCnt));
				}
			}
			else
			{
				uOutData = 0;
			}
		}
		else
		{
			uOutData = 0;
		}

		if((g_XT_End - g_XT_Start) > (lPeriodCnt * 1000))
		{
			g_XT_Start += lPeriodCnt * 1000;
		}

		g_outputdata_P2[iCH] = uOutData;	
	}
}

void GetAnal_P2(int iCH)
{
	switch (g_iP2_FunctionType[iCH])
	{
		case CLOSE_ANALOG:
		{
			g_outputdata_P2[iCH] = 0;
			break;
		}

		case SINE_ANALOG:
		{
			GetAnal_Sine(iCH);
			break;
		}
		
		case TRIANGE_ANALOG:
		{
			GetAnal_Sawtooth(iCH);
			break;
		}

		case SAWTOOTH_ANALOG:
		{
			GetAnal_Sawtooth(iCH);
			break;
		}

		default:
		{
			break;
		}
	}
}

/*************************************************************************/

void GetAllOutput()
{	
	int i;
	g_outputdata_JF8 = 0;
	g_outputdata_JF7 = 0;

	for (i = 0; i < 16; i++)
	{
		g_outputdata_JF8 += g_uSetJF8Out[i];
		g_outputdata_JF7 += g_uSetJF7Out[i];
	}

	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
	Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
	Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
}

/*************************************************************************/

int main()
{
	GetParamsInialization();

	// g_fJF8_PWM_Frequency[0] = 10000.0;
	// g_fJF8_PWM_Duty[0] = 25.0;
	// g_fJF8_PWM_Delay[0] = 0.0;
	// g_dJF8_PWM_Ttotal[0] = (double)(1.0 / g_fJF8_PWM_Frequency[0]);
	// g_dJF8_PWM_Ton[0] = (double)(g_fJF8_PWM_Duty[0] * 0.01) * g_dJF8_PWM_Ttotal[0];
	
	// g_fJF8_PWM_Frequency[2] = 10000.0;
	// g_fJF8_PWM_Duty[2] = 45.0;
	// g_fJF8_PWM_Delay[0] = 0.0;
	// g_dJF8_PWM_Ttotal[2] = (double)(1.0 / g_fJF8_PWM_Frequency[2]);
	// g_dJF8_PWM_Ton[2] = (double)(g_fJF8_PWM_Duty[2] * 0.01) * g_dJF8_PWM_Ttotal[2];

	// g_iP2_FunctionType[0] = SAWTOOTH_ANALOG;
	// g_fP2_Anal_Freq[0] = 10000.0;
	// g_fP2_Anal_Amp[0] = 8.0;
	// g_fP2_Anal_Ratio[0] = 0.3;
	// g_fP2_Anal_Delay[0] = 2.0;
	// g_dAnal_Period[0] = (double)(1.0 / g_fP2_Anal_Freq[0]);
	// g_dAnal_Omega[0] = (double)(2.0 * PI * g_fP2_Anal_Freq[0]);

	// g_iP2_FunctionType[1] = TRIANGE_ANALOG;
	// g_fP2_Anal_Freq[1] = 10000.0;
	// g_fP2_Anal_Amp[1] = 8.0;
	// g_fP2_Anal_Ratio[1] = 0.5;
	// g_fP2_Anal_Delay[1] = 2.0;
	// g_dAnal_Period[1] = (double)(1.0 / g_fP2_Anal_Freq[1]);
	// g_dAnal_Omega[1] = (double)(2.0 * PI * g_fP2_Anal_Freq[1]);

	SetupInterrupt();

	int i;

	while (1)
	{
		for (i = 0; i < 16; i++)
		{
			GetPWM_JF8(i);
			GetPWM_JF7(i);
		}

		GetAnal_P2(0);
		GetAnal_P2(1);

		GetAllOutput();
	}

	return 0;
}
