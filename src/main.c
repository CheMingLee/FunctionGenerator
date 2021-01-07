#include "Xscugic.h"
#include "Xil_exception.h"
#include "xttcps.h"
#include "xparameters.h"
#include "xil_io.h"

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
	{100000, 0, 0, 0},	/* Ticker timer counter initial setup, only output freq */
};

static XScuGic Intc; //GIC

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
static void TickHandler(void *CallBackRef);

// input
u32 PWM_Frequency;
u32 PWM_Duty;
int PWM_Count;

// output
u32 outputdata_JF8;
int i;
u32 mask;
int TickCount;

int main()
{	
	// setup input
	PWM_Frequency = 2;
	PWM_Duty = 20;
	PWM_Count = 100 / PWM_Duty - 1;
	
	XTtcPs_Config *Config;
	XTtcPs Timer;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable[TTC_DEVICE_ID];
	TimerSetup->OutputHz = 100 * PWM_Frequency / PWM_Duty;

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
    i = 0;
    outputdata_JF8 = Xil_In32(IO_ADDR_OUTPUT_STATUS);
    mask = 1 << (i);

	// start
	TickCount = 0;

    while(1)
	{
    	Xil_Out32(IO_ADDR_OUTPUT, outputdata_JF8);
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

	if (TickCount == PWM_Count)
	{
		outputdata_JF8 = outputdata_JF8 | mask;
		TickCount = 0;
	}
	else
	{
		outputdata_JF8 = 0;
		TickCount++;
	}
}
