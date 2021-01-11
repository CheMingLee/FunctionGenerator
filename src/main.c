#include "math.h"
#include "stdlib.h"
#include "Xscugic.h"
#include "Xil_exception.h"
#include "xttcps.h"
#include "xparameters.h"
#include "xil_io.h"

// math define
#define PI acos(-1)
#define UNIT_TIME 1000000 // Unit: 1 microsecond
#define HZ_INTERRUPT 1000000

// CMD define
#define NO_FUNCTION 0
#define SINE_WAVE 1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3

// output define
#define IO_ADDR_OUTPUT				(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 0)
#define IO_ADDR_OUTPUT_EX			(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 4)
#define IO_ADDR_LEDOUT				(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 68)
#define IO_ADDR_8BIT_OUT			(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 72)
#define IO_ADDR_OUTPUT_STATUS	    (XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 8)
#define IO_ADDR_OUTPUT_EX_STATUS	(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 12)
#define IO_ADDR_LEDOUT_STATUS		(XPAR_IO_CONTROL_0_S00_AXI_BASEADDR + 24)
#define LCTRL_ADDR_ANALOG1_OUT		(XPAR_LASER_CONTROL_0_S00_AXI_BASEADDR + 152)
#define LCTRL_ADDR_ANALOG2_OUT		(XPAR_LASER_CONTROL_0_S00_AXI_BASEADDR + 156)

// TTC define
#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

typedef struct {
	u32 OutputHz;	/* Output frequency */
	u16 Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

static TmrCntrSetup SettingsTable[1] = {
	{HZ_INTERRUPT, 0, 0, 0},	/* Ticker timer counter initial setup, only output freq */
};

typedef struct {
	int FunctionType;	// Function type
	double Frequency;	// Frequency
	double Amplitude;	// Amplitude
	double Ratio; // Sawtooth Wave Period Ratio
} AnalCmd;

static AnalCmd Anal_Input[2] = {
	{SINE_WAVE, 10000.0, 5.5, 0},
	{TRIANGLE_WAVE, 10000.0, 5.5, 0}
};

static XScuGic Intc; //GIC

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
static void TickHandler(void *CallBackRef);

// output functions
u32 GetSineWaveData(double dFreq, double dAmp);
u32 GetTriangleWaveData();
u32 GetSawtoothWaveData();
u32 PWMWave();

// input
double g_dPWM_Frequency; // Hz
double g_dPWM_Duty; // 0-100
double g_dPWM_Toff; // Unit: 1 microsecond
double g_dPWM_Ton; // Unit: 1 microsecond
double g_dAnal_Frequency[2]; // Hz
double g_dAnal_Amp[2]; // V
double g_dSine_Omega; // rad per microsecond
int g_iTriangle_HalfPeriod; // Unit: 1 microsecond
int g_iSawtooth_Period; // Unit: 1 microsecond

// output
u32 g_outputdata_Sine;
int g_outputdata_P2[2];
u32 g_outputdata_JF8;
int g_iOutput;
u32 g_mask;
int g_PWM_TickCount; // Unit: 1 microsecond
u32 g_Anal_TickCount[2]; // Unit: 1 microsecond

int main()
{
	// set input PWM
	g_dPWM_Frequency = 10000.0;
	g_dPWM_Duty = 50;
	double dPeriod = (1.0 / g_dPWM_Frequency) * UNIT_TIME;
	g_dPWM_Ton = g_dPWM_Duty / g_dPWM_Frequency / 100 * UNIT_TIME;
	g_dPWM_Toff = dPeriod - g_dPWM_Ton;

	// set input analog
	// g_dAnal_Frequency[0] = 10000.0;
	// g_dAnal_Frequency[1] = 10000.0;
	// g_dAnal_Amp[0] = 5.5;
	// g_dAnal_Amp[1] = 5.5;
	// g_dSine_Omega = 2.0 * PI * g_dAnal_Frequency[0] / UNIT_TIME;
	// g_iTriangle_HalfPeriod = round(1.0 / g_dAnal_Frequency[1] * 0.5 * UNIT_TIME);
	// g_iSawtooth_Period = round(1.0 / g_dAnal_Frequency[1] * UNIT_TIME);

	XTtcPs_Config *Config;
	XTtcPs Timer;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable[TTC_DEVICE_ID];
	XTtcPs_Stop(&Timer);

	//initialise the timer
	Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
	XTtcPs_CfgInitialize(&Timer, Config, Config->BaseAddress);
	TimerSetup->Options |= (XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);
	XTtcPs_SetOptions(&Timer, TimerSetup->Options);
	XTtcPs_CalcIntervalFromFreq(&Timer, TimerSetup->OutputHz, &(TimerSetup->Interval), &(TimerSetup->Prescaler));
	XTtcPs_SetInterval(&Timer, TimerSetup->Interval);
	XTtcPs_SetPrescaler(&Timer, TimerSetup->Prescaler);

    SetupInterruptSystem(&Intc, &Timer);

	// select output
    g_iOutput = 0;
    g_outputdata_JF8 = Xil_In32(IO_ADDR_OUTPUT_STATUS);
    g_outputdata_JF8 = 0;
    g_mask = 1 << (g_iOutput);

	// start
	g_PWM_TickCount = 0;
	g_Anal_TickCount[0] = 0;
	g_Anal_TickCount[1] = 0;
	g_outputdata_P2[0] = 0;
	g_outputdata_P2[1] = 0;

    while(1)
	{
    	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);

		Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2[0]);
		Xil_Out32(LCTRL_ADDR_ANALOG2_OUT, g_outputdata_P2[1]);
    }

    return 0;
}

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt)
{
	XScuGic_Config *IntcConfig; //GIC config
	Xil_ExceptionInit();

	//initialise the GIC
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);

	//connect to the hardware
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, GicInstancePtr);
	XScuGic_Connect(GicInstancePtr, TTC_INTR_ID, (Xil_ExceptionHandler)TickHandler, (void *)TtcPsInt);
	XScuGic_Enable(GicInstancePtr, TTC_INTR_ID);
	XTtcPs_EnableInterrupts(TtcPsInt, XTTCPS_IXR_INTERVAL_MASK);
	XTtcPs_Start(TtcPsInt);

	// Enable interrupts in the Processor.
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
}

