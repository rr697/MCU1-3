#include "config.h"
#include <plib.h>
#include "I2C_Reg_defs.h"


static unsigned char I2CDataIn;
static int I2Cstate;
static unsigned char I2C_request;

void motorM(void){
    while(1){
        switch(I2C_request){
                case 0x80:
                    if(I2CDataIn == 0x00){
                        //mPORTASetBits(BIT_079);
                        SetDCOC1PWM(8192); 
                        SetDCOC2PWM(8192); 
                         
                     
                    }
                    else if (I2CDataIn == 0x01){
                        //mPORTAClearBits(BIT_0);
                        SetDCOC1PWM(0); 
                        SetDCOC2PWM(0); 
                    }
                    else {
                        //mPORTAClearBits(BIT_0);
                    }
                   
                case 0x81:
                    if(I2CDataIn == 0x10){
                    mPORTASetBits(BIT_0);
                    SetDCOC1PWM(0); 
                    SetDCOC2PWM(0); 
                    }
                break;
        }
    }
}