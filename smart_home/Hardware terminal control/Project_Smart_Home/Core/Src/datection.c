#include "detection.h"
#include "adc.h"

void Detection_Init(uint32_t *data)
{
	uint8_t i;
	
	for(i= 0;i < 3;i++)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1 , 100);    //设置CPU等待ADC转换的时间
		data[i] = 4096 - HAL_ADC_GetValue(&hadc1);
	}
	
	HAL_ADC_Stop(&hadc1);
}
