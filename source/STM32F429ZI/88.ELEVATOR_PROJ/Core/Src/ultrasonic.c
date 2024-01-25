#include "ultrasonic.h"

extern volatile int TIM10_10ms_ultrasonic;
extern void lcd_string(uint8_t *str);
extern void move_cursor(uint8_t row, uint8_t column);
extern uint8_t lcd_display_mode_flag;

void ultrasonic_processing(void);

volatile int distance; // 거리를 측정한 펄스 갯수를 저장 하는 변수
volatile int ic_cpt_finish_flag=0; // 초음파 거리 측정 완료 indicator변수
// 1. Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c에 가서
//  HAL_TIM_IC_CaptureCallback를 잘라내기를 해서 이곳으로 가져 온다.
// 2. 초음파 센서의 ECHO핀의 상승 에지와 하강에지 발생시 이곳으로 들어 온다.
volatile uint8_t  is_first_capture=0;  // 0:상승에지 1:하강에지
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)
	{
		if (is_first_capture == 0)   // 상승에지
		{
			__HAL_TIM_SET_COUNTER(htim,0);   // clear H/W counter
			is_first_capture=1;  // 상승에지를 만났다는 flag변수 indicator를 set
		}
		else if (is_first_capture == 1)   // 하강 에지를 만나면
		{
			is_first_capture=0;  // 다음 echo 펄스를 count하기 위해 변수 초기화
			distance=HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // 현재까지 count한 펄스수를 읽어 온다.
			ic_cpt_finish_flag=1;   // 초음파 측정 완료
		}
	}
}

void ultrasonic_processing(void)
{
	int dis; // 거리를 측정한 펄스 갯수를 저장 하는 변수
	char lcd_buff[20];

	if (TIM10_10ms_ultrasonic >= 100)   // 1초
	{
		TIM10_10ms_ultrasonic=0;
		make_trigger();
		if (ic_cpt_finish_flag)  // 초음파 측정이 완료 되었으면  if (ic_cpt_finish_flag >= 1)
		{
			ic_cpt_finish_flag=0;
			dis = distance;
			dis = dis * 0.034 / 2;  // 1us가 0.034cm를 이동
			// 왕복 거리를 리턴해 주기 떄문에 /2를 해주는 것이다.
			printf("dis: %dcm\n", dis);
			if (lcd_display_mode_flag==2)
			{
				sprintf(lcd_buff, "dis: %dcm    ", dis);
				move_cursor(1,0);
				lcd_string(lcd_buff);
			}
		}
	}
}

void make_trigger()
{
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);
	delay_us(2);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 1);
	delay_us(10);
	HAL_GPIO_WritePin(ULTRASONIC_TRIGGER_GPIO_Port, ULTRASONIC_TRIGGER_Pin, 0);
}







