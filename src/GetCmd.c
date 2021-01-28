#include "SettingDefine.h"

/*************************************************************************/

typedef struct {
	int m_iChannel;
	int m_iflag;
	float m_fFreq;
	float m_fDuty;
	float m_fDelay;
}Params_PWM;

typedef struct {
	int m_iChannel;
	int m_iFuncType;
	float m_fFreq;
	float m_fAmp;
	float m_fRatio;
	float m_fDelay;
}Params_Analog;

/*************************************************************************/

// Time
extern XTime g_XT_Delay_Start;
extern double g_dRunTime;

// flag
extern bool g_bflag_start;

// input
extern int g_iPWM_flag[32];
extern float g_fPWM_Frequency[32];		// Hz
extern float g_fPWM_Duty[32];			// 0-100
extern float g_fPWM_Delay[32];			// s
extern int g_iP2_FunctionType[2];
extern float g_fP2_Anal_Freq[2];		// Hz
extern float g_fP2_Anal_Amp[2];			// V
extern float g_fP2_Anal_Ratio[2];		// 0-1
extern float g_fP2_Anal_Delay[2];		// s

// PWM
extern double g_dPWM_Ttotal[32];		// s
extern double g_dPWM_Ton[32];			// s

// Analog
extern double g_dAnal_Period[2];		// s
extern double g_dAnal_Omega[2];			// rad/s

/*************************************************************************/

void SetPWM()
{
	int iCH;
	Params_PWM PWM_Params;
	
	memcpy(&PWM_Params.m_iChannel, (void *)IO_ADDR_BRAM_IN_DATA, 4);
	memcpy(&PWM_Params.m_iflag, (void *)IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&PWM_Params.m_fFreq, (void *)IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&PWM_Params.m_fDuty, (void *)IO_ADDR_BRAM_IN_DATA + 12, 4);
	memcpy(&PWM_Params.m_fDelay, (void *)IO_ADDR_BRAM_IN_DATA + 16, 4);

	iCH = PWM_Params.m_iChannel;
	g_iPWM_flag[iCH] = PWM_Params.m_iflag;
	g_fPWM_Frequency[iCH] = PWM_Params.m_fFreq;
	g_fPWM_Duty[iCH] = PWM_Params.m_fDuty;
	g_fPWM_Delay[iCH] = PWM_Params.m_fDelay;

	if (PWM_Params.m_iflag > 0)
	{
		if (PWM_Params.m_fFreq > 0.0 && PWM_Params.m_fDuty > 0.0)
		{
			g_dPWM_Ttotal[iCH] = (double)(1.0 / PWM_Params.m_fFreq);
			g_dPWM_Ton[iCH] = (double)(PWM_Params.m_fDuty * 0.01) * g_dPWM_Ttotal[iCH];
		}
	}
}

void GetParamsToApp(u16 usAsk)
{
	int iCH;
	char *pData;
	u16 usSize;
	Params_PWM PWM_Params;
	Params_Analog P2_Params;

	memcpy(&iCH, (void *)IO_ADDR_BRAM_IN_DATA, 4);

	switch (usAsk)
	{
		case CMD_GETDIGITAL:
		{
			PWM_Params.m_iChannel = iCH;
			PWM_Params.m_iflag = g_iPWM_flag[iCH];
			PWM_Params.m_fFreq = g_fPWM_Frequency[iCH];
			PWM_Params.m_fDuty = g_fPWM_Duty[iCH];
			PWM_Params.m_fDelay = g_fPWM_Delay[iCH];
			pData = (char *)&PWM_Params;
			usSize = sizeof(PWM_Params);
			break;
		}
		case CMD_GETANALOG:
		{
			P2_Params.m_iChannel = iCH;
			P2_Params.m_iFuncType = g_iP2_FunctionType[iCH];
			P2_Params.m_fFreq = g_fP2_Anal_Freq[iCH];
			P2_Params.m_fAmp = g_fP2_Anal_Amp[iCH];
			P2_Params.m_fRatio = g_fP2_Anal_Ratio[iCH];
			P2_Params.m_fDelay = g_fP2_Anal_Delay[iCH];
			pData = (char *)&P2_Params;
			usSize = sizeof(P2_Params);
			break;
		}
		case CMD_GETRUNTIME:
		{
			pData = (char *)&g_dRunTime;
			usSize = sizeof(g_dRunTime);
			break;
		}
		default:
		{
			break;
		}
	}

	Xil_Out32(IO_ADDR_BRAM_OUT_SIZE, usSize+4);
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK, usAsk);
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK_SIZE, usSize);
	memcpy((void *)IO_ADDR_BRAM_OUT_DATA, pData, usSize);
}

/*************************************************************************/

