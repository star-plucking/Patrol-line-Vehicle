#include "motor.h"
#include "encoder.h"

extern float Velcity_Kp, Velcity_Ki, Velcity_Kd;
extern TIM_HandleTypeDef htim3;
void moto(int modeR, int modeL)
{
	// derection  modeR
	//  forward     1
	//  backward    0
	if (modeR == 1) // forward
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);	  // PA2 0
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); // PA3 1
	}
	else if (modeR == 0) // backward
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET); // PA2 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);	  // PA3 0
	}

	// derection  modeL
	//  forward     1
	//  backward    0
	if (modeL == 1) // forward
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);	  // PA4 0
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // PA5 1
	}
	else if (modeL == 0) // backward
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // PA10 1
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);	  // PA11 0
	}
}

// 电机控制程序（暂定）
void Motor(int R, int L, int TargetVelocityR, int TargetVelocityL)
{

	// Derection Contrl
	moto(R, L);

	int Velocity_PWM1 = 0, Velocity_PWM2 = 0;

#ifdef ENCODER
	int encoder_A, encoder_B;                              // 满意了吧
	encoder_B=-Read_Encoder(2);		                       // 读取编码器数值
    encoder_A=-Read_Encoder(4);                            // 读取编码器数值
    Velocity_PWM2=Velocity_A(TargetVelocityR,encoder_A);   // 算PWM
	Velocity_PWM1=Velocity_B(TargetVelocityL,encoder_B);
#endif

#ifndef ENCODER
	Velocity_PWM2 = TargetVelocityR + BIAS ;
	Velocity_PWM1 = TargetVelocityL ;
#endif

	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, Velocity_PWM1); // TIM3CH1  PA6  L
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, Velocity_PWM2); // TIM3CH2  PA7  R
}

// 以下是例程提供的电机闭环控制

int Velocity_A(int TargetVelocity, int CurrentVelocity)
{
	int Bias;							   // 定义相关变量
	static int ControlVelocity, Last_bias; // 静态变量，函数调用结束后其值依然存在

	Bias = TargetVelocity - CurrentVelocity; // 求速度偏差

	ControlVelocity += Velcity_Kp * (Bias - Last_bias) + Velcity_Ki * Bias; // 增量式PI控制器
																			// Velcity_Kp*(Bias-Last_bias) 作用为限制加速度
																			// Velcity_Ki*Bias             速度控制值由Bias不断积分得到 偏差越大加速度越大
	Last_bias = Bias;
	return ControlVelocity; // 返回速度控制值
}

int Velocity_B(int TargetVelocity, int CurrentVelocity)
{
	int Bias;							   // 定义相关变量
	static int ControlVelocity, Last_bias; // 静态变量，函数调用结束后其值依然存在

	Bias = TargetVelocity - CurrentVelocity; // 求速度偏差

	ControlVelocity += Velcity_Kp * (Bias - Last_bias) + Velcity_Ki * Bias; // 增量式PI控制器
																			// Velcity_Kp*(Bias-Last_bias) 作用为限制加速度
																			// Velcity_Ki*Bias             速度控制值由Bias不断积分得到 偏差越大加速度越大
	Last_bias = Bias;
	return ControlVelocity; // 返回速度控制值
}
