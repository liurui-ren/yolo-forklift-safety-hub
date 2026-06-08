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
  * If no LICENSE file is found, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2s_audio.h"
#include "audio_track1.h"
#include "audio_track2.h"
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

#define RX_BUF_SIZE 32
#define OFFLINE_TIMEOUT_MS 3000

static uint8_t rx_buf[RX_BUF_SIZE];
static uint8_t rx_idx = 0;
static uint8_t sys_state = 0;
static uint8_t prev_state = 0xFF;
static uint32_t last_state_time = 0;
static uint32_t state_blink_timer = 0;
static uint8_t audio_playing = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void i2s_pins_safe(void);
static void play_audio_track1(void);
static void play_audio_track2(void);
static void parse_rx_buf(void);
static void apply_state(uint8_t state);
static void stop_all_alarm(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void i2s_pins_safe(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_15, GPIO_PIN_RESET);
}

static void play_audio_track1(void)
{
    audio_playing = 1;
    HAL_I2S_DeInit(&hi2s2);
    I2S_Audio_Init();
    I2S_Audio_PlayADPCM(audio_track1_adpcm, sizeof(audio_track1_adpcm), AUDIO_TRACK1_NUM_SAMPLES);
    i2s_pins_safe();
    audio_playing = 0;
}

static void play_audio_track2(void)
{
    audio_playing = 1;
    HAL_I2S_DeInit(&hi2s2);
    I2S_Audio_Init();
    I2S_Audio_PlayADPCM(audio_track2_adpcm, sizeof(audio_track2_adpcm), AUDIO_TRACK2_NUM_SAMPLES);
    i2s_pins_safe();
    audio_playing = 0;
}

static void stop_all_alarm(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

static void parse_rx_buf(void)
{
    if (rx_idx == 1 && (rx_buf[0] == '0' || rx_buf[0] == '1' || rx_buf[0] == '2'))
    {
        sys_state = rx_buf[0] - '0';
        last_state_time = HAL_GetTick();
        return;
    }

    if (rx_idx < 7) return;
    if (rx_buf[0] != 'S' || rx_buf[1] != 'T' || rx_buf[2] != 'A' ||
        rx_buf[3] != 'T' || rx_buf[4] != 'E' || rx_buf[5] != ':') return;

    uint8_t code = rx_buf[6];
    if (code == '0' || code == '1' || code == '2')
    {
        sys_state = code - '0';
        last_state_time = HAL_GetTick();
    }
}

static void apply_state(uint8_t state)
{
    if (state == prev_state) return;
    prev_state = state;

    stop_all_alarm();

    if (state == 1)
    {
        state_blink_timer = HAL_GetTick();
    }
    else if (state == 2)
    {
        state_blink_timer = HAL_GetTick();
    }
}
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  I2S_Audio_Init();
  i2s_pins_safe();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);

  __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);
  __HAL_UART_DISABLE_IT(&huart2, UART_IT_PE);
  __HAL_UART_DISABLE_IT(&huart2, UART_IT_ERR);
  HAL_NVIC_DisableIRQ(USART2_IRQn);

  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
      __HAL_UART_CLEAR_OREFLAG(&huart2);
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_FE))
      __HAL_UART_CLEAR_FEFLAG(&huart2);
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_NE))
      __HAL_UART_CLEAR_NEFLAG(&huart2);
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_PE))
      __HAL_UART_CLEAR_PEFLAG(&huart2);

  {
      uint32_t poweron_start = HAL_GetTick();
      while ((HAL_GetTick() - poweron_start) < 5000)
      {
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
          HAL_Delay(200);
          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
          HAL_Delay(800);
      }
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (USART2->SR & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE))
    {
        volatile uint32_t tmp_sr = USART2->SR;
        volatile uint32_t tmp_dr = USART2->DR;
        (void)tmp_sr;
        (void)tmp_dr;
    }

    if (USART2->SR & USART_SR_RXNE)
    {
        uint8_t ch = (uint8_t)(USART2->DR & 0xFF);

        if (ch == '\n' || ch == '\r')
        {
            if (rx_idx > 0)
            {
                parse_rx_buf();
                rx_idx = 0;
            }
        }
        else
        {
            if (rx_idx < RX_BUF_SIZE - 1)
            {
                rx_buf[rx_idx++] = ch;
            }
            else
            {
                rx_idx = 0;
            }
        }
    }

    if (last_state_time > 0 && (HAL_GetTick() - last_state_time) > OFFLINE_TIMEOUT_MS)
    {
        sys_state = 0;
        last_state_time = 0;
    }

    apply_state(sys_state);

    if (sys_state == 1)
    {
        if (!audio_playing)
        {
            play_audio_track1();
        }

        uint32_t elapsed = HAL_GetTick() - state_blink_timer;
        if (elapsed < 500)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
        else if (elapsed < 1000)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else if (elapsed < 6000)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else
        {
            state_blink_timer = HAL_GetTick();
        }

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
        HAL_Delay(800);
    }
    else if (sys_state == 2)
    {
        if (!audio_playing)
        {
            play_audio_track2();
        }

        uint32_t elapsed = HAL_GetTick() - state_blink_timer;
        if (elapsed < 250)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
        else if (elapsed < 500)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else if (elapsed < 750)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        }
        else if (elapsed < 1000)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else if (elapsed < 6000)
        {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        }
        else
        {
            state_blink_timer = HAL_GetTick();
        }

        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
        HAL_Delay(100);
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file where the assert_param error occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add your own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
