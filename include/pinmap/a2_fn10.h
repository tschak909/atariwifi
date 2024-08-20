/* FujiNet Hardware Pin Mapping */
#ifdef PINMAP_A2_FN10

#include "common.h"

/* IWM Bus Pins */
//      SP BUS                  GPIO                 SIO             LA (with SIO-10IDC cable)
//      ---------               -----------          -------------   -------------------------
#define SP_WRPROT               GPIO_NUM_27
#define SP_PHI0                 GPIO_NUM_39      //  CMD       7     D4
#define SP_PHI1                 GPIO_NUM_22      //  PROC      9     D6
#define SP_PHI2                 GPIO_NUM_36      //  MOTOR     8     D5
#define SP_PHI3                 GPIO_NUM_26      //  INT       13    D7
#define SP_RDDATA               GPIO_NUM_21      //  DATAIN    3     D2
#define SP_WRDATA               GPIO_NUM_33      //  DATAOUT   5     D3
#define SP_ENABLE               GPIO_NUM_32      //  CLKOUT    2     D1
#define SP_EXTRA                GPIO_NUM_32      //  CLKOUT - used for debug/diagnosing - signals when WRDATA is sampled by ESP32

#define SP_REQ                  SP_PHI0
#define SP_ACK                  SP_WRPROT

#endif /* PINMAP_A2_FN10 */
