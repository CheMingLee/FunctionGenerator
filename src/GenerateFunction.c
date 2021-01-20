#include "SettingDefine.h"

/*************************************************************************/

static void GetAppCmd();
static void SetLED();
static float* GetPWM_Params();
static u32 GetPWM_CH_OnOff(bool bOnOff, u32 uOutStatus, u32 uCH);
static void SetPWM_JF8();
static void SetPWM_JF7();
static float* GetAnal_Params();
static void SetAnal_Function(int iCH);
static void SetAnalog_1();
static void SetAnalog_2();
static bool CheckFlag();
static void SetFlagInZero();
static void SetFlagOutOne();

/*************************************************************************/

// input
extern float g_fJF8_PWM_Frequency[16]; // Hz
extern float g_fJF8_PWM_Duty[16]; // 0-100
extern float g_fJF8_PWM_Delay[16]; // s
extern float g_fJF7_PWM_Frequency[16]; // Hz
extern float g_fJF7_PWM_Duty[16]; // 0-100
extern float g_fJF7_PWM_Delay[16]; // s
extern int g_iP2_FunctionType[2];
extern float g_fP2_Anal_Freq[2]; // Hz
extern float g_fP2_Anal_Amp[2]; // V
extern float g_fP2_Anal_Ratio[2]; // 0-1
extern float g_fP2_Anal_Delay[2]; // s

// output
extern u32 g_outputdata_JF8;
extern u32 g_outputdata_JF7;
extern u32 g_outputdata_P2[2];

/*************************************************************************/

static float* GetPWM_Params()
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

static u32 GetPWM_CH_OnOff(bool bOnOff, u32 uOutStatus, u32 uCH)
{
	u32 mask;
	u32 uOutData;

	uOutData = uOutStatus;
	mask = 1 << uCH;

	if (bOnOff)
	{
		uOutData |= mask;
	}
	else
	{
		uOutData &= ~mask;
	}

	return uOutData;
}

static void SetPWM_JF8()
{
	u32 u32Channel, u32OutStatus;
	float* pParams;
	
	u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);

	pParams = GetPWM_Params();

	g_fJF8_PWM_Frequency[u32Channel] = pParams[0];
	g_fJF8_PWM_Duty[u32Channel] = pParams[1];
	g_fJF8_PWM_Delay[u32Channel] = pParams[2];

	u32OutStatus = Xil_In32(IO_ADDR_OUTPUT_STATUS);
	g_outputdata_JF8 = u32OutStatus;

	if (pParams[0] <= 0.0 || pParams[1] <= 0.0)
	{
		g_outputdata_JF8 = GetPWM_CH_OnOff(false, u32OutStatus, u32Channel);
	}
	else
	{
		g_outputdata_JF8 = GetPWM_CH_OnOff(true, u32OutStatus, u32Channel);
	}

	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
}

static void SetPWM_JF7()
{
	u32 u32Channel, u32OutStatus;
	float* pParams;
	
	u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);

	pParams = GetPWM_Params();

	g_fJF7_PWM_Frequency[u32Channel] = pParams[0];
	g_fJF7_PWM_Duty[u32Channel] = pParams[1];
	g_fJF7_PWM_Delay[u32Channel] = pParams[2];

	u32OutStatus = Xil_In32(IO_ADDR_OUTPUT_EX_STATUS);

	if (pParams[0] <= 0.0 || pParams[1] <= 0.0)
	{
		g_outputdata_JF7 = GetPWM_CH_OnOff(false, u32OutStatus, u32Channel);
	}
	else
	{
		g_outputdata_JF7 = GetPWM_CH_OnOff(true, u32OutStatus, u32Channel);
	}

	Xil_Out32(IO_ADDR_OUTPUT_EX, g_outputdata_JF7);
}

/*************************************************************************/

static float* GetAnal_Params()
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

static void SetAnal_Function(int iCH)
{
	if (iCH == 0 || iCH == 1)
	{
		int iFuncType;
		float* pParams;
		u32 u32OutData;
	
		iFuncType = Xil_In32(IO_ADDR_BRAM_IN_DATA);
		pParams = GetAnal_Params();

		g_iP2_FunctionType[iCH] = iFuncType;
		g_fP2_Anal_Freq[iCH] = pParams[0];
		g_fP2_Anal_Amp[iCH] = pParams[1];
		g_fP2_Anal_Ratio[iCH] = pParams[2];
		g_fP2_Anal_Delay[iCH] = pParams[3];

		u32OutData = round(65535.0 / 11.0 * pParams[1]);

		switch (iFuncType)
		{
			case CLOSE_ANALOG:
			{
				u32OutData = 0;
				break;
			}

			case SINE_ANALOG:
			{
				u32OutData = round(u32OutData / 2.0);
				break;
			}
			
			case TRIANGE_ANALOG:
			{
				break;
			}

			case SAWTOOTH_ANALOG:
			{
				break;
			}

			default:
			{
				break;
			}
		}

		g_outputdata_P2[iCH] = u32OutData;
	}
}

