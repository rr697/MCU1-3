#include "config.h"

#define I2CBAUD 100000 //PIC32 master clock operating at 10kHz (irrelevant if PIC32 is a slave)
#define BRG_VAL  ((PBCLK/2/I2CBAUD)-2)

//---I2C Register Addresses & associated bit masks---

#define I2CAddress 0x1C


