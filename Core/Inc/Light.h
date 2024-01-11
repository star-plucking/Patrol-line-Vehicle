#include "stm32f1xx.h"
#include "gpio.h"
#include "stm32f1xx_hal.h"

#define MODE1
// #define MODE2
//#define MODE3

//--------------------------------------------------------
#ifdef MODE1

#define LargeCurve 330     
#define MidCurve   300
#define FullSpeed  220     

//实际延时=Delayt * 0.0138 s

#define LongDelay  17000 // 30000*0.0138ms=414ms
#define MidDelay   500  // 9000*0.0138ms=124ms
#define ShortDelay 5000     // 0*0.0138ms=0ms

#define MotorBias -20

#define LargeIntegral 1
#define MidIntegral   3
#define SmallIntegral 1

#endif

//--------------------------------------------------------

#ifdef MODE2
#define LargeCurve 300 
#define MidCurve   280
#define FullSpeed  240

#define LongDelay  30  // 30*0.0138ms=0.414ms
#define MidDelay   15  // 15*0.0138ms=0.207ms
#define ShortDelay 0

#define LdelayBias 30
#define ForwardBias 40
#define MotorBias 10

#endif

//-----------------------------------------------------------

#ifdef MODE3

#define LargeCurve   180
#define MidCurve     150 //150
#define FullSpeed    240
#define MidCurveBias 50

//直接延时,Delayt ms
#define LongDelay  675  // 650
#define MidDelay   300  //200
#define ShortDelay 90   // 90

#define LdelayBias  30
#define ForwardBias 75
#define MotorBias   10  

#define EmptyBias -1
#define OUTDelay1 200-90
#define OUTDelay2 200-90
#endif

//--------------------------------------------------------------

void ReadLight(uint8_t *);
void LineCtrl(int *TVR, int *TVL, int *R, int *L, int *Delay, int *bias);



