/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "lcd.h"
#include "i2c_hal.h"
#include "key.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t xms = 0;
volatile uint32_t last_tim_capA = 0;//A存上一次的上升沿
volatile uint32_t periodA_between = 0;//A存两次之间的周期

volatile uint32_t last_tim_capB = 0;//B存上一次的上升沿
volatile uint32_t periodB_between = 0;//B存两次之间的周期

volatile uint32_t tim_lastrecordA = 0;
volatile uint32_t tim_lastrecordB = 0;
//频率的变量
volatile uint32_t fA = 0;
volatile uint32_t fB = 0;
volatile uint32_t view_mode=0;//频率or周期

//按键的变量

volatile uint32_t key3_start=0;//按键3开始，清0用
volatile uint32_t Lcd_val=0;//这个是为了存lcd当前要显示的界面的值的，与key_scan的函数有关
volatile uint8_t Lcd_exchange=0;//界面切换

volatile uint32_t PD=1000;
volatile uint32_t PH=5000;
volatile int32_t PX=0;//它可以是负值，要有符号
volatile uint32_t NDA=0,NDB=0,NHA=0 , NHB=0;
volatile uint32_t lcd_lasttick=0;  //用来控制屏幕刷新度的，防止他刷新过快

volatile int32_t fA_cal=0;//A捕获在校验之后的值
volatile int32_t fB_cal=0;//B捕获在校验之后的值
volatile uint32_t over_A=0,over_B=0;//超出的值

volatile int32_t fA_max=0,fA_min=0,fB_max=0,fB_min=0;//检测最大最小值
volatile uint32_t fA_3swindow=0,fB_3swindow=0;//题目要求检测3s内的最大最小值

volatile uint8_t key_set=0;//这个是在加减时候的按键位置，题目默认PD=0
volatile uint8_t LED_mask=0;//LED亮的编号
char line_code_0 [21]={0};
char line_code_1 [21]={0};
char line_code_2 [21]={0};
char line_code_3 [21]={0};
char line_code_4 [21]={0};
char line_code_5 [21]={0};
char line_code_6 [21]={0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LED_Disp(uint8_t mask);

uint8_t Key_read(void);//读取按键
uint8_t Key_scan(void);//扫描按键
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	LCD_Init();
	HAL_TIM_Base_Start_IT(&htim6);
	
	HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_1);//用了中断就开他
	HAL_TIM_IC_Start_IT(&htim3,TIM_CHANNEL_1);//用了中断就开他
	
	LCD_SetTextColor(White);
	LCD_SetBackColor(Black);
	LCD_Clear(Black);
	sprintf(line_code_1,"        DATA ");//第一次没进循环，先让他显示
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(Lcd_val&0x08)//B4定义为界面切换
	  {
		Lcd_exchange++;
		if(Lcd_exchange>2)Lcd_exchange=0;
		  LCD_Clear(Black);
		if(Lcd_exchange==0) sprintf(line_code_1,"        DATA ");
		else if(Lcd_exchange==1) 
		{sprintf(line_code_1,"        PARA ");
			key_set=0;//当我按到第二个界面时，默认成PD
		}
		else sprintf(line_code_1,"        RECD");
		LCD_DisplayStringLine(Line1, (uint8_t*)line_code_1);
	  }
/********************采集代码A,不包含显示************************/
	  if((xms-tim_lastrecordA)>=100)
	  {
		  tim_lastrecordA=xms;
		  fA=(periodA_between>0)?(1000000u/periodA_between):0;
		  fA_cal=fA+PX;
		  
		  if(fA_cal>fA_max) fA_max=fA_cal;//这两行是窗口期A的最大最小比较
		  if(fA_cal<fA_min) fA_min=fA_cal;
		  
		  if(xms-fA_3swindow>=3000)
		  {
			if(fA_3swindow!=0&&fA_max-fA_min>=PD) NDA++;
				fA_max=fA_cal;//每次进来都更新一下，不然一直保持上一次的值
				fA_min=fA_cal;
				fA_3swindow=xms;//把3秒的时钟重新赋值进去，这些都是更新数据的必备操作
		  }
	  }
	  
