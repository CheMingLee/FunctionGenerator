#include "Xscugic.h"
#include "Xil_exception.h"
#include "xttcps.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "GenerateFunction.c"

#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID

#define HZ_INTERRUPT		100

typedef struct {
	u32 OutputHz;	/* Output frequency */
	u16 Interval;	/* Interval value */
	u8 Prescaler;	/* Prescaler value */
	u16 Options;	/* Option settings */
} TmrCntrSetup;

static TmrCntrSetup SettingsTable[1] = {
	{HZ_INTERRUPT, 0, 0, 0},	/* Ticker timer counter initial setup, only output freq */
};

static XScuGic Intc; //GIC

static void SetupInterrupt(void);
static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt);
static void TickHandler(void *CallBackRef);

static void SetupInterrupt(void)
{
	XTtcPs_Config *Config;
	XTtcPs Timer;
	TmrCntrSetup *TimerSetup;

	TimerSetup = &SettingsTable[TTC_DEVICE_ID];

	XTtcPs_Stop(&Timer);

 	//initialise the timer
 	Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
 	XTtcPs_CfgInitialize(&Timer, Config, Config->BaseAddress);

 	TimerSetup->Options |= (XTTCPS_OPTION_INTERVAL_MODE |
 							XTTCPS_OPTION_WAVE_DISABLE);

 	XTtcPs_SetOptions(&Timer, TimerSetup->Options);
 	XTtcPs_CalcIntervalFromFreq(&Timer, TimerSetup->OutputHz,&(TimerSetup->Interval), &(TimerSetup->Prescaler));

    XTtcPs_SetInterval(&Timer, TimerSetup->Interval);
    XTtcPs_SetPrescaler(&Timer, TimerSetup->Prescaler);

    SetupInterruptSystem(&Intc, &Timer);
}

static void SetupInterruptSystem(XScuGic *GicInstancePtr, XTtcPs *TtcPsInt)
{
	XScuGic_Config *IntcConfig; //GIC config
	Xil_ExceptionInit();

	//initialise the GIC
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

	XScuGic_CfgInitialize(GicInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);

	//connect to the hardware
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				GicInstancePtr);

	XScuGic_Connect(GicInstancePtr, TTC_INTR_ID,
			(Xil_ExceptionHandler)TickHandler, (void *)TtcPsInt);


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

				break;
			}
			
			case CMD_SETANALOG1OUT:
			{

				break;
			}

			case CMD_SETANALOG2OUT:
			{

				break;
			}

			default:
			{
				break;
			}
		}
	}

    // XTime start, end;
	// double timePWM;
	
	// XTime_GetTime(&start);
	// GetPWM();
	// XTime_GetTime(&end);
	// timePWM = (double)(end - start) / COUNTS_PER_SECOND * UNIT_TIME;
}
