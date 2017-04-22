/*
 * File:        MCU1
 * Author:      Matthew Filipek
 * Target PIC:  PIC32MX250F128B
 */



////////////////////////////////////
#include "config.h"
#include <plib.h>
#include "I2C_Reg_defs.h"
////////////////////////////////////

static unsigned char I2CDataIn;
static int I2Cstate = 0;
static unsigned char I2C_request;
///////////////////////////////////////////////////////////////////
//
// Slave I2C interrupt handler
// This handler is called when a qualifying I2C events occurs
// this means that as well as Slave events
// Master and Bus Collision events will also trigger this handler.
//
///////////////////////////////////////////////////////////////////
static unsigned char WDTCount = 0;
void __ISR(_I2C_1_VECTOR, ipl3SOFT) _SlaveI2CHandler(void) {
    unsigned char temp;

    // check for MASTER and Bus events and respond accordingly
    if (IFS1bits.I2C1MIF) {
        mI2C1MClearIntFlag();
        return;
    }
    if (IFS1bits.I2C1BIF) {
        I2Cstate = 0;
        mI2C1BClearIntFlag();
        return;
    }

    // handle the incoming message
    if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0)) {
        // reset any state variables needed by a message sequence
        // perform a dummy read
        temp = SlaveReadI2C1();
        I2C1CONbits.SCLREL = 1; // release the clock
        I2Cstate = 0;
    } else if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1)) {//data received, input to slave        
        // writing data to our module
        I2CDataIn = SlaveReadI2C1();
        I2C1CONbits.SCLREL = 1; // release clock stretch bit
        
        if( I2Cstate == 0 ){
            I2Cstate = 1;
            I2C_request = I2CDataIn;
        }
        else if(I2Cstate == 1){
            switch(I2C_request){
                case 0x80:
                    if(I2CDataIn > 0x80){
                        mPORTASetBits(BIT_0);
                    }
                    else{
                        mPORTAClearBits(BIT_0);
                    }
                break;
            } 
            I2Cstate = 0;
        }
        
        
    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0)) {
        // read of the slave device, read the address
        temp = SlaveReadI2C1();
        if (I2Cstate == 1) {
            I2Cstate = 0;
            switch (I2C_request) {
                default:
                    SlaveWriteI2C1(I2C_request);//arbitrary data
                    break;
            }
        }
        else{
            SlaveWriteI2C1(0x5F);//arbitrary data
        }
    } else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1)) {
        // output the data until the MASTER terminates the
        // transfer with a NACK, continuing reads return 0
        switch (I2C_request) {
            default:
                SlaveWriteI2C1(0x7F);//arbitrary data. Put long transmission data here
                break;
        }

    }

    mI2C1SClearIntFlag();
}

void InitI2C(void) {

    CloseI2C1();
    
    // Enable the I2C module with clock stretching enabled
    // define a Master BRG of 400kHz although this is not used by the slave
    OpenI2C1(I2C_SLW_DIS | I2C_ON | I2C_7BIT_ADD | I2C_STR_DIS|I2C_SM_EN, BRG_VAL); //48);

    // set the address of the slave module
    I2C1ADD = I2CAddress;
    I2C1MSK = 0;
    // configure the interrupt priority for the I2C peripheral
    mI2C1SetIntPriority(I2C_INT_PRI_3 | I2C_INT_SLAVE);

    // clear pending interrupts and enable I2C interrupts
    mI2C1SClearIntFlag();
    EnableIntSI2C1;
}


// === Main  ======================================================


void main(void) {


    SYSTEMConfigPerformance(sys_clock);
    // === I2C Init ================
    InitI2C();

    mJTAGPortEnable(0);

    INTEnableSystemMultiVectoredInt();
    
    mPORTASetPinsDigitalOut(BIT_0);
    mPORTAClearBits(BIT_0);
    //round robin thread schedule
    while (1) {
        I2C1CONbits.SCLREL = 1; // release the clock

    }
} // main

// === end  ======================================================