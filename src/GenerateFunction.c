#include "SettingDefine.h"

// u32 GetSineWaveData(double dFreq, double dAmp);
// u32 GetTriangleWaveData();
// u32 GetSawtoothWaveData();
static void GetPWM();
static void SetLED();
static bool CheckFlag();
static void SetFlagInZero();
static void SetFlagOutOne();
/************************** Extern Variable *****************************/

// LED test
extern u32 g_setLED_output;

// input
extern double g_dPWM_Frequency; // Hz
extern double g_dPWM_Duty; // 0-100

// output
extern u32 g_outputdata_JF8;
extern int g_iOutput;
extern u32 g_mask;
extern int g_PWM_TickCount; // Unit: 1 microsecond

/*************************************************************************/

static void GetPWM()
{
    double dPWM_Toff; // Unit: 1 microsecond
    double dPWM_Ton; // Unit: 1 microsecond
    double dPeriod;
    dPeriod = (1.0 / g_dPWM_Frequency) * UNIT_TIME;
	dPWM_Ton = g_dPWM_Duty / g_dPWM_Frequency / 100 * UNIT_TIME;
	dPWM_Toff = dPeriod - dPWM_Ton;

    // if (g_PWM_TickCount < dPWM_Toff)
	// {
	// 	g_outputdata_JF8 = 0;

	// 	if (g_PWM_TickCount >= (dPWM_Toff + dPWM_Ton - 1))
	// 	{
	// 		g_PWM_TickCount = 0;
	// 	}
	// 	else
	// 	{
	// 		g_PWM_TickCount++;
	// 	}
	// }
	// else
	// {
	// 	g_outputdata_JF8 = g_outputdata_JF8 | g_mask;

	// 	if (g_PWM_TickCount >= (dPWM_Toff + dPWM_Ton - 1))
	// 	{
	// 		g_PWM_TickCount = 0;
	// 	}
	// 	else
	// 	{
	// 		g_PWM_TickCount++;
	// 	}
	// }
}

/*************************************************************************/

static void SetLED()
{
	u32 uLedStatus;
	u16 usAsk;
	
	g_setLED_output = Xil_In32(IO_ADDR_BRAM_IN_DATA);
	Xil_Out32(IO_ADDR_LEDOUT, g_setLED_output);
	SetFlagInZero();

	usAsk = CMD_SETLED;
	Xil_Out32(IO_ADDR_BRAM_OUT_ASK, usAsk);
	uLedStatus = Xil_In32(IO_ADDR_LEDOUT_STATUS);
	Xil_Out32(IO_ADDR_BRAM_OUT_DATA, uLedStatus);
	SetFlagOutOne();
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



