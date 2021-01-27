#include "SettingDefine.h"

/*************************************************************************/

// SetupInterrupt.c
void SetupInterrupt();

/*************************************************************************/

// Time
XTime g_XT_Start, g_XT_End;
XTime g_XT_Delay_Start;
XTime g_XT_RunTime_Start, g_XT_RunTime_End;
double g_dRunTime;

// flag
bool g_bflag_start;

// input
int g_iPWM_flag[32];
float g_fPWM_Frequency[32];			// Hz
float g_fPWM_Duty[32];				// 0-100
float g_fPWM_Delay[32];				// s
int g_iP2_FunctionType[2];
float g_fP2_Anal_Freq[2];			// Hz
float g_fP2_Anal_Amp[2];			// V
float g_fP2_Anal_Ratio[2];			// 0-1
float g_fP2_Anal_Delay[2];			// s

// PWM
double g_dPWM_Ttotal[32];			// s
double g_dPWM_Ton[32];				// s

// Analog
double g_dAnal_Period[2];			// s
double g_dAnal_Omega[2];			// rad/s

// output
u32 g_uOutSetting[16];
u32 g_uSetPWMOut[32];
u32 g_outputdata_JF8;
u32 g_outputdata_JF7;
u32 g_outputdata_P2[2];

/*************************************************************************/

void GetParamsInialization()
{
	int i;
	for (i = 0; i < 32; i++)
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

		if (i < 16)
		{
			g_uOutSetting[i] = 1 << i;
		}
		
		g_iPWM_flag[i] = 0;
		g_fPWM_Frequency[i] = 0.0;
		g_fPWM_Duty[i] = 0.0;
		g_fPWM_Delay[i] = 0.0;

		g_dPWM_Ttotal[i] = 0.0;
		g_dPWM_Ton[i] = 0.0;

		g_uSetPWMOut[i] = 0;
	}

	g_outputdata_JF8 = 0;
	g_outputdata_JF7 = 0;

	XTime_GetTime(&g_XT_Start);
}

/*************************************************************************/

void GetPWM(int iCH)
{
	long long lPeriodCnt, lTonCnt, lDelayCnt;
	int iCH_bit;

	if (iCH < 16)
	{
		iCH_bit = iCH;
	}
	else
	{
		iCH_bit = iCH - 16;
	}

	XTime_GetTime(&g_XT_End);

	lPeriodCnt = g_dPWM_Ttotal[iCH] * COUNTS_PER_SECOND;
	lTonCnt = g_dPWM_Ton[iCH] * COUNTS_PER_SECOND;
	lDelayCnt = g_fPWM_Delay[iCH] * COUNTS_PER_SECOND;

	if ((g_XT_End - g_XT_Delay_Start) >= lDelayCnt)
	{
		if (g_fPWM_Frequency[iCH] > 0.0 && g_fPWM_Duty[iCH] > 0.0)
		{
			if (((g_XT_End - g_XT_Start) % lPeriodCnt) >= lTonCnt)
			{
				g_uSetPWMOut[iCH] = 0;
			}
			else
			{
				g_uSetPWMOut[iCH] = g_uOutSetting[iCH_bit];
			}
		}
		else
		{
			g_uSetPWMOut[iCH] = 0;
		}
	}
	else
	{
		g_uSetPWMOut[iCH] = 0;
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
	
	if (iCH == 0 || iCH == 1)
	{
		XTime_GetTime(&g_XT_End);

		lPeriodCnt = g_dAnal_Period[iCH] * COUNTS_PER_SECOND;
		lDelayCnt = g_fP2_Anal_Delay[iCH] * COUNTS_PER_SECOND;

		if ((g_XT_End - g_XT_Delay_Start) >= lDelayCnt)
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

	if (iCH == 0 || iCH == 1)
	{
		XTime_GetTime(&g_XT_End);

		lPeriodCnt = g_dAnal_Period[iCH] * COUNTS_PER_SECOND;
		lRatioPeriodCnt = g_fP2_Anal_Ratio[iCH] * lPeriodCnt;
		lDownRatioPeriodCnt = lPeriodCnt - lRatioPeriodCnt;
		lDelayCnt = g_fP2_Anal_Delay[iCH] * COUNTS_PER_SECOND;

		if ((g_XT_End - g_XT_Delay_Start) >= lDelayCnt)
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
	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
	Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
	Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
}

/*************************************************************************/

int main()
{
	GetParamsInialization();

	SetupInterrupt();

	int i;
	g_bflag_start = false;

	while (1)
	{
		XTime_GetTime(&g_XT_RunTime_Start);

		if (g_bflag_start)
		{
			g_outputdata_JF8 = 0;
			g_outputdata_JF7 = 0;

			for (i = 0; i < 32; i++)
			{
				if (g_iPWM_flag[i] > 0)
				{
					GetPWM(i);
				}
				else
				{
					g_uSetPWMOut[i] = 0;
				}

				if (i < 16)
				{
					g_outputdata_JF8 += g_uSetPWMOut[i];
				}
				else
				{
					g_outputdata_JF7 += g_uSetPWMOut[i];
				}
			}

			if (g_iP2_FunctionType[0] > 0)
			{
				GetAnal_P2(0);
			}
			else
			{
				g_outputdata_P2[0] = 0;
			}

			if (g_iP2_FunctionType[1] > 0)
			{
				GetAnal_P2(1);
			}
			else
			{
				g_outputdata_P2[1] = 0;
			}
		}
		else
		{
			GetParamsInialization();
		}

		GetAllOutput();

		XTime_GetTime(&g_XT_RunTime_End);

		g_dRunTime = (double)(g_XT_RunTime_End - g_XT_RunTime_Start) / (double)COUNTS_PER_SECOND * 1000000.0;
	}

	return 0;
}
