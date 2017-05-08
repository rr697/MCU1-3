#include "config.h"
#include <plib.h>
#include "I2C_Reg_defs.h"
#include "ptThread.h"

static int LF_dir;
static int LM_dir;
static int LB_dir;
static int LF_reverse;
static int LM_reverse;
static int LB_reverse;
static int last_last_LB_effort;
static int last_last_LM_effort;
static int last_last_LF_effort;
static int last_LF_effort;
static int last_LM_effort;
static int last_LB_effort;
#define MIN_STOP_SPEED 2100
#define MAX_STOP_SPEED 2500

static PT_THREAD(protothread_Anim(struct pt *pt)){
    PT_BEGIN(pt);
    while(1){
       
        
        PT_YIELD_TIME_msec(100);
    }
    
    PT_END(pt);
}


static PT_THREAD(protothread_pid(struct pt *pt)) {
    PT_BEGIN(pt);
    while (1) {

        if (Status1 & F_ENC_STAT) {
            
            if(LF_effort > MIN_STOP_SPEED && LF_effort < MAX_STOP_SPEED){
                if(last_LF_effort != LF_effort){//check for divide by zero condition
                    LF_dir = (LF_Speed- LF_lastSpeed)/(LF_effort - last_LF_effort) ;//+((RB_Speed- RB_lastSpeed)/(RB_effort - last_RB_effort)- (RB_lastSpeed- RB_last_last_speed)/(last_RB_effort - last_last_RB_effort))/(RB_effort-last_last_RB_effort);// ds/de gives direction information    
                }
            }
            else if (LF_effort <= MIN_STOP_SPEED){//effort very small, speed is certainly negative
                LF_dir = -1;
            }
            else if (LF_effort >= MAX_STOP_SPEED){//effort very large, positive speed
                LF_dir = 1;
            }
            else{
                LF_dir = LF_reverse;
            }
            
            if(LF_dir < 0) {
                if(LF_Speed > 0){
                    LF_Speed *= -1;
                }
                LF_reverse = -1;//save direction
            }
            else if(LF_dir == 0 && (LF_reverse==-1)){
                if(LF_Speed>0){
                    LF_Speed *= -1;
                }//in special case when speed did not change, latch last direction
            }
            else{
                if(LF_Speed < 0){
                    LF_Speed *= -1; 
                }
                LF_reverse = 1; //save direction
            }            
            
            
            
            LF_error = LF_PWM - LF_Speed; // get error from difference

            if (abs(LF_error) > IntThresh) {
                LF_integral += LF_error;
            } else {
                LF_integral *= 0.9;
            }
            if (LF_integral > 400) {
                LF_integral = 400;
            } else if (LF_integral < -400) {
                LF_integral = -400;
            }
            LF_effort += ((LF_error * LF_P)/ 16.0) +((LF_integral * LF_I)/64.0) + (((LF_lastSpeed - LF_Speed) * LF_D)/32.0);
            
            if(LF_effort > 3999){
                LF_effort = 3999;
            }
            else if(LF_effort < 1){
                LF_effort = 1;
            }
            
        } 


        if (Status1 & M_ENC_STAT && config1 & SIX_WHEEL_EN) {
            if(LM_effort > MIN_STOP_SPEED && LM_effort < MAX_STOP_SPEED){
                if(last_LM_effort != LM_effort ){//check for divide by zero condition
                    LM_dir = (LM_Speed- LM_lastSpeed)/(LM_effort - last_LM_effort) ;// ds/de gives direction information    
                }
            }
            else if (LM_effort <= MIN_STOP_SPEED){//effort very small, speed is certainly negative
                LM_dir = -1;
            }
            else if (LM_effort >= MAX_STOP_SPEED){//effort very large, positive speed
                LM_dir = 1;
            }
            else{
                LM_dir = LM_reverse;
            }
            
            if(LM_dir < 0) {
                if(LM_Speed > 0){
                    LM_Speed *= -1;
                }
                LM_reverse = -1;//save direction
            }
            else if(LM_dir == 0 && (LM_reverse==-1)){
                if(LM_Speed>0){
                    LM_Speed *= -1;
                }//in special case when speed did not change, latch last direction
            }
            else{
                if(LM_Speed < 0){
                    LM_Speed *= -1; 
                }
                LM_reverse = 1; //save direction
            }            
            
            
            
            LM_error = LM_PWM - LM_Speed; // get error from difference

            if (abs(LM_error) > IntThresh) {
                LM_integral += LM_error;
            } else {
                LM_integral *= 0.9;
            }
            if (LM_integral > 400) {
                LM_integral = 400;
            } else if (LM_integral < -400) {
                LM_integral = -400;
            }
            LM_effort += ((LM_error * LM_P)/16.0) +((LM_integral * LM_I)/64.0) + (((LM_lastSpeed - LM_Speed) * LM_D)/32.0);
            
            if(LM_effort > 3999){
                LM_effort = 3999;
            }
            else if(LM_effort < 1){
                LM_effort = 1;
            }
            
        } 

        if (Status1 & B_ENC_STAT) {
            
            if(LB_effort > MIN_STOP_SPEED && LB_effort < MAX_STOP_SPEED){
                if(last_LB_effort != LB_effort){//check for divide by zero condition
                    LB_dir = (LB_Speed- LB_lastSpeed)/(LB_effort - last_LB_effort) ;//+((RB_Speed- RB_lastSpeed)/(RB_effort - last_RB_effort)- (RB_lastSpeed- RB_last_last_speed)/(last_RB_effort - last_last_RB_effort))/(RB_effort-last_last_RB_effort);// ds/de gives direction information    
                }
            }
            else if (LB_effort <= MIN_STOP_SPEED){//effort very small, speed is certainly negative
                LB_dir = -1;
            }
            else if (LB_effort >= MAX_STOP_SPEED){//effort very large, positive speed
                LB_dir = 1;
            }
            else{
                LB_dir = LB_reverse;
            }
            
            if(LB_dir < 0) {
                if(LB_Speed > 0){
                    LB_Speed *= -1;
                }
                LB_reverse = -1;//save direction
            }
            else if(LB_dir == 0 && (LB_reverse==-1)){
                if(LB_Speed>0){
                    LB_Speed *= -1;
                }//in special case when speed did not change, latch last direction
            }
            else{
                if(LB_Speed < 0){
                    LB_Speed *= -1; 
                }
                LB_reverse = 1; //save direction
            }            
            LB_speed_out = LB_Speed; 
            
            
            LB_error = LB_PWM - LB_Speed; // get error from difference

            if (abs(LB_error) > IntThresh) {
                LB_integral += LB_error;
            } else {
                LB_integral *= 0.9;
            }
            if (LB_integral > 400) {
                LB_integral = 400;
            } else if (LB_integral < -400) {
                LB_integral = -400;
            }
            LB_effort += ((LB_error * LB_P)/16.0) +((LB_integral * LB_I)/64.0) + (((LB_lastSpeed - LB_Speed) * LB_D)/32.0);
            
            if(LB_effort > 3999){
                LB_effort = 3999;
            }
            else if(LB_effort < 1){
                LB_effort = 1;
            }
            
        } 

        
        LM_lastSpeed = LM_Speed;
        LB_lastSpeed = LB_Speed;
        LF_lastSpeed = LF_Speed;
        last_LB_effort = LB_effort;
        last_LM_effort = LM_effort;
        last_LF_effort = LF_effort;
        SetDCOC4PWM(LF_effort);
        SetDCOC1PWM(LM_effort);
        SetDCOC5PWM(LB_effort);
        PT_YIELD_TIME_msec(20);
    }
    PT_END(pt);
}