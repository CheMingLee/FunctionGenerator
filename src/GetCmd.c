#include "SettingDefine.h"

/*************************************************************************/

void GetAppCmd();
void GetLED(u16 usAsk);
void GetChParams(u16 usAsk, u16 usSize);
void SetLED();
void SetPWM();
void SetAnal_P2(int iCH);
bool CheckFlag();
void SetFlagInZero();
void SetFlagOutOne();

/*************************************************************************/

typedef struct {
	float m_fFreq;
	float m_fDuty;
	float m_fDelay;
}Params_PWM;

typedef struct {
	int m_iFuncType;
	float m_fFreq;
	float m_fAmp;
	float m_fRatio;
	float m_fDelay;
}Params_Analog;

/*************************************************************************/

// Time
extern XTime g_XT_JF8_Delay_Start, g_XT_JF7_Delay_Start;
extern XTime g_XT_P2Ch1_Delay_Start, g_XT_P2Ch2_Delay_Start;

// input
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
	int iChannel;
	Params_PWM PWM_Params;
	
	memcpy(&iChannel, (void *)IO_ADDR_BRAM_IN_DATA, 4);
	memcpy(&PWM_Params.m_fFreq, (void *)IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&PWM_Params.m_fDuty, (void *)IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&PWM_Params.m_fDelay, (void *)IO_ADDR_BRAM_IN_DATA + 12, 4);

	g_fPWM_Frequency[iChannel] = PWM_Params.m_fFreq;
	g_fPWM_Duty[iChannel] = PWM_Params.m_fDuty;
	g_fPWM_Delay[iChannel] = PWM_Params.m_fDelay;

	if (PWM_Params.m_fFreq > 0.0 && PWM_Params.m_fDuty > 0.0)
	{
		g_dPWM_Ttotal[iChannel] = (double)(1.0 / PWM_Params.m_fFreq);
		g_dPWM_Ton[iChannel] = (double)(PWM_Params.m_fDuty * 0.01) * g_dPWM_Ttotal[iChannel];
	}
}

void GetChParams(u16 usAsk, u16 usSize)
{
	int iCH;
	char *pData;

	memcpy(&iCH, (void *)IO_ADDR_BRAM_IN_DATA, 4);

	switch (usAsk)
	{
		case CMD_GETDIGITAL_FREQ:
		{
			pData = (char *)&g_fPWM_Frequency[iCH];
			break;
		}
		case CMD_GETDIGITAL_DUTY:
		{
			pData = (char *)&g_fPWM_Duty[iCH];
			break;
		}
		case CMD_GETDIGITAL_DELAY:
		{
			pData = (char *)&g_fPWM_Delay[iCH];
			break;
		}
		case CMD_GETANALOG_FUNCTION:
		{
			pData = (char *)&g_iP2_FunctionType[iCH];
			break;
		}
		case CMD_GETANALOG_FREQ:
		{
			pData = (char *)&g_fP2_Anal_Freq[iCH];
			break;
		}
		case CMD_GETANALOG_AMP:
		{
			pData = (char *)&g_fP2_Anal_Amp[iCH];
			break;
		}
		case CMD_GETANALOG_RATIO:
		{
			pData = (char *)&g_fP2_Anal_Ratio[iCH];
			break;
		}
		case CMD_GETANALOG_DELAY:
		{
			pData = (char *)&g_fP2_Anal_Delay[iCH];
			break;
		}
		default:
		{
			break;
		}
	}

	Xil_Out16(IO_ADDR_BRAM_OUT_ASK, usAsk);
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK_SIZE, usSize);

	memcpy((void *)IO_ADDR_BRAM_OUT_DATA, pData, 4);
	Xil_Out32(IO_ADDR_BRAM_OUT_SIZE, usSize+4);
}

/*************************************************************************/

void SetAnal_P2(int iCH)
{
	if (iCH == 0 || iCH == 1)
	{
		Params_Analog P2_Params;

		memcpy(&P2_Params.m_iFuncType, (void *)IO_ADDR_BRAM_IN_DATA, 4);
		memcpy(&P2_Params.m_fFreq, (void *)IO_ADDR_BRAM_IN_DATA + 4, 4);
		memcpy(&P2_Params.m_fAmp, (void *)IO_ADDR_BRAM_IN_DATA + 8, 4);
		memcpy(&P2_Params.m_fRatio, (void *)IO_ADDR_BRAM_IN_DATA + 12, 4);
		memcpy(&P2_Params.m_fDelay, (void *)IO_ADDR_BRAM_IN_DATA + 16, 4);

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
			case CMD_SETOUTPUT:
			{
				SetPWM();
				XTime_GetTime(&g_XT_JF8_Delay_Start);
				break;
			}
			case CMD_SETOUTPUTEX:
			{
				SetPWM();
				XTime_GetTime(&g_XT_JF7_Delay_Start);
				break;
			}
			case CMD_SETANALOG1OUT:
			{
				SetAnal_P2(0);
				XTime_GetTime(&g_XT_P2Ch1_Delay_Start);
				break;
			}
			case CMD_SETANALOG2OUT:
			{
				SetAnal_P2(1);
				XTime_GetTime(&g_XT_P2Ch2_Delay_Start);
				break;
			}
			case CMD_GETLED:
			{
				GetLED(CMD_GETLED);
				SetFlagOutOne();
				break;
			}
			case CMD_GETDIGITAL_FREQ:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETDIGITAL_DUTY:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETDIGITAL_DELAY:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG_FUNCTION:
			{
				GetChParams(uCmd, sizeof(int));
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG_FREQ:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG_AMP:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG_RATIO:
			{
				GetChParams(uCmd, sizeof(float));
				SetFlagOutOne();
				break;
			}
			case CMD_GETANALOG_DELAY:
			{
				GetChParams(uCmd, sizeof(float));
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
