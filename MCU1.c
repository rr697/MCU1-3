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

static signed char I2CDataIn;
static int I2Cstate = 0;
static unsigned char I2C_request;
static int move_forward;
static int stop;
static int move_back;
static int move_left;
static int move_right; 
static int speed_A; 
static int speed_B; 
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
    //move_forward = 2250; 
    //stop = 0;

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
                case 1:
                    if(I2CDataIn> 0){
                        mPORTASetBits(BIT_2);
                        SetDCOC1PWM(16384-abs(I2CDataIn)*128); 
                    }
                    else 
                    {
                       mPORTAClearBits(BIT_2);  
                       SetDCOC1PWM(abs(I2CDataIn)*128); 
                     
                    }
                   
                case 2:
                    if(I2CDataIn > 0){
                    mPORTBSetBits(BIT_13);
                    SetDCOC2PWM(16384-abs(I2CDataIn)*128);  
                    }
                    else
                    {
                        mPORTBClearBits(BIT_13);
                        SetDCOC2PWM(abs(I2CDataIn)*128);
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
    //mPORTASetBits(BIT_4);
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

void init_inputCapture(void){
  OpenCapture1(  IC_EVERY_RISE_EDGE | IC_INT_1CAPTURE | IC_TIMER3_SRC | IC_ON );
  // turn on the interrupt so that every capture can be recorded
  ConfigIntCapture1(IC_INT_ON | IC_INT_PRIOR_3 | IC_INT_SUB_PRIOR_3 );
  INTClearFlag(INT_IC1);
  // connect PIN 24 to IC1 capture unit
  PPSInput(1, IC4, RPB4);
}
void __ISR(_INPUT_CAPTURE_4_VECTOR, ipl3SOFT) inputCapture(void)
{
    if(mPORTAReadBits(BIT_0)){
        mIC4ReadCapture();
        speed_A = -127/max(mIC4ReadCapture(),1);
        
    }
    else
    {
        
        speed_A = 127/max(mIC4ReadCapture(),1);
    }
}
// === Main  ======================================================


void main(void) {
   
      OpenTimer2(T2_ON|T2_SOURCE_INT|T2_PS_1_1,16384);
      OpenTimer3(T3_ON|T3_SOURCE_INT|T3_PS_1_64,0xffff);
   
      ConfigIntTimer2(T2_INT_OFF| T2_INT_PRIOR_6);
      mT2ClearIntFlag();
      //mPORTBSetPinsDigitalOut(BIT_7|BIT_5);
      mPORTASetPinsDigitalOut(BIT_0|BIT_2);
      mPORTBSetPinsDigitalOut(BIT_11|BIT_13);
      mPORTAClearBits(BIT_2);
      mPORTBClearBits(BIT_13);
   
      RPB11R =0x05;
      RPA0R = 0x05; 
      OpenOC1(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0 , 0);
      OpenOC2(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE, 0 , 0);
    SYSTEMConfigPerformance(sys_clock);
    // === I2C Init ================
    InitI2C();
    init_inputCapture();
 

    mJTAGPortEnable(0);

    INTEnableSystemMultiVectoredInt();
    
    mPORTASetPinsDigitalOut(BIT_0);
    mPORTAClearBits(BIT_0);
    
    
    //mPORTASetPinsDigitalOut(BIT_1);
    //mPORTASetBits(BIT_1);
    //round robin thread schedule
    while (1) {
        I2C1CONbits.SCLREL = 1; // release the clock

    }
} // main

// === end  ======================================================
