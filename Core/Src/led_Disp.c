/*
这个代码用于LED在蓝桥杯在不同界面LED的不同值
P2作为锁存器高电平开启，低电平关闭
* 函数名：LED_Disp
 * 功  能：按位掩码控制 8 颗 LED 的亮灭
 * 参  数：mask —— bit0~bit7 分别对应 LD1~LD8，1 = 亮，0 = 灭
 *          0x01 → 只亮 LD1
 *          0x81 → LD1 和 LD8 同时亮
 *          0x00 → 全灭
 * 说  明：PC8~PC15 与 LCD 数据线复用，必须经 74HC573 锁存器（LE = PD2）
 *         中转，否则 LCD 一刷新就把 LED 的电平冲掉。
*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

void LED_Disp(uint8_t mask)
{
	uint16_t on_set=0;//拉低
	uint16_t off_set=0;//拉高
	for(uint8_t i=0;i<8;i++)
	{
		if(mask&(1u<<i))
		on_set |=(uint16_t)(0x0100u <<i);
		else
		off_set |=(uint16_t)(0x0100u <<i);			
	}
	if(on_set)HAL_GPIO_WritePin(GPIOC,on_set,GPIO_PIN_RESET);
	if(off_set)HAL_GPIO_WritePin(GPIOC,off_set,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
}
