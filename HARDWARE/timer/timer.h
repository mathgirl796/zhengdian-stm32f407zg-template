#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#include <math.h>
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/6/16
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

void TIM2_Int_Init(u16 arr,u16 psc);
void TIM2_Int_Init_us(u32 us);
void TIM2_Int_Stop(void);

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Init_us(u32 us); // us 在65536以下是准确的，再高的话会被截断一点. 最多6553500
void TIM3_Int_Stop(void);

/////// TIM4被usmart占用


void TIM5_Int_Init(u16 arr,u16 psc);
void TIM5_Int_Init_us(u32 us);
void TIM5_Int_Stop(void);

#endif