static void SetAnalog_1()
{
	SetAnal_Function(0);
}

static void SetAnalog_2()
{
	SetAnal_Function(1);
}

/*************************************************************************/

static void SetLED()
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

static bool CheckFlag()
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

static void SetFlagInZero()
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

static void SetFlagOutOne()
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

static void GetAppCmd()
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
				break;
			}

			case CMD_SETOUTPUTEX:
			{
				SetPWM_JF7();
				SetFlagInZero();
				break;
			}
			
			case CMD_SETANALOG1OUT:
			{
				SetAnalog_1();
				SetFlagInZero();
				break;
			}

			case CMD_SETANALOG2OUT:
			{
				SetAnalog_2();
				SetFlagInZero();
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

// input
// double g_dPWM_Frequency; // Hz
// double g_dPWM_Duty; // 0-100
// double dPWM_Toff; // Unit: 1 microsecond
// double dPWM_Ton; // Unit: 1 microsecond
// double g_dAnal_Frequency[2]; // Hz
// double g_dAnal_Amp[2]; // V
// double g_dSine_Omega; // rad per microsecond
// int g_iTriangle_HalfPeriod; // Unit: 1 microsecond
// int g_iSawtooth_Period; // Unit: 1 microsecond

// output
// u32 g_outputdata_Sine;
// int g_outputdata_P2[2];
// u32 g_outputdata_JF8;
// int g_iOutput;
// u32 g_mask;
// int g_PWM_TickCount; // Unit: 1 microsecond
// u32 g_Anal_TickCount[2]; // Unit: 1 microsecond

// set input analog
// g_dAnal_Frequency[0] = 10000.0;
// g_dAnal_Frequency[1] = 10000.0;
// g_dAnal_Amp[0] = 5.5;
// g_dAnal_Amp[1] = 5.5;
// g_dSine_Omega = 2.0 * PI * g_dAnal_Frequency[0] / UNIT_TIME;
// g_iTriangle_HalfPeriod = round(1.0 / g_dAnal_Frequency[1] * 0.5 * UNIT_TIME);
// g_iSawtooth_Period = round(1.0 / g_dAnal_Frequency[1] * UNIT_TIME);

// Sine
// if (g_dSine_Omega * g_Anal_TickCount[0] >= 2.0 * PI)
// {
//  	g_Anal_TickCount[0] = (g_dSine_Omega * g_Anal_TickCount[0] - 2.0 * PI) / g_dSine_Omega;
// }
// else
// {
//  	g_Anal_TickCount[0]++;
// }
// g_outputdata_P2[0] = round(32767.0 / 5.5 * (g_dAnal_Amp[0] * sin(g_dSine_Omega * g_Anal_TickCount[0]))) + 32767;

// u32 GetSineWaveData(double dFreq, double dAmp)
// {
// 	static u32 iSineTickCount;
// 	double dOmega = 2.0 * PI * dFreq / UNIT_TIME;
// 	iSineTickCount++;

// 	return round(32767.0 / 5.5 * (dAmp * sin(dOmega * iSineTickCount))) + 32767;
// }

// Triangle
// g_Anal_TickCount[1]++;
// g_outputdata_P2[1] = round(65535.0 / 11.0 * (2.0 * g_dAnal_Amp[1] / g_iTriangle_HalfPeriod) * (g_iTriangle_HalfPeriod - abs(g_Anal_TickCount[1] % (2 * g_iTriangle_HalfPeriod) - g_iTriangle_HalfPeriod)));

// Sawtooth
// g_Anal_TickCount[1]++;
// g_outputdata_P2[1] = round(65535.0 / 11.0 * (2.0 * g_dAnal_Amp[1] / g_iSawtooth_Period) * (g_iSawtooth_Period - abs((g_Anal_TickCount[1] % g_iSawtooth_Period) - g_iSawtooth_Period)));



