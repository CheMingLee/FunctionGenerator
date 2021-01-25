#include "SettingDefine.h"

/*************************************************************************/

void GetAppCmd();
void SetLED();
void SetPWM(int iChOffset);
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

void SetPWM(int iChOffset)
{
	// 0 -> JF8, 16 -> JF7
	if (iChOffset == 0 || iChOffset == 16)
	{
		u32 u32Channel;
		Params_PWM PWM_Params;
		
		u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);
		u32Channel += iChOffset;
		memcpy(&PWM_Params.m_fFreq, IO_ADDR_BRAM_IN_DATA + 4, 4);
		memcpy(&PWM_Params.m_fDuty, IO_ADDR_BRAM_IN_DATA + 8, 4);
		memcpy(&PWM_Params.m_fDelay, IO_ADDR_BRAM_IN_DATA + 12, 4);

		g_fPWM_Frequency[u32Channel] = PWM_Params.m_fFreq;
		g_fPWM_Duty[u32Channel] = PWM_Params.m_fDuty;
		g_fPWM_Delay[u32Channel] = PWM_Params.m_fDelay;

		if (PWM_Params.m_fFreq > 0.0 && PWM_Params.m_fDuty > 0.0)
		{
			g_dPWM_Ttotal[u32Channel] = (double)(1.0 / PWM_Params.m_fFreq);
			g_dPWM_Ton[u32Channel] = (double)(PWM_Params.m_fDuty * 0.01) * g_dPWM_Ttotal[u32Channel];
		}
	}
}

/*************************************************************************/

void SetAnal_P2(int iCH)
{
	if (iCH == 0 || iCH == 1)
	{
		Params_Analog P2_Params;

		P2_Params.m_iFuncType = Xil_In32(IO_ADDR_BRAM_IN_DATA);
		memcpy(&P2_Params.m_fFreq, IO_ADDR_BRAM_IN_DATA + 4, 4);
		memcpy(&P2_Params.m_fAmp, IO_ADDR_BRAM_IN_DATA + 8, 4);
		memcpy(&P2_Params.m_fRatio, IO_ADDR_BRAM_IN_DATA + 12, 4);
		memcpy(&P2_Params.m_fDelay, IO_ADDR_BRAM_IN_DATA + 16, 4);

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

void SetLED()
{
	u32 setLED_output;
	u32 uLedStatus;
	u16 usAsk;
	
	setLED_output = Xil_In32(IO_ADDR_BRAM_IN_DATA);
	Xil_Out32(IO_ADDR_LEDOUT, setLED_output);

	usAsk = CMD_SETLED;
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK, usAsk);
	Xil_Out16(IO_ADDR_BRAM_OUT_ASK_SIZE, sizeof(usAsk));
	uLedStatus = Xil_In32(IO_ADDR_LEDOUT_STATUS);
	Xil_Out32(IO_ADDR_BRAM_OUT_DATA, uLedStatus);
	Xil_Out32(IO_ADDR_BRAM_OUT_SIZE, sizeof(uLedStatus)+4);
}

/*************************************************************************/

bool CheckFlag()
{
	bool bFlag;
	u32 uFlag;
	u32 mask;
	int i;

	uFlag = Xil_In32(IO_ADDR_BRAM_IN_FLAG);
	i = 0;
	mask = 1 << i;
	bFlag = false;

	if (uFlag == mask)
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
		uCmd = Xil_In32(IO_ADDR_BRAM_IN_CMD);

		switch (uCmd)
		{
			case CMD_SETLED:
			{
				SetLED();
				SetFlagOutOne();
				SetFlagInZero();
				break;
			}

			case CMD_SETOUTPUT:
			{
				SetPWM(0);
				SetFlagInZero();
				XTime_GetTime(&g_XT_JF8_Delay_Start);
				break;
			}

			case CMD_SETOUTPUTEX:
			{
				SetPWM(16);
				SetFlagInZero();
				XTime_GetTime(&g_XT_JF7_Delay_Start);
				break;
			}
			
			case CMD_SETANALOG1OUT:
			{
				SetAnal_P2(0);
				SetFlagInZero();
				XTime_GetTime(&g_XT_P2Ch1_Delay_Start);
				break;
			}

			case CMD_SETANALOG2OUT:
			{
				SetAnal_P2(1);
				SetFlagInZero();
				XTime_GetTime(&g_XT_P2Ch2_Delay_Start);
				break;
			}

			default:
			{
				break;
			}
		}
	}
}

/*************************************************************************/
