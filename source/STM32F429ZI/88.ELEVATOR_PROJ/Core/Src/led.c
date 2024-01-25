#include "led.h"   // < >
#include "button.h"

#define LED_ON_UP 0
#define LED_ON_DOWN 1


void led_main(void);
void led_all_on(void);
void led_all_off(void);
void led_on_down();
void led_on_up();
void flower_on();
void flower_off();
void led_keepon_up();
void led_keepon_down();

extern volatile int t1ms_counter;  // volatile : for disable optimize

void button0_led_all_on_off_toggle(void)
{
	static int button0_count=0;   // static으로 선언을 하면 전역 변수 처럼 동작을 한다.

	// 버튼을 한번 눌렀다 뗀 상태라면
	if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
	{
		button0_count++;
		button0_count %= 2;
		if (button0_count)
		{
			led_all_on();
		}
		else
		{
			led_all_off();
		}
	}
}
void button0_toggle()
{
	// 1: led_all_on()
	// 2: led_all_off
	// 3: flower_on
	// 4: flower_off
	// 5: led_keepon_up
	// 6: led_keepon_down
	// 7: led_on_up
	// 8: led_on_down
}

int func_index=0;

void led_main(void)
{
	while(1)
	{
		if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);   // LED1
		}
		else if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);   // LED2
		}
		else if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);   // LED3
		}
		else if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
		{
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);   // LED1
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);   // LED2
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);   // LED3
		}
#if 0
		if (t1ms_counter >= 200)
		{
			t1ms_counter=0;
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);   // LED1
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);   // LED2
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);   // LED3
		}
#endif

#if 0
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);   // LED1
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);   // LED2
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);   // LED3
		HAL_Delay(200);   // 200ms
#endif
#if 0
		button0_led_all_on_off_toggle();
#endif

#if 0
		// PHASE#5
		// flower on/off
		flower_on();
		flower_off();
#endif

#if 0
		// PHASE#4
		// [STM32] led_keepon_up/down
		// 1. source 2.동작 동영상
		// Phase#4
		// 0->1->2->3->4->5->6->7
		// 앞전에 ON 했던 LED는 그대로 유지
		led_keepon_up();
		led_all_off();
		// 7->6->5->4->3->2->1->0
		// 앞전에 ON 했던 LED는 그대로 유지
		led_keepon_down();
		led_all_off();
#endif

#if 0
		// Phase#3
		// 0->1->2->3->4->5->6->7
		// 해당 되는 bit의 LED만 ON

		switch(func_index)
		{
			case LED_ON_UP:
				led_on_up();
				break;
			case LED_ON_DOWN:
				led_on_down();
				break;
			default:
				break;
		}
//		led_on_up();
//		// 7->6->5->4->3->2->1->0
//		led_on_down();
#endif

#if 0
		// Phase#2
//		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_All);
//		HAL_Delay(500);
#endif
		// phase#1
#if 0
		led_all_on();
		HAL_Delay(300);
		led_all_off();
		HAL_Delay(300);
#endif
	}
}

void flower_on()
{
	HAL_GPIO_WritePin(GPIOD, 0xff, GPIO_PIN_RESET);

	for(int i=0; i < 4 ; i++)
	{
		HAL_GPIO_WritePin(GPIOD, 0x10 << i, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOD, 0x08 >> i, GPIO_PIN_SET);
		HAL_Delay(300);
	}
}


void flower_off()
{
	HAL_GPIO_WritePin(GPIOD, 0xff, GPIO_PIN_SET);


	for(int i=0; i < 4 ; i++)
	{
		HAL_GPIO_WritePin(GPIOD, 0x80 >> i, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, 0x01 << i, GPIO_PIN_RESET);
		HAL_Delay(300);
	}
}

void led_keepon_up()
{
	for (int i=0; i < 8; i++)
	{
		HAL_GPIO_WritePin(GPIOD, 0x01 << i, 1);
		HAL_Delay(200);
	}
}

void led_keepon_down()
{
	for (int i=0; i < 8; i++)
	{
		HAL_GPIO_WritePin(GPIOD, 0x80 >> i, 1);
		HAL_Delay(200);
	}
}

// 0->1->2->3->4->5->6->7
// 해당 되는 bit의 LED만 ON
void led_on_up()
{
	static int i=0;

#if 1
	if (t1ms_counter >= 200)
	{
		t1ms_counter=0;
		led_all_off();
		HAL_GPIO_WritePin(GPIOD, 0x01 << i, 1);
		i++;
		if (i >= 8)
		{
			i=0;
			func_index=LED_ON_DOWN;
		}
	}
#else  // orginal
	for (int i=0; i < 8; i++)
	{
		led_all_off();
		HAL_GPIO_WritePin(GPIOD, 0x01 << i, 1);
		HAL_Delay(200);
	}
#endif
}

// 7->6->5->4->3->2->1->0
void led_on_down()
{
	static int i=0;
#if 1
	if (t1ms_counter >= 200)
	{
		t1ms_counter=0;
		led_all_off();
		HAL_GPIO_WritePin(GPIOD, 0x80 >> i, 1);
		i++;
		if (i >= 8)
		{
			i=0;
			func_index=LED_ON_UP;
		}
	}
#else
	for (int i=0; i < 8; i++)
	{
		led_all_off();
		HAL_GPIO_WritePin(GPIOD, 0x80 >> i, 1);
		HAL_Delay(200);
	}
#endif
}

void led_all_on(void)
{
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
//			GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 1);
	HAL_GPIO_WritePin(GPIOD, 0xff, 1);
}

void led_all_off(void)
{
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
//			GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, 0);
	HAL_GPIO_WritePin(GPIOD, 0xff, 0);
}
