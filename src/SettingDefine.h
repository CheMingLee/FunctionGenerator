#include "stdlib.h"
#include "stdbool.h"
#include "math.h"
#include "string.h"
#include "xparameters.h"
#include "xtime_l.h"
#include "xil_io.h"

// BRAM define
#define IO_ADDR_BRAM_IN_FLAG		(XPAR_BRAM_0_BASEADDR + 0)
#define IO_ADDR_BRAM_IN_CMD         (XPAR_BRAM_0_BASEADDR + 8)
#define IO_ADDR_BRAM_IN_DATA        (XPAR_BRAM_0_BASEADDR + 12)
#define IO_ADDR_BRAM_OUT_SIZE       (XPAR_BRAM_0_BASEADDR + 260)
#define IO_ADDR_BRAM_OUT_ASK        (XPAR_BRAM_0_BASEADDR + 264)
#define IO_ADDR_BRAM_OUT_ASK_SIZE   (XPAR_BRAM_0_BASEADDR + 266)
#define IO_ADDR_BRAM_OUT_DATA       (XPAR_BRAM_0_BASEADDR + 268)

// CMD define
#define CMD_SETLED                  0
#define CMD_SETOUTPUT               1
#define CMD_SETOUTPUTEX             2
#define CMD_SETANALOG1OUT           3
#define CMD_SETANALOG2OUT           4

// Analog function type define 
#define CLOSE_ANALOG		        0
#define SINE_ANALOG			        1
#define TRIANGE_ANALOG		        2
#define SAWTOOTH_ANALOG		        3

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

// math define
#define PI 							acos(-1)
#define UNIT_TIME 					1000000         // Unit: 1 microsecond
