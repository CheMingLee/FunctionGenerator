#include "SetupInterrupt.c"
#include "SettingDefine.h"

// input
double g_dPWM_Frequency; // Hz
double g_dPWM_Duty; // 0-100

// output
u32 g_outputdata_JF8;
int g_iOutput;
u32 g_mask;
int g_PWM_TickCount; // Unit: 1 microsecond

// LED test
u32 g_setLED_output;

int main()
{
	// set input PWM
	g_dPWM_Frequency = 10000.0;
	g_dPWM_Duty = 50;

	// select output
    g_iOutput = 0;
    g_outputdata_JF8 = Xil_In32(IO_ADDR_OUTPUT_STATUS);
    g_outputdata_JF8 = 0;
    g_mask = 1 << (g_iOutput);

	// start
	g_PWM_TickCount = 0;

	g_setLED_output = 0;

	SetupInterrupt();

	while (1)
	{
		Xil_Out32(IO_ADDR_LEDOUT, g_setLED_output);
		// Xil_Out32(IO_ADDR_OUTPUT, g_outputdata_JF8);
	}

	return 0;
}
