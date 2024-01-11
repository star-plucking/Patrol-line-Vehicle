#include "Light.h"
#include "motor.h"

// 读引脚电平
// 引脚对应巡线模块顺序：从左到右

// PIN|  A12  |  A11   |  A10  |  A9   |
// NO |   1   |   2    |   3   |   3   |

static int Count = 0;
static int Times = 0;
static int CircleMode = 0;
static int CircleTimes = 0;
static int ShaBiCount=0;

inline void ReadLight(uint8_t *LightPin)
{
	LightPin[0] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11);
	LightPin[1] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
	LightPin[2] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);
	LightPin[3] = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9);
}

/*-----------------------------------------
	- 三种开环方法，参数中的Delay为延时时间（次数）
	- bias用于积分法中的修正，其他方法暂时用不到
	- 目前MODE1最稳定
-------------------------------------------*/

// 利用if的开环巡线控制
void LineCtrl(int *TVR, int *TVL, int *R, int *L, int *Delay, int *bias)
{
	// 读电平
	uint8_t LightPin[4] = {0u, 0u, 0u, 0u};
	ReadLight(LightPin);

#ifdef MODE3

	/*-------------------------------------
		Version=3.0
		带偏置的转向模式
		完全使用DELAY函数的方法
		使用摇摆式的空白检测模式，12次摇摆后自动退出-> 容易发生转向，不稳定
	---------------------------------------*/

	// 左大弯  0 0 0 1
	//    0 0 1 1
	//    0 1 0 1
	if (LightPin[0] == 0u && LightPin[3] == 1u && !(LightPin[1] == 1u && LightPin[2] == 1u))
	{
		// *TVR=LargeCurve;*TVL=LargeCurve;*R=1;*L=0;
		Count = 0;
		Motor(0, 0, LargeCurve, LargeCurve);
		HAL_Delay(ForwardBias);
		Motor(1, 0, LargeCurve, LargeCurve);
		HAL_Delay(LongDelay);
		Motor(1, 1, LargeCurve, LargeCurve);
		HAL_Delay(LongDelay);
	}
	// 右大
	//  1 0 0 0
	//  1 1 0 0
	//  1 0 1 0
	else if (LightPin[3] == 0u && LightPin[0] == 1u && !(LightPin[1] == 1u && LightPin[2] == 1u))
	{
		// *TVR=LargeCurve;*TVL=LargeCurve;*R=0;*L=1;
		Motor(0, 0, LargeCurve, LargeCurve);
		HAL_Delay(ForwardBias);
		Motor(0, 1, LargeCurve, LargeCurve);
		HAL_Delay(LongDelay + LdelayBias);
		Motor(1, 1, LargeCurve, LargeCurve);
		HAL_Delay(LongDelay);
	}
	// 左中 0 1 1 1
	else if (LightPin[0] == 0u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 1u)
	{
		//  *TVR=LargeCurve/10*8;*TVL=0;*R=1;*L=1;
		Motor(1, 1, (LargeCurve + MidCurveBias) / 10 * 10, (LargeCurve + MidCurveBias) / 10 * 2);
		HAL_Delay(MidDelay);
		Count = 0;
	}
	// 右中
	else if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 0u)
	{
		// *TVR=0;*TVL=LargeCurve/10*8;*R=1;*L=1;
		Motor(1, 1, (LargeCurve + MidCurveBias) / 10 * 2, (LargeCurve + MidCurveBias) / 10 * 10);
		HAL_Delay(MidDelay);
		Count = 0;
	}
	// 左小 1 1 0 1
	else if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[2] == 0u && LightPin[3] == 1u)
	{
		// *TVR=0;*TVL=MidCurve;*R=1;*L=0;
		Motor(0, 1, MidCurve, 0);
		HAL_Delay(ShortDelay);
		*bias = 2;
		Count = 0;
	}
	// 右小
	else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 1u && LightPin[3] == 1u)
	{
		// *TVR=MidCurve;*TVL=0;*R=0;*L=1;
		Motor(1, 0, 0, MidCurve);
		HAL_Delay(ShortDelay);
		*bias = 1;
		Count = 0;
	}
	else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 1u)
	{
		// *TVR=FullSpeed;*TVL=FullSpeed;*R=1;*L=1;
		Motor(1, 1, FullSpeed, FullSpeed);
		Count = 0;
	}
	else
	{
		// *TVR=FullSpeed;*TVL=FullSpeed;*R=1;*L=1;
		/*
		if(*bias==1)
		{
			Motor(0,1,FullSpeed+80,0);
		  HAL_Delay(200);
			*bias=0;
			return;
		}
		else if(*bias==2)
		{
			Motor(1,0,0,FullSpeed+80);
		  HAL_Delay(200);
			*bias=0;
			return;
		}
		*/
		if (Count <= 12 && (Count % 2) == 0)
		{
			if (Count == 0)
			{
				Times = 0;
				Count = 2;
			}
			Motor(0, 1, FullSpeed + EmptyBias, FullSpeed + EmptyBias);
			HAL_Delay(1);

			if (Times == OUTDelay2 + (Count / 2) * 50)
			{
				Count++;
				Times = 0;
			}
			else
			{
				Times++;
			}

			return;
		}
		else if (Count <= 12 && (Count % 2) == 1)
		{
			Motor(1, 0, FullSpeed + EmptyBias, FullSpeed + EmptyBias);
			HAL_Delay(1);
			if (Times == OUTDelay2 + (Count / 2) * 50)
			{
				Count++;
				Times = 0;
			}
			else
			{
				Times++;
			}
			return;
		}
		else if (Count > 12)
		{
			Motor(1, 1, 0, 0);
		}
	}

