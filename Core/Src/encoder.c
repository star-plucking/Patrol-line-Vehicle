#include "encoder.h"

int Read_Encoder(uint8_t TIMX)
{
   int Encoder_TIM;    
   switch(TIMX)
	 {
	   case 2:  Encoder_TIM= (short)TIM2 -> CNT;  TIM2 -> CNT=0;break;	
	   case 4:  Encoder_TIM= (short)TIM4 -> CNT;  TIM4 -> CNT=0;break;	
	   default: Encoder_TIM=0;
	 }
	 
//	 if(Encoder_TIM>0xefff)Encoder_TIM=Encoder_TIM-0xffff; //转化计数值为有方向的值，大于0正转，小于0反转。
//	 if(Encoder_TIM<0)  Encoder_TIM=-Encoder_TIM;
		return Encoder_TIM;
}




