#include "stm32f1xx.h"
#include "gpio.h"
#include "stm32f1xx_hal.h"

//#define ENCODER

#define  BIAS  16;
void moto(int modeR,int modeL);

int Velocity_A(int TargetVelocity, int CurrentVelocity);
int Velocity_B(int TargetVelocity, int CurrentVelocity);
 
void Motor(int R,int L,int TargetVelocityR,int TargetVelocityL);

