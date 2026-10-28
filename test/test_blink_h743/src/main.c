
#include "main.h"
#include "rng.h"
#include "gpio.h"

int main(void)
{

	MPU_Config();
	
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_RNG_Init();

	uint32_t rng;

  while (1)
  {
		HAL_RNG_GenerateRandomNumber(&hrng,&rng);
		
		HAL_GPIO_TogglePin(PE3_GPIO_Port,PE3_Pin);
		HAL_Delay(rng%600);
  }
  /* USER CODE END 3 */
}