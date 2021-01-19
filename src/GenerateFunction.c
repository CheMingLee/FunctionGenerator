#include "SettingDefine.h"

/*************************************************************************/

static void SetLED();
static void SetPWM_JF8();
static bool CheckFlag();
static void SetFlagInZero();
static void SetFlagOutOne();

/*************************************************************************/

// input
extern float g_dJF8_PWM_Frequency[16]; // Hz
extern float g_dJF8_PWM_Duty[16]; // 0-100
extern float g_dJF8_PWM_Delay[16]; // s

// output
extern u32 g_outputdata_JF8;

/*************************************************************************/

static void SetPWM_JF8()
{
	u32 mask;
	u32 u32Channel;
	float fFreq, fDuty, fDelay;

	u32Channel = Xil_In32(IO_ADDR_BRAM_IN_DATA);
	memcpy(&fFreq, IO_ADDR_BRAM_IN_DATA + 4, 4);
	memcpy(&fDuty, IO_ADDR_BRAM_IN_DATA + 8, 4);
	memcpy(&fDelay, IO_ADDR_BRAM_IN_DATA + 12, 4);
	g_dJF8_PWM_Frequency[u32Channel] = fFreq;
	g_dJF8_PWM_Duty[u32Channel] = fDuty;
	g_dJF8_PWM_Delay[u32Channel] = fDelay;

	g_outputdata_JF8 = Xil_In32(IO_ADDR_OUTPUT_STATUS);
	mask = 1 << u32Channel;

	if (g_dJF8_PWM_Frequency[u32Channel] > 0)
	{
		g_outputdata_JF8 |= mask;
	}
	else
	{
		g_outputdata_JF8 &= ~mask;
	}

	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
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