#endif

#ifdef MODE2
	/*------------------------------------------
		Version = 2.0
		不带偏置的转向模式
		转向不合理
		容易跑到未知状态
	     2024/1/6 过于不稳定，此方法完全放弃
	--------------------------------------------*/
	if ((*Delay) == 0)
	{
		// 右大
		if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 0u)
		{
			*TVR = LargeCurve;
			*TVL = LargeCurve;
			*R = 0;
			*L = 1;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = LongDelay;
			return;
		} // 左大
		else if (LightPin[0] == 0u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 1u)
		{
			*TVR = LargeCurve;
			*TVL = LargeCurve;
			*R = 1;
			*L = 0;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = LongDelay;
			return;
		}
		// 右小
		else if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[2] == 0u && LightPin[3] == 1u)
		{
			*TVR = LargeCurve;
			*TVL = LargeCurve;
			*R = 0;
			*L = 1;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = ShortDelay;
			return;
		}
		else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 1u && LightPin[3] == 1u)
		{
			*TVR = LargeCurve;
			*TVL = LargeCurve;
			*R = 1;
			*L = 0;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = ShortDelay;
			return;
		}
		else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 1u)
		{
			*TVR = FullSpeed;
			*TVL = FullSpeed;
			*R = 1;
			*L = 1;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = 0;
			return;
		}
		else
		{
			*TVR = 0;
			*TVL = 0;
			*R = 1;
			*L = 1;
			// Motor(1,0,LargeCurve,LargeCurve);
			*Delay = 0;
		}
	}
	else
	{
		(*Delay)--;
	}
}
#endif

#ifdef MODE1
/*-----------------------------------------
	Version=1.2
	最稳定                                              2024/1/6
	大型角度时容易出错->采用while法和后退法一定程度解决 2024/1/7
	尝试更改大角度转弯逻辑来解决大角度转弯问题          2024/1/8
	速度不够理想
------------------------------------------*/

if (CircleMode == 1 || CircleMode == 2 || CircleMode == 3 || CircleMode == 4)
{
	// 转弯前退一段路，防止转向过度
	if (CircleMode == 1 || CircleMode == 2)
	{
		*TVR = LargeCurve + MotorBias;
		*TVL = LargeCurve + MotorBias;
		*R = 0;
		*L = 0;
		CircleTimes++;
		int maxCircleTimes;
		if (CircleMode == 1)
		{
			maxCircleTimes = 25000;
		}
		else if (CircleMode == 2)
		{
			maxCircleTimes = 25000;
		}
		if (CircleTimes >= maxCircleTimes) // 一个持续时间13.8ms的反向输出，起到类似于急停的作用
		{
			if (CircleMode == 1)
			{
				CircleMode = 3;
				*TVR = LargeCurve * 0;
				*TVL = LargeCurve;
				*R = 1;
				*L = 0;
				*Delay = 0;
			}
			else if (CircleMode == 2)
			{
				CircleMode = 4;
				*TVR = LargeCurve ;
				*TVL = LargeCurve * 0;
				*R = 0;
				*L = 1;
				*Delay = 0;
			}
			CircleTimes = 0;
			Count=0;
		}
		return;
	}
	CircleTimes++;
	Count++;
	if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[2] == 1u && LightPin[3] == 1u) // watch Dog
	{
		if (Count >= 120000)
		{
			*TVR = 0;
			*TVL = 0;
			*R = 0;
			*L = 0;
			*Delay = 0;
			Times = 0;
			CircleMode = 0;
			return;
		}
		return;
	} 
	// 备选方案，while型，直到读到 1 0 0 1 ,始终转动
	if ( CircleTimes >= 60000 && (( LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 1u) 
	     ||(LightPin[0] == 0u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 1u && CircleMode == 4) 
	     ||(LightPin[0] == 1u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 0u && CircleMode == 3)))
	{
		CircleMode = 0;
		*TVR = FullSpeed;
		*TVL = FullSpeed;
		*R = 1;
		*L = 1;
		*Delay = 0;
		Count = 0;
		CircleTimes = 0;
	}
}