/********************采集代码B,不包含显示************************/	  
	  if((xms-tim_lastrecordB)>=100)
	  {
		  tim_lastrecordB=xms;
		  fB=(periodB_between>0)?(1000000u/periodB_between):0;
		  fB_cal=fB+PX;
		  
		  if(fB_cal>fB_max) fB_max=fB_cal;//这两行是窗口期B的最大最小比较
		  if(fB_cal<fB_min) fB_min=fB_cal;
		  
		    if(xms-fB_3swindow>=3000)
		  {
			if(fB_3swindow!=0&&fB_max-fB_min>=PD) NDB++;
				fB_max=fB_cal;//每次进来都更新一下，不然一直保持上一次的值
				fB_min=fB_cal;
				fB_3swindow=xms;//把3秒的时钟重新赋值进去，这些都是更新数据的必备操作
		  }
	  }
		Lcd_val=Key_scan();//这行很重要，决定按键显示界面
/*****************************************************************/
	  if(xms-lcd_lasttick>=100)//100ms刷新一次，防止刷的太快死机了
	  {
		lcd_lasttick=xms;
		  if(Lcd_exchange==0)//当他什么都没有按下，也就是上电状态，显示的也只有DATA他们
		  {
			  if(view_mode==0)//频率模式
			  {
				  if(fA_cal<0)sprintf(line_code_3,"     A=NULL     ");//A的捕获
				else if(fA_cal>1000) 
				 sprintf(line_code_3,"     A=%ld.%02ldKHz     ",(long)fA_cal/1000,(long)fA_cal%1000/10);
				else sprintf(line_code_3,"     A=%ldHz      ",(long)fA_cal);
				
				
				 if(fB_cal<0)sprintf(line_code_4,"     B=NULL     ");//B的捕获
				else if(fB_cal>1000) 
				 sprintf(line_code_4,"     B=%ld.%02ldKHz     ",(long)fB_cal/1000,(long)fB_cal%1000/10);
				else sprintf(line_code_4,"     B=%ldHz      ",(long)fB_cal);
				
				 sprintf(line_code_5,"                   ");
				 sprintf(line_code_6,"                   ");
			  }
			  
			 else			//周期模式
			 {
				 if(fA_cal<0)sprintf(line_code_3,"     A=NULL     ");//A的捕获
				else if((1000000/fA_cal)>1000) 
				 sprintf(line_code_3,"     A=%ld.%02ldms     ",(long)(1000000/fA_cal)/1000,(long)(1000000/fA_cal)%1000/10);
				else sprintf(line_code_3,"     A=%ldus      ",(long)(1000000/fA_cal));
				
				
				 if(fB_cal<0)sprintf(line_code_4,"     B=NULL     ");//B的捕获
				else if((1000000/fB_cal)>1000) 
				 sprintf(line_code_4,"     B=%ld.%02ldms     ",(long)(1000000/fB_cal)/1000,(long)(1000000/fB_cal)%1000/10);
				else sprintf(line_code_4,"     B=%ldus      ",(long)(1000000/fB_cal));
				
				 sprintf(line_code_5,"                   ");
				 sprintf(line_code_6,"                   ");
			 }
		  }
		  
		  
		  else if(Lcd_exchange==1)//当切换一次，现在应该是参数界面
		  {
			sprintf(line_code_3,"     PD=%luHz      ",(unsigned long)PD);
			sprintf(line_code_4,"     PH=%luHz      ",(unsigned long)PH);
			sprintf(line_code_5,"     PX=%dHz      ",PX);
			sprintf(line_code_6,"                   ");
		  }
		  else
		  {
			sprintf(line_code_3,"     NDA=%lu        ",(unsigned long)NDA);
			sprintf(line_code_4,"     NDB=%lu        ",(unsigned long)NDB);
			sprintf(line_code_5,"     NHA=%lu        ",(unsigned long)NHA);
			sprintf(line_code_6,"     NHB=%lu        ",(unsigned long)NHB);
		  }
		LCD_DisplayStringLine(Line0, (uint8_t*)line_code_0);
		LCD_DisplayStringLine(Line1, (uint8_t*)line_code_1); 
		LCD_DisplayStringLine(Line2, (uint8_t*)line_code_2);
		LCD_DisplayStringLine(Line3, (uint8_t*)line_code_3);
		LCD_DisplayStringLine(Line4, (uint8_t*)line_code_4);
		LCD_DisplayStringLine(Line5, (uint8_t*)line_code_5);
		LCD_DisplayStringLine(Line6, (uint8_t*)line_code_6);
	  }
