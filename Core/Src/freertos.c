/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD_interface.h"
#include "queue.h"
#include "event_groups.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	FAN_MAX_SPEED,
	FAN_HAIF_SPEED,
	FAN_LOW_SPEED
}fanSpeed_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t correctPasword[4]={1,1,1,1};
uint8_t enteredPassword[4]={0};
extern ADC_HandleTypeDef hadc1;

/* USER CODE END Variables */
/* Definitions for KeypadTask */
osThreadId_t KeypadTaskHandle;
const osThreadAttr_t KeypadTask_attributes = {
  .name = "KeypadTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow1,
};
/* Definitions for Password */
osThreadId_t PasswordHandle;
const osThreadAttr_t Password_attributes = {
  .name = "Password",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TempTask */
osThreadId_t TempTaskHandle;
const osThreadAttr_t TempTask_attributes = {
  .name = "TempTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for lightTask */
osThreadId_t lightTaskHandle;
const osThreadAttr_t lightTask_attributes = {
  .name = "lightTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Window */
osThreadId_t WindowHandle;
const osThreadAttr_t Window_attributes = {
  .name = "Window",
  .stack_size = 64 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for Passwors */
osMessageQueueId_t PassworsHandle;
const osMessageQueueAttr_t Passwors_attributes = {
  .name = "Passwors"
};
/* Definitions for TempQueue */
osMessageQueueId_t TempQueueHandle;
const osMessageQueueAttr_t TempQueue_attributes = {
  .name = "TempQueue"
};
/* Definitions for GasQueue */
osMessageQueueId_t GasQueueHandle;
const osMessageQueueAttr_t GasQueue_attributes = {
  .name = "GasQueue"
};
/* Definitions for LCD_Semaphoer */
osSemaphoreId_t LCD_SemaphoerHandle;
const osSemaphoreAttr_t LCD_Semaphoer_attributes = {
  .name = "LCD_Semaphoer"
};
/* Definitions for window */
osEventFlagsId_t windowHandle;
const osEventFlagsAttr_t window_attributes = {
  .name = "window"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void ADC_Start_IT(uint8_t channel);
uint8_t CheckPassword(uint8_t* ptAPassword);
/* USER CODE END FunctionPrototypes */

void Keypad_Task(void *argument);
void Password_Task(void *argument);
void Temperature_Task(void *argument);
void Light_Task(void *argument);
void Window_Task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of LCD_Semaphoer */
  LCD_SemaphoerHandle = osSemaphoreNew(1, 1, &LCD_Semaphoer_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Passwors */
  PassworsHandle = osMessageQueueNew (4, sizeof(uint8_t), &Passwors_attributes);

  /* creation of TempQueue */
  TempQueueHandle = osMessageQueueNew (3, sizeof(uint32_t), &TempQueue_attributes);

  /* creation of GasQueue */
  GasQueueHandle = osMessageQueueNew (16, sizeof(uint32_t), &GasQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of KeypadTask */
  KeypadTaskHandle = osThreadNew(Keypad_Task, NULL, &KeypadTask_attributes);

  /* creation of Password */
  PasswordHandle = osThreadNew(Password_Task, NULL, &Password_attributes);

  /* creation of TempTask */
  TempTaskHandle = osThreadNew(Temperature_Task, NULL, &TempTask_attributes);

  /* creation of lightTask */
  lightTaskHandle = osThreadNew(Light_Task, NULL, &lightTask_attributes);

  /* creation of Window */
  WindowHandle = osThreadNew(Window_Task, NULL, &Window_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of window */
  windowHandle = osEventFlagsNew(&window_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_Keypad_Task */
/**
  * @brief  Function implementing the KeypadTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_Keypad_Task */
void Keypad_Task(void *argument)
{
  /* USER CODE BEGIN Keypad_Task */
	uint8_t IncremmentButton=0;
	uint8_t DecremmentButton=0;
	uint8_t EnterButton=0;
	uint8_t DigitNumberPass=0;
	uint8_t ConfirmDigit=99;
  /* Infinite loop */
  for(;;)
  {
		//vTaskSuspend(PasswordHandle);
		//vTaskSuspend(KeypadTaskHandle);
	  IncremmentButton = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
	  DecremmentButton =  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
	  EnterButton = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
	  if(IncremmentButton == 1)
	  {
		  if(DigitNumberPass<9)
		  {
			  DigitNumberPass++;
		  }
		  else if(DigitNumberPass == 9)
		  {
			  DigitNumberPass = 0;
		  }
		  xQueueSendToFront(PassworsHandle,(void*)&DigitNumberPass,portMAX_DELAY);
	  }
	  else if(DecremmentButton == 1)
	  {
		  if(DigitNumberPass == 0)
		  {
			  DigitNumberPass = 9;
		  }
		  else{
			  DigitNumberPass--;
		  }
		  xQueueSendToFront(PassworsHandle,(void*)&DigitNumberPass,portMAX_DELAY);

	  }
	  if(EnterButton == 1)
	  {
		  xQueueSendToFront(PassworsHandle,(void*)&ConfirmDigit,portMAX_DELAY);
	  }
	  HAL_Delay(150);
	  //osDelay(1);
  }
  /* USER CODE END Keypad_Task */
}

/* USER CODE BEGIN Header_Password_Task */
/**
* @brief Function implementing the Password thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Password_Task */
void Password_Task(void *argument)
{
  /* USER CODE BEGIN Password_Task */
	uint8_t DigitNumberPass=0;
	BaseType_t RetVal=pdFALSE;
	uint8_t LCDPassPos = 1;
  /* Infinite loop */
  for(;;)
  {
	  LCD_Send_String_Pos("Check password", 1, 1);
	  RetVal = xQueueReceive(PassworsHandle, &DigitNumberPass, portMAX_DELAY);
	  if(RetVal == pdTRUE)
	  {
		  if(DigitNumberPass != 99){
			  LCD_Send_Number_Pos(DigitNumberPass, 2, LCDPassPos);
			  enteredPassword[LCDPassPos-1]=DigitNumberPass;

		  }
		  else if(DigitNumberPass == 99)
		  {

			  LCDPassPos++;
		  }
		  if(LCDPassPos==6)
		  {
				if (CheckPassword(enteredPassword) == 1)
				{

					LCD_Send_Command(_LCD_CLEAR);
					LCD_Send_String_Pos("correct password", 1, 1);
					HAL_Delay(3000);
					LCD_Send_Command(_LCD_CLEAR);
					LCD_Send_String_Pos("Door is opened", 1, 1);
					vTaskSuspend(PasswordHandle);
					vTaskSuspend(KeypadTaskHandle);
				}
				else
				{

					LCD_Send_Command(_LCD_CLEAR);
					LCD_Send_String_Pos("wrong password", 1, 1);
					HAL_Delay(3000);
					LCD_Send_String_Pos("Check password", 1, 1);
					LCDPassPos=1;
					enteredPassword[0]=0;
					enteredPassword[1]=0;
					enteredPassword[2]=0;
					enteredPassword[3]=0;
				}
		  }
	  }
	  else
	  {

	  }

  }
  /* USER CODE END Password_Task */
}

/* USER CODE BEGIN Header_Temperature_Task */
/**
* @brief Function implementing the TempTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Temperature_Task */
void Temperature_Task(void *argument)
{
  /* USER CODE BEGIN Temperature_Task */
	BaseType_t RetVal=pdFALSE;
	uint32_t Temp=0;
  /* Infinite loop */
  for(;;)
  {
	  HAL_ADC_Start_IT(&hadc1);
	  //HAL_ADC_Start_IT(&hadc1);
	  RetVal = xQueueReceive(TempQueueHandle, &Temp, portMAX_DELAY);
	  if(RetVal == pdTRUE)
	  {
			osSemaphoreAcquire(LCD_SemaphoerHandle,portMAX_DELAY);
			LCD_Send_String_Pos("Temp: ", 1, 1);
			LCD_Send_Number_Pos(Temp / 80, 2, 1);

			Temp = Temp / 80;
			if (Temp > 35) {
				FanSpeed(FAN_MAX_SPEED);
			} else if (Temp > 20 && Temp < 35) {
				FanSpeed(FAN_HAIF_SPEED);
			} else {
				FanSpeed(FAN_LOW_SPEED);
			}
			osSemaphoreRelease(LCD_SemaphoerHandle);
	  }
	  else
	  {

	  }
      osDelay(100);
  }
  /* USER CODE END Temperature_Task */
}

/* USER CODE BEGIN Header_Light_Task */
/**
* @brief Function implementing the lightTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Light_Task */
void Light_Task(void *argument)
{
  /* USER CODE BEGIN Light_Task */
	BaseType_t RetVal=pdFALSE;
	uint32_t Gas=0;
  /* Infinite loop */
  for(;;)
  {
	  //HAL_ADC_Start_IT(&hadc1);
	  RetVal = xQueueReceive(GasQueueHandle, &Gas, portMAX_DELAY);
	  if(RetVal == pdTRUE)
	  {
		  osSemaphoreAcquire(LCD_SemaphoerHandle,portMAX_DELAY);

		  LCD_Send_String_Pos("Gas:      ", 1, 7);
		  LCD_Send_Number_Pos(Gas / 15, 2, 7);
		  Gas = Gas / 15;
		  if(Gas>200)
		  {
			  xEventGroupSetBits(windowHandle,0x01);
		  }
		  osSemaphoreRelease(LCD_SemaphoerHandle);
	  }
	  else
	  {

	  }
	  osDelay(100);
  }
  /* USER CODE END Light_Task */
}

/* USER CODE BEGIN Header_Window_Task */
/**
* @brief Function implementing the Window thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Window_Task */
void Window_Task(void *argument)
{
  /* USER CODE BEGIN Window_Task */
	BaseType_t RetVal=pdFALSE;
	uint32_t Gas=0;
	EventBits_t uxBits;
  /* Infinite loop */
  for(;;)
  {
		uxBits = xEventGroupWaitBits(windowHandle, 0x01, pdTRUE, pdTRUE,
				portMAX_DELAY);
		if(uxBits & 1)
		{
			osSemaphoreAcquire(LCD_SemaphoerHandle, portMAX_DELAY);
			HAL_Delay(500);
			LCD_Send_String_Pos("gas leakage      ", 1, 1);
			LCD_Send_String_Pos("Window is opened", 2, 1);
			HAL_Delay(2500);
			LCD_Send_Command(_LCD_CLEAR);
			osSemaphoreRelease(LCD_SemaphoerHandle);
		}


  }
  /* USER CODE END Window_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
uint8_t CheckPassword(uint8_t* ptAPassword)
{
	uint8_t CheckVar=0;
	for(uint8_t counter=0;counter<4;counter++)
	{
		if(ptAPassword[counter] == correctPasword[counter])
		{
			CheckVar++;
		}
	}
	if(CheckVar == 4)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



void FanSpeed(fanSpeed_t speed)
{
	switch (speed) {
	case FAN_MAX_SPEED:
		TIM1->CCR1 = 65535;
		break;
	case FAN_HAIF_SPEED:
		TIM1->CCR1 = 65535/3;
		break;
	case FAN_LOW_SPEED:
		TIM1->CCR1 = 65535/35;
		break;
	}
}
/* USER CODE END Application */

