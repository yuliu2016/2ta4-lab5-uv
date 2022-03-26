/****** 



 1. both OD mode and PP mode can drive the motor! However, some pin can not output  high in OD mode!!! 
   (maybe because those pins have other alternate functions)). 

 2. the signals do not need to be inverted before feeded in H-bridge! 
*/

/*
Pins:
Button 1: PC1
Button 2: PD2
Button 3: PC3
Red: PC4
Gray: PC5
Yellow: PB2
Black: PB12
*/


#include "main.h"


#define COLUMN(x) ((x) * (((sFONT *)BSP_LCD_GetFont())->Width))    //see font.h, for defining LINE(X)


void LCD_DisplayString(uint16_t LineNumber, uint16_t ColumnNumber, uint8_t *ptr);
void LCD_DisplayInt(uint16_t LineNumber, uint16_t ColumnNumber, int Number);
void LCD_DisplayFloat(uint16_t LineNumber, uint16_t ColumnNumber, float Number, int DigitAfterDecimalPoint);


static void SystemClock_Config(void);
static void Error_Handler(void);

static void ExtBtn_Config();
static void MotorDriver_Config();
static void TIM3_Config();

TIM_HandleTypeDef Tim3_Handle;

static uint8_t direction = CW;
static uint8_t mode = FULLSTEP;
static uint16_t period = 61;
static uint16_t stepperState = 0;

static const uint8_t sequence[8][4] = {
	{0, 0, 1, 0},
	{0, 1, 1, 0},
	{0, 1, 0, 0},
	{0, 1, 0, 1},
	{0, 0, 0, 1},
	{1, 0, 0, 1},
	{1, 0, 0, 0},
	{1, 0, 1, 0}
};

static void StepperWrite(uint16_t state);
static void UpdateSettings();

int main(void){
	
		/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
		HAL_Init();
		
	
		 /* Configure the system clock to 180 MHz */
		SystemClock_Config();
		
		HAL_InitTick(0x0000); // set systick's priority to the highest.
	
		BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
		ExtBtn_Config();
		MotorDriver_Config();
		TIM3_Config();


		BSP_LCD_Init();
		//BSP_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address);
		BSP_LCD_LayerDefaultInit(0, LCD_FRAME_BUFFER);   //LCD_FRAME_BUFFER, defined as 0xD0000000 in _discovery_lcd.h
															// the LayerIndex may be 0 and 1. if is 2, then the LCD is dark.
		//BSP_LCD_SelectLayer(uint32_t LayerIndex);
		BSP_LCD_SelectLayer(0);
		//BSP_LCD_SetLayerVisible(0, ENABLE); //do not need this line.
		BSP_LCD_Clear(LCD_COLOR_WHITE);  //need this line, otherwise, the screen is dark	
		BSP_LCD_DisplayOn();
	 
		BSP_LCD_SetFont(&Font20);  //the default font,  LCD_DEFAULT_FONT, which is defined in _lcd.h, is Font24
	
	
		LCD_DisplayString(2, 3, (uint8_t *)"Lab");
	
		LCD_DisplayInt(2, 8, 5);

		UpdateSettings();

		// Start timer
		HAL_TIM_Base_Start_IT(&Tim3_Handle);

		int incremCount = 0;
		int decremCount = 0;
		
		while(1) {	
			int incremDown = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
			int decremDown = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);

			if (incremDown) {
				if (++incremCount >= 6) {
					period++;
					UpdateSettings();
				}
			} else {
				// reset the count
				incremCount = 0;
			}

			if (decremDown) {
				if (++decremCount >= 6) {
					if (period > 5) {
						period--;
						UpdateSettings();
					}
				}
			} else {
				// reset the count
				decremCount = 0;
			}
			HAL_Delay(100);

		} // end of while loop
	
}  //end of main


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Activate the Over-Drive mode */
  HAL_PWREx_EnableOverDrive();
 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}





void LCD_DisplayString(uint16_t LineNumber, uint16_t ColumnNumber, uint8_t *ptr)
{  
  //here the LineNumber and the ColumnNumber are NOT  pixel numbers!!!
		while (*ptr!=NULL)
    {
				BSP_LCD_DisplayChar(COLUMN(ColumnNumber),LINE(LineNumber), *ptr); //new version of this function need Xpos first. so COLUMN() is the first para.
				ColumnNumber++;
			 //to avoid wrapping on the same line and replacing chars 
				if ((ColumnNumber+1)*(((sFONT *)BSP_LCD_GetFont())->Width)>=BSP_LCD_GetXSize() ){
					ColumnNumber=0;
					LineNumber++;
				}
					
				ptr++;
		}
}