static void TickHandler(void *CallBackRef)
{
	u32 StatusEvent;

	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

	// Sine
	g_outputdata_Sine = GetSineWaveData(double dFreq, double dAmp);

	// Sine
	// if (g_dSine_Omega * g_Anal_TickCount[0] >= 2.0 * PI)
	// {
	// 	g_Anal_TickCount[0] = 0;
	// }
	// else
	// {
	// 	g_Anal_TickCount[0]++;
	// }
	// g_Anal_TickCount[0]++;
	// g_outputdata_P2[0] = round(32767.0 / 5.5 * (g_dAnal_Amp[0] * sin(g_dSine_Omega * g_Anal_TickCount[0]))) + 32767;

	// Triangle
	// g_Anal_TickCount[1]++;
	// g_outputdata_P2[1] = round(65535.0 / 11.0 * (2.0 * g_dAnal_Amp[1] / g_iTriangle_HalfPeriod) * (g_iTriangle_HalfPeriod - abs(g_Anal_TickCount[1] % (2 * g_iTriangle_HalfPeriod) - g_iTriangle_HalfPeriod)));

	// Sawtooth
	// g_Anal_TickCount[1]++;
	// g_outputdata_P2[1] = round(65535.0 / 11.0 * (2.0 * g_dAnal_Amp[1] / g_iSawtooth_Period) * (g_iSawtooth_Period - abs((g_Anal_TickCount[1] % g_iSawtooth_Period) - g_iSawtooth_Period)));
	
	// PWM
	if (g_PWM_TickCount < g_dPWM_Toff)
	{
		g_outputdata_JF8 = 0;

		if (g_PWM_TickCount >= (g_dPWM_Toff + g_dPWM_Ton - 1))
		{
			g_PWM_TickCount = 0;
		}
		else
		{
			g_PWM_TickCount++;
		}
	}
	else
	{
		g_outputdata_JF8 = g_outputdata_JF8 | g_mask;

		if (g_PWM_TickCount >= (g_dPWM_Toff + g_dPWM_Ton - 1))
		{
			g_PWM_TickCount = 0;
		}
		else
		{
			g_PWM_TickCount++;
		}
	}
}

u32 GetSineWaveData(double dFreq, double dAmp)
{
	static u32 iSineTickCount;
	double dOmega = 2.0 * PI * dFreq / UNIT_TIME;
	iSineTickCount++;

	return round(32767.0 / 5.5 * (dAmp * sin(dOmega * iSineTickCount))) + 32767;
}

