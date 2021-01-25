#include "SettingDefine.h"

/*************************************************************************/

typedef struct {
	u32 OutputHz; /* Output frequency */
	u16 Interval; /* Interval value */
	u8 Prescaler; /* Prescaler value */
	u16 Options; /* Option settings */
} TmrCntrSetup;

static TmrCntrSetup SettingsTable[1] = {
	{HZ_INTERRUPT, 0, 0, 0}, /* Ticker timer counter initial setup, only output freq */
};

/*************************************************************************/

XTtcPs Timer;
XScuGic Intc; //GIC

/*************************************************************************/

// GenerateFunction.c
void GetAppCmd();

void SetupInterrupt();
void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
void TickHandler(void *CallBackRef);

/*************************************************************************/

void SetupInterrupt()
{
	XTtcPs_Config *Config;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable[TTC_DEVICE_ID];

	XTtcPs_Stop(&Timer);

 	//initialise the timer
 	Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
 	XTtcPs_CfgInitialize(&Timer, Config, Config->BaseAddress);

 	TimerSetup->Options |= (XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);

 	XTtcPs_SetOptions(&Timer, TimerSetup->Options);
 	XTtcPs_CalcIntervalFromFreq(&Timer, TimerSetup->OutputHz,&(TimerSetup->Interval), &(TimerSetup->Prescaler));

    XTtcPs_SetInterval(&Timer, TimerSetup->Interval);
    XTtcPs_SetPrescaler(&Timer, TimerSetup->Prescaler);

    SetupInterruptSystem(&Intc, &Timer);
}

void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt)
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

void TickHandler(void *CallBackRef)
{
	u32 StatusEvent;

	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

	GetAppCmd();
}