if ((LightPin[0] == 0u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 0u) ||
	(LightPin[0] == 1u && LightPin[1] == 1u && LightPin[2] == 1u && LightPin[3] == 1u))
{ // 相当于跑到赛道外直接刹住
	if (Count >= 100000)
	{
		*TVR = 0;
		*TVL = 0;
		*R = 0;
		*L = 0;
		*Delay = 0;
		Times = 0;
		return;
	}
	Count++;
	return;
}

if (*Delay == 0)
{
	// 左大弯  0 0 0 1
	//        0 0 1 1
	//        0 1 0 1

	//   0 x 0 0
	if ( (LightPin[0] == 0u && LightPin[3] == 1u && !(LightPin[1] == 1u && LightPin[2] == 1u)))
	{
		*TVR = LargeCurve;
		*TVL = LargeCurve;
		*R = 1;
		*L = 0;
		(*bias) += LargeIntegral;
		CircleMode = 1;
		// Motor(1,0,LargeCurve,LargeCurve);
		*Delay = LongDelay;
		Count = 0;
		CircleTimes = 0;
	}
	// 右大
	//  0 0 x 0
	else if ((LightPin[3] == 0u && LightPin[0] == 1u && !(LightPin[1] == 1u && LightPin[2] == 1u)))
	{
		for (int i = 100000; i > 0; i--)
			;
		*TVR = LargeCurve;
		*TVL = LargeCurve;
		*R = 0;
		*L = 1;
		// Motor(0,1,LargeCurve,LargeCurve);
		CircleMode = 2;
		(*bias) -= LargeIntegral;
		(*bias) -= LargeIntegral;
		*Delay = LongDelay;
		Count = 0;
		CircleTimes = 0;
	}
	// 左中 0 1 1 1
	else if (LightPin[0] == 0u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 1u)
	{
		*TVR = LargeCurve / 10 * 9;
		*TVL = LargeCurve / 10 * 2;
		*R = 1;
		*L = 1;
		// Motor(1,0,LargeCurve,LargeCurve);
		(*bias) += MidIntegral;
		*Delay = MidDelay;
		Count = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
	// 右中
	else if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[1] == 1u && LightPin[3] == 0u)
	{
		*TVR = LargeCurve / 10 * 2;
		*TVL = LargeCurve / 10 * 9;
		*R = 1;
		*L = 1;
		// Motor(0,1,LargeCurve,LargeCurve);
		(*bias) -= MidIntegral;
		*Delay = MidDelay;
		Count = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
	// 左小 1 1 0 1
	else if (LightPin[0] == 1u && LightPin[1] == 1u && LightPin[2] == 0u && LightPin[3] == 1u)
	{
		*TVR = MidCurve;
		*TVL = 0;
		*R = 0;
		*L = 1;

		(*bias) += SmallIntegral; // 右偏2
		// Motor(1,0,MidCurve,0);
		*Delay = ShortDelay;
		Count = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
	// 右小
	else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 1u && LightPin[3] == 1u)
	{
		*TVR = 0;
		*TVL = MidCurve;
		*R = 1;
		*L = 0;
		(*bias) -= SmallIntegral; // 左偏移1
		// Motor(0,1,0,MidCurve);
		*Delay = ShortDelay;
		Count = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
	else if (LightPin[0] == 1u && LightPin[1] == 0u && LightPin[2] == 0u && LightPin[3] == 1u)
	{
		*TVR = FullSpeed;
		*TVL = FullSpeed;
		*R = 1;
		*L = 1;
		// Motor(1,1,FullSpeed,FullSpeed);
		*Delay = 0;
		Times++;
		Count = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
	else
	{
		*TVR = FullSpeed;
		*TVL = FullSpeed;
		*R = 1;
		*L = 1;
		// Motor(1,1,FullSpeed,FullSpeed);
		*Delay = 0;
			if(ShaBiCount>0){
		ShaBiCount--;
	}
	}
}
else
{
	(*Delay)--;
	if(ShaBiCount>0){
		ShaBiCount--;
	}
}
#endif
}

/*后续应该还可以加一个类似于记忆的功能，记录偏离方向，这样如果是全白的状态依然可以回去*/
