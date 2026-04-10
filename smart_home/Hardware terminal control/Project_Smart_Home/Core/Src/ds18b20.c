#include "ds18b20.h"
#include "gpio.h"
#include "delay.h"

static void DS18B20_Mode(uint8_t mode)
{
	GPIO_InitTypeDef conf = {0};
	conf.Pin = DS18B20_PIN;
	conf.Speed = GPIO_SPEED_FREQ_HIGH;
	if(mode == 1)
	{
		conf.Pull = GPIO_NOPULL;
		conf.Mode = GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(DS18B20_GPIO_PORT,&conf);
	}
	else
	{
		conf.Mode = GPIO_MODE_INPUT;
		conf.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(DS18B20_GPIO_PORT,&conf);
	}
}

void DS18B20_Init(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
	DS18B20_Mode(1);
	HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_PIN, GPIO_PIN_SET); // 总线空闲高电平
}



static void Write_Data(uint8_t data)
{
	uint8_t i = 0;
	for(i = 0;i < 8;i++)
	{
		DS18B20_Mode(1);
		
		HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_RESET);
		Delay_US(2);
		
		if(data & 0x01)
		{
	
			HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_SET);
		}
		else
		{
			HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_RESET);
		}
		Delay_US(60);
		HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_SET);
		Delay_US(1);
		data >>= 1;
	}
}

static uint8_t Recv_Data(void)
{
	uint8_t i,dat = 0;
	for(i = 0;i < 8;i++)
	{
		DS18B20_Mode(1);
		HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_RESET);
		Delay_US(2);
		
		DS18B20_Mode(0);  // 切换输入读取
		Delay_US(5);   // 5us后采样
		
		if(HAL_GPIO_ReadPin(DS18B20_GPIO_PORT,DS18B20_PIN))
		{
			dat |= 0x80;
		}
		Delay_US(50);
    DS18B20_Mode(1);
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_PIN, GPIO_PIN_SET);
		
		if(i < 7) dat >>= 1;
	}
	return dat;
}

static uint8_t DS18B20_Reset(void)
{
	DS18B20_Mode(1);
	//复位脉冲
	HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_RESET);
	Delay_US(480);
	
	HAL_GPIO_WritePin(DS18B20_GPIO_PORT,DS18B20_PIN,GPIO_PIN_SET);
	DS18B20_Mode(0);
	Delay_US(60);
	
	if(HAL_GPIO_ReadPin(DS18B20_GPIO_PORT,DS18B20_PIN))
	{
		return 1;
	}
	
	Delay_US(420);
	return 0;
}
	
uint8_t DS18B20_Get_Data(float *data)
{
    uint8_t LSB, MSB;
    int16_t temp;

    // 第一步：复位
    if(DS18B20_Reset()) return 1;

    // 第二步：启动温度转换
    Write_Data(0xCC);
    Write_Data(0x44);

	    // ✅ 寄生供电最重要一步：转换期间总线保持强高电平供电
    DS18B20_Mode(1);
    HAL_GPIO_WritePin(DS18B20_GPIO_PORT, DS18B20_PIN, GPIO_PIN_SET);
    HAL_Delay(1);

    // 第三步：再次复位读数据
    if(DS18B20_Reset()) return 1;

    Write_Data(0xCC);
    Write_Data(0xBE);

    // 读高低字节
    LSB = Recv_Data();
    MSB = Recv_Data();

    // 温度计算
    temp = (MSB << 8) | LSB;
    *data = temp * 0.0625f;

    return 0;
}