/********************按键部分的代码************************/
	  if(Lcd_val&0x04)//先按键，其实先后一样的
	  {
		  if(Lcd_exchange==1)//判断界面是不是我要的那个参数界面
		  {
			key_set++;
			  if(key_set>2)key_set=0;
		  }
		  if(Lcd_exchange==0)
		  {
			view_mode=~view_mode;
		  }
	  }
		  if(Lcd_val&0x01)//当按下1，+100
	  {
		  if(Lcd_exchange==1)
		  {
			if(key_set==0) {if(PD<1000)PD+=100;}//他们三个都是有限定范围的
			else if(key_set==1){if(PH<10000)PH+=100;}
			else	{if(PX<1000)PX+=100;}
		  }
	  }
	  if(Lcd_val&0x02)//当按下2，-100
	  {
		   if(Lcd_exchange==1)
		  {
			if(key_set==0) {if(PD>100)PD-=100;}//他们三个都是有限定范围的
			else if(key_set==1){if(PH>1000)PH-=100;}
			else	{if(-PX<1000)PX-=100;}
		  }
	  }
	  if((Lcd_val&0x04)&&(Lcd_exchange==2))//当按下3，全部清0
	  {
		  if(xms-key3_start>=1000)
		  {
			NDA=0;NDB=0;NHA=0;NHB=0;
			fA_max=fA_cal;fA_min=fA_cal;fA_3swindow=xms;
			fB_max=fB_cal;fB_min=fB_cal;fB_3swindow=xms;
			  key3_start=xms;
		  }
		
	  }
/********************LED部分的代码************************/	
	  LED_mask=0;//更新都要重新清0，或重新赋值
	  if(Lcd_exchange==0)LED_mask|=0x01;//LED在不同情况下显示的代码
	  if(fA_cal>PH)LED_mask|=0x02;
	  if(fB_cal>PH)LED_mask|=0x04;
	  if(NDA>=3|NDB>=3)LED_mask|=0x80;
	  LED_Disp(LED_mask);
	 
	  if(fA_cal>PH)//如果A的频率大于限定值，NHA+1,但只加一次
	  {
		if(over_A==0)
		{
			NHA++;
			over_A=1;
		}
	  }
	  else over_A=0;	//else别挂错地方了，不然会一直猛跳
	  if(fB_cal>PH)//如果B的频率大于限定值，NHB+1,但只加一次
	  {
		if(over_B==0)
		{
			NHB++;
			over_B=1;
		}
	  }
	  else over_B=0;
    /* USER CODE END WHILE */
	 
	
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM6)
	{
		xms++;
	}
} 
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)//A捕获
	{
		uint32_t now_capA=HAL_TIM_ReadCapturedValue(&htim2,TIM_CHANNEL_1);
		periodA_between = now_capA - last_tim_capA;
		last_tim_capA = now_capA;
	}
	if(htim->Instance==TIM3)//B捕获
	{
		uint32_t now_capB=HAL_TIM_ReadCapturedValue(&htim3,TIM_CHANNEL_1);
		periodB_between = now_capB - last_tim_capB;
		last_tim_capB = now_capB;
	}

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

