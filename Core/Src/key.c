#include "main.h"
#include "tim.h"
#include "gpio.h"
#include "key.h"

volatile uint8_t key_nowval = 0;//现在读取到的按键读数
volatile uint8_t key_oldval = 0;//上一次读取到的按键读数
volatile uint32_t key_lasttick = 0;//上次采样的时间
volatile uint32_t key_cnt = 0;//按键次数
volatile uint32_t key_start=0;//按键开始

uint8_t Key_read(void)
{
	 uint8_t temp=0;
	
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0)==GPIO_PIN_RESET)  temp|=0x01;
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1)==GPIO_PIN_RESET)  temp|=0x02;
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_2)==GPIO_PIN_RESET)  temp|=0x04;
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==GPIO_PIN_RESET)  temp|=0x08;
	
	return temp;
}
uint8_t Key_scan(void)//扫描按键函数,这个不要在主循环中调用，函数里面我写了return 0
	//每次都会调用回0，导致按键不准，建议写一个变量存一下，我这里写的是Lcd_val
{
	uint8_t keyup=0;
	uint8_t change=0;
	uint8_t keydown=0;
	
	if(xms-key_lasttick<=20) return 0;//20ms进一次
	key_lasttick = xms; 
	
	key_nowval=Key_read();//我把读取到的按键赋给nowval
	
	change=key_nowval^key_oldval;//亦或门，只研究谁变了0x0011
	keyup=(uint8_t)~key_nowval&change;//这行很重要，~表示谁松开了，最后&是为了找出松开位置
	//uint8可加可不加，可以只算这8位
	
	keydown=key_nowval&change;//表示谁按下了
	//接下来的代码是 刚按下的键”转换为“长按计时的起点
	if(keydown&0x01)
	{
		key_start=xms;
	}
	if(keydown&0x04)//按键三
	{
		key3_start=xms;
	}
	key_oldval=key_nowval;
	
	return keyup;
}
