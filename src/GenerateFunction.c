#include "SettingDefine.h"

/*************************************************************************/

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

// Time
extern XTime g_XT_JF8_Delay_Start, g_XT_JF7_Delay_Start;
extern XTime g_XT_P2Ch1_Delay_Start, g_XT_P2Ch2_Delay_Start;

// input
extern float g_fJF8_PWM_Frequency[16];		// Hz
extern float g_fJF8_PWM_Duty[16];			// 0-100
extern float g_fJF8_PWM_Delay[16];			// s
extern float g_fJF7_PWM_Frequency[16];		// Hz
extern float g_fJF7_PWM_Duty[16];			// 0-100
extern float g_fJF7_PWM_Delay[16];			// s
extern int g_iP2_FunctionType[2];
extern float g_fP2_Anal_Freq[2];			// Hz
extern float g_fP2_Anal_Amp[2];				// V
extern float g_fP2_Anal_Ratio[2];			// 0-1
extern float g_fP2_Anal_Delay[2];			// s

// PWM
extern double g_dJF8_PWM_Ttotal[16];		// s
extern double g_dJF8_PWM_Ton[16];			// s
extern double g_dJF7_PWM_Ttotal[16];		// s
extern double g_dJF7_PWM_Ton[16];			// s

// Analog
extern double g_dAnal_Period[2];			// s
extern double g_dAnal_Omega[2];				// rad/s

// output
extern u32 g_outputdata_JF8;
extern u32 g_outputdata_JF7;
extern u32 g_outputdata_P2[2];

/*************************************************************************/

float* GetPWM_Params()
{
	float fParams[3];
	float fFreq, fDuty, fDelay;

	memcpy(&fFreq, IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&fDuty, IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&fDelay, IO_ADDR_BRAM_IN_DATA + 12, 4);

	fParams[0] = fFreq;
	fParams[1] = fDuty;
	fParams[2] = fDelay;

	return fParams;
}

void SetPWM_JF8()
{
	u32 u32Channel;
	float* pParams;
	
	u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);

	pParams = GetPWM_Params();

	g_fJF8_PWM_Frequency[u32Channel] = pParams[0];
	g_fJF8_PWM_Duty[u32Channel] = pParams[1];
	g_fJF8_PWM_Delay[u32Channel] = pParams[2];

	if (pParams[0] > 0.0 && pParams[1] > 0.0)
	{
		g_dJF8_PWM_Ttotal[u32Channel] = (double)(1.0 / g_fJF8_PWM_Frequency[u32Channel]);
		g_dJF8_PWM_Ton[u32Channel] = (double)(g_fJF8_PWM_Duty[u32Channel] * 0.01) * g_dJF8_PWM_Ttotal[u32Channel];
	}
}

void SetPWM_JF7()
{
	u32 u32Channel;
	float* pParams;
	
	u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);

	pParams = GetPWM_Params();

	g_fJF7_PWM_Frequency[u32Channel] = pParams[0];
	g_fJF7_PWM_Duty[u32Channel] = pParams[1];
	g_fJF7_PWM_Delay[u32Channel] = pParams[2];

	if (pParams[0] > 0.0 && pParams[1] > 0.0)
	{
		g_dJF7_PWM_Ttotal[u32Channel] = (double)(1.0 / g_fJF7_PWM_Frequency[u32Channel]);
		g_dJF7_PWM_Ton[u32Channel] = (double)(g_fJF7_PWM_Duty[u32Channel] * 0.01) * g_dJF7_PWM_Ttotal[u32Channel];
	}
}

/*************************************************************************/

float* GetAnal_Params()
{
	float fParams[4];
	float fFreq, fAmp, fRatio, fDelay;

	memcpy(&fFreq, IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&fAmp, IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&fRatio, IO_ADDR_BRAM_IN_DATA + 12, 4);
	memcpy(&fDelay, IO_ADDR_BRAM_IN_DATA + 16, 4);

	fParams[0] = fFreq;
	fParams[1] = fAmp;
	fParams[2] = fRatio;
	fParams[3] = fDelay;

	return fParams;
}

void SetAnal_P2(int iCH)
{
	if (iCH == 0 || iCH == 1)
	{
		int iFuncType;
		float* pParams;
	
		iFuncType = Xil_In32(IO_ADDR_BRAM_IN_DATA);
		pParams = GetAnal_Params();

		g_iP2_FunctionType[iCH] = iFuncType;
		g_fP2_Anal_Freq[iCH] = pParams[0];
		g_fP2_Anal_Amp[iCH] = pParams[1];
		g_fP2_Anal_Ratio[iCH] = pParams[2];
		g_fP2_Anal_Delay[iCH] = pParams[3];

		if (pParams[0] > 0.0 && pParams[1] > 0.0)
		{
			g_dAnal_Period[iCH] = (double)(1.0 / g_fP2_Anal_Freq[iCH]);
			g_dAnal_Omega[iCH] = (double)(2.0 * PI * g_fP2_Anal_Freq[iCH]);
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
				SetPWM_JF8();
				SetFlagInZero();
				XTime_GetTime(&g_XT_JF8_Delay_Start);
				break;
			}

			case CMD_SETOUTPUTEX:
			{
				SetPWM_JF7();
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
