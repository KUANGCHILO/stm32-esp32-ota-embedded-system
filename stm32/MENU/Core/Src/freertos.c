/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "4x4_keypad.h"
#include "usart.h"
#include <string.h>
#include "../../App/menu/menu.h"
#include "../../App/menu/menu_render.h"
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
/* USER CODE BEGIN Variables */
extern MenuContext ctx;
/* USER CODE END Variables */
/* Definitions for GetKeyTask */
osThreadId_t GetKeyTaskHandle;
const osThreadAttr_t GetKeyTask_attributes = {
  .name = "GetKeyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for rendertask */
osThreadId_t rendertaskHandle;
const osThreadAttr_t rendertask_attributes = {
  .name = "rendertask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for i2cMutex */
osMutexId_t i2cMutexHandle;
const osMutexAttr_t i2cMutex_attributes = {
  .name = "i2cMutex",
  .attr_bits = osMutexRecursive,
};
/* Definitions for BtnPressed */
osSemaphoreId_t BtnPressedHandle;
const osSemaphoreAttr_t BtnPressed_attributes = {
  .name = "BtnPressed"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void GetketpadTask(void *argument);
void RenderTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Create the recursive mutex(es) */
  /* creation of i2cMutex */
  i2cMutexHandle = osMutexNew(&i2cMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of BtnPressed */
  BtnPressedHandle = osSemaphoreNew(1, 0, &BtnPressed_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of GetKeyTask */
  GetKeyTaskHandle = osThreadNew(GetketpadTask, NULL, &GetKeyTask_attributes);

  /* creation of rendertask */
  rendertaskHandle = osThreadNew(RenderTask, NULL, &rendertask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_GetketpadTask */
/**
  * @brief  Function implementing the GetKeyTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_GetketpadTask */
void GetketpadTask(void *argument)
{
  /* USER CODE BEGIN GetketpadTask */
  /* Infinite loop */
  for(;;)
  {
    osSemaphoreAcquire(BtnPressedHandle , osWaitForever);
    ButtonEvent evt = Keypad_GetKey();    
    if(evt != 0) {
        osDelay(1);
        ctx.handle_event(&ctx,evt);
        //menu_handle_event(&ctx,evt);
        //HAL_UART_Transmit(&huart2, (uint8_t *)key, 1, HAL_MAX_DELAY);
    }
  }
  /* USER CODE END GetketpadTask */
}

/* USER CODE BEGIN Header_RenderTask */
/**
* @brief Function implementing the rendertask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RenderTask */
void RenderTask(void *argument)
{
  /* USER CODE BEGIN RenderTask */
  /* Infinite loop */
  for(;;)
  {
    osMutexAcquire(i2cMutexHandle, osWaitForever);
    if (ctx.active_screen) {
      ctx.active_screen();
    } else if (ctx.need_redraw) {
      menu_render(&ctx);
      ctx.need_redraw = 0;
    }
    osDelay(50);
    osMutexRelease(i2cMutexHandle);
  }
  /* USER CODE END RenderTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

