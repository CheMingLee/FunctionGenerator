#include "math.h"
#include "Xscugic.h"
#include "Xil_exception.h"
#include "xttcps.h"
#include "xparameters.h"
#include "xil_io.h"

// math define
#define PI acos(-1)

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
	{1000000, 0, 0, 0},	/* Ticker timer counter initial setup, only output freq */
};

static XScuGic Intc; //GIC

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
static void TickHandler(void *CallBackRef);

// input
double g_dPWM_Frequency; // Hz
double g_dPWM_Duty; // 0-100
double g_dPWM_Toff; // Unit: 1 microsecond
double g_dPWM_Ton; // Unit: 1 microsecond
double g_dAnal_Frequency; // Hz
double g_dAnal_Amp; // V
double g_dAnal_Omega; // rad per microsecond

// output
int g_outputdata_P2;
u32 g_outputdata_JF8;
int g_iOutput;
u32 g_mask;
int g_PWM_TickCount; // Unit: 1 microsecond
int g_Anal_TickCount; // Unit: 1 microsecond

int main()
{
	// set input PWM
	g_dPWM_Frequency = 10000.0;
	g_dPWM_Duty = 50;
	double dPeriod = (1.0 / g_dPWM_Frequency) * 1000000.0; // Unit: 1 microsecond
	g_dPWM_Ton = g_dPWM_Duty / g_dPWM_Frequency * 10000.0;
	g_dPWM_Toff = dPeriod - g_dPWM_Ton;

	// set input analog
	g_dAnal_Frequency = 10000.0;
	g_dAnal_Amp = 5.5;
	g_dAnal_Omega = 2.0*PI*g_dAnal_Frequency / 1000000.0;
	
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
	g_Anal_TickCount = 0;
	g_outputdata_P2 = 0;

    while(1)
	{
    	Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
		Xil_Out32(LCTRL_ADDR_ANALOG1_OUT, g_outputdata_P2);
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
	g_Anal_TickCount++;
	g_outputdata_P2 = round(32767.0 / 5.5 * (g_dAnal_Amp * sin(g_dAnal_Omega * g_Anal_TickCount))) + 32767;

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