void LCD_DisplayInt(uint16_t LineNumber, uint16_t ColumnNumber, int Number)
{  
  //here the LineNumber and the ColumnNumber are NOT  pixel numbers!!!
		char lcd_buffer[15];
		sprintf(lcd_buffer,"%d",Number);
	
		LCD_DisplayString(LineNumber, ColumnNumber, (uint8_t *) lcd_buffer);
}

void LCD_DisplayFloat(uint16_t LineNumber, uint16_t ColumnNumber, float Number, int DigitAfterDecimalPoint)
{  
  //here the LineNumber and the ColumnNumber are NOT  pixel numbers!!!
		char lcd_buffer[15];
		
		sprintf(lcd_buffer,"%.*f",DigitAfterDecimalPoint, Number);  //6 digits after decimal point, this is also the default setting for Keil uVision 4.74 environment.
	
		LCD_DisplayString(LineNumber, ColumnNumber, (uint8_t *) lcd_buffer);
}


static void ExtBtn_Config() {
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clock Enable
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// PC1
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = GPIO_PIN_1;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_1);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	// PD2
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_2);

	HAL_NVIC_SetPriority(EXTI2_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);

	// PC3
	GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Pin = GPIO_PIN_3;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	__HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_3);

	HAL_NVIC_SetPriority(EXTI3_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

static void StepperWrite(uint16_t state) {
	uint8_t *pin_for_state = &sequence[state];
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, pin_for_state[0]);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, pin_for_state[1]);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, pin_for_state[2]);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, pin_for_state[3]);

	LCD_DisplayInt(11,3, pin_for_state[0]);
	LCD_DisplayInt(12,3, pin_for_state[1]);
	LCD_DisplayInt(13,3, pin_for_state[2]);
	LCD_DisplayInt(14,3, pin_for_state[3]);
}

static void MotorDriver_Config() {
	GPIO_InitTypeDef GPIO_InitStructure;

	// Clock Enable
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// PC4
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_4;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	// PC5
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	// PB2
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_2;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

	// PB12
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void TIM3_Config(void) {
	/*
	Prescaler calculation:
	APB1 clock has a frequency of SystemCoreClock / 2 (~90MHz)
	For stepper motor: (~90MHz)/(4800Hz) - 1 = ~18749
	*/
	Tim3_Handle.Instance = TIM3;
	Tim3_Handle.Init.Period = 6000; // temporary, changed as needed
	Tim3_Handle.Init.Prescaler = ((SystemCoreClock / 2) / 4800) - 1; // fixed
	Tim3_Handle.Init.ClockDivision = 0;
	Tim3_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_Base_Init(&Tim3_Handle);
}


static void UpdateSettings() {
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	LCD_DisplayString(5, 1, (uint8_t *) "direction");
	LCD_DisplayString(6, 1, (uint8_t *) "step mode");
	LCD_DisplayString(7, 1, (uint8_t *) "period(s)     ");

	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	LCD_DisplayString(5, 12, (uint8_t *) (direction == CW ? "CW " : "CCW"));
	LCD_DisplayString(6, 12, (uint8_t *) (mode == FULLSTEP ? "FULL" : "HALF"));
	LCD_DisplayInt(7, 12, period);

	// Calculate and update the ARR
	if (mode == FULLSTEP) {
		TIM3->ARR = 100 * period - 1;
	} else {
		TIM3->ARR = 50 * period - 1;
	}
	TIM3->EGR = TIM_EGR_UG;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
		if(GPIO_Pin == KEY_BUTTON_PIN)  //GPIO_PIN_0
		{
			direction = direction == CW ? CCW : CW;
			UpdateSettings();
		}
		
		
		if(GPIO_Pin == GPIO_PIN_1)
		{
			period++;
			UpdateSettings();
		}  //end of PIN_1

		if(GPIO_Pin == GPIO_PIN_2)
		{
			mode = mode == FULLSTEP ? HALFSTEP : FULLSTEP;
			UpdateSettings();
		} //end of if PIN_2	
		
		if(GPIO_Pin == GPIO_PIN_3)
		{
			if (period > 5) {
				period--;
				UpdateSettings();
			}
		} //end of if PIN_23
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)   //see  stm32fxx_hal_tim.c for different callback function names.
																															//for timer 3 , Timer 3 use update event initerrupt
{
	if (direction == CW) {
		if (mode == FULLSTEP && stepperState % 2 != 0) {
			stepperState += 2;
		} else {
			stepperState += 1;
		}
	} else {
		if (mode == FULLSTEP && stepperState % 2 != 0) {
			stepperState -= 2;
		} else {
			stepperState -= 1;
		}
	}
	stepperState %= 8; // wrap around
	StepperWrite(stepperState);
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef * htim) //see  stm32fxx_hal_tim.c for different callback function names. 
{																																//for timer4 
}
 
static void Error_Handler(void)
{
  /* Turn LED4 on */
  BSP_LED_On(LED4);
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
/**
  * @}
  */

/**
  * @}
  */