void SetAnal_P2()
{
	Params_Analog P2_Params;
	int iCH;

	memcpy(&P2_Params.m_iChannel, (void *)IO_ADDR_BRAM_IN_DATA, 4);
	memcpy(&P2_Params.m_iFuncType, (void *)IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&P2_Params.m_fFreq, (void *)IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&P2_Params.m_fAmp, (void *)IO_ADDR_BRAM_IN_DATA + 12, 4);
	memcpy(&P2_Params.m_fRatio, (void *)IO_ADDR_BRAM_IN_DATA + 16, 4);
	memcpy(&P2_Params.m_fDelay, (void *)IO_ADDR_BRAM_IN_DATA + 20, 4);

	iCH = P2_Params.m_iChannel;
	g_iP2_FunctionType[iCH] = P2_Params.m_iFuncType;
	g_fP2_Anal_Freq[iCH] = P2_Params.m_fFreq;
	g_fP2_Anal_Amp[iCH] = P2_Params.m_fAmp;
	g_fP2_Anal_Ratio[iCH] = P2_Params.m_fRatio;
	g_fP2_Anal_Delay[iCH] = P2_Params.m_fDelay;

	if (P2_Params.m_fFreq > 0.0 && P2_Params.m_fAmp > 0.0)
	{
		g_dAnal_Period[iCH] = (double)(1.0 / P2_Params.m_fFreq);
		g_dAnal_Omega[iCH] = (double)(2.0 * PI * P2_Params.m_fFreq);
	}
}

/*************************************************************************/

void GetLED(u16 usAsk)
{
	u32 uLedStatus;

	Xil_Out16(IO_ADDR_BRAM_OUT_ASK, usAsk);
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK_SIZE, sizeof(uLedStatus));
	uLedStatus = Xil_In32(IO_ADDR_LEDOUT_STATUS);
	Xil_Out32(IO_ADDR_BRAM_OUT_DATA, uLedStatus);
	Xil_Out32(IO_ADDR_BRAM_OUT_SIZE, sizeof(uLedStatus)+4);
}

void SetLED()
{
	u32 setLED_output;
	
	setLED_output = Xil_In32(IO_ADDR_BRAM_IN_DATA);
	Xil_Out32(IO_ADDR_LEDOUT, setLED_output);
}

/*************************************************************************/

bool CheckFlag()
{
	bool bFlag;
	u32 uFlag;

	uFlag = Xil_In32(IO_ADDR_BRAM_IN_FLAG);
	bFlag = false;

	if (uFlag & 0x01)
	{
		bFlag = true;
	}
	
	return bFlag;
}

void SetFlagInZero()
{
	u32 uFlag;
	u32 mask;
	int i;
	i = 0;
	mask = 1 << i;
	uFlag = Xil_In32(IO_ADDR_BRAM_IN_FLAG);
	uFlag &= ~mask;
	Xil_Out32(IO_ADDR_BRAM_IN_FLAG, uFlag);
}

void SetFlagOutOne()
{
	u32 uFlag;
	u32 mask;
	int i;
	i = 1;
	mask = 1 << i;
	uFlag = Xil_In32(IO_ADDR_BRAM_IN_FLAG);
	uFlag |= mask;
	Xil_Out32(IO_ADDR_BRAM_IN_FLAG, uFlag);
}

/*************************************************************************/

void GetAppCmd()
{
	if (CheckFlag())
	{
		u16 uCmd;
		uCmd = Xil_In16(IO_ADDR_BRAM_IN_CMD);

		switch (uCmd)
		{
			case CMD_SETLED:
			{
				SetLED();
				GetLED(CMD_SETLED);
				SetFlagOutOne();
				break;
			}
			case CMD_SETDIGITAL:
			{
				SetPWM();
				break;
			}
			case CMD_SETANALOG:
			{
				SetAnal_P2();
				break;
			}
			case CMD_SETSTART:
			{
				XTime_GetTime(&g_XT_Delay_Start);
				g_bflag_start = true;
				break;
			}
			case CMD_SETSTOP:
			{
				g_bflag_start = false;
				break;
			}
			case CMD_GETLED:
			{
				GetLED(CMD_GETLED);
				SetFlagOutOne();
				break;
			}
			case CMD_GETDIGITAL:
			{
				GetParamsToApp(CMD_GETDIGITAL);
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG:
			{
				GetParamsToApp(CMD_GETANALOG);
				SetFlagOutOne();
				break;
			}
			case CMD_GETRUNTIME:
			{
				GetParamsToApp(CMD_GETRUNTIME);
				SetFlagOutOne();
				break;
			}
			default:
			{
				break;
			}
		}
		SetFlagInZero();
	}
}

/*************************************************************************/
