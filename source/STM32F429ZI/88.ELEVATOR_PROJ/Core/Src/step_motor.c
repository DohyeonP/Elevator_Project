#include "main.h"
#include "button.h"

#define UP 1
#define DOWN 2
#define MAX 13

extern int get_button(GPIO_TypeDef *GPIO, uint16_t GPIO_PIN, uint8_t button_number);
extern void delay_us(unsigned long us);
extern void lcd_string(uint8_t *str);
extern void move_cursor(uint8_t row, uint8_t column);

void step_motot_main_test(void);
void step_motor_drive(int step);
void step_motor_processing(void);
void set_rpm(int rpm);

extern volatile int TIM10_10ms_stepmotor_delay;
extern volatile int TIM10_10ms_wait;

extern int e_floor;
extern int up_down_flag;
extern int step_stop_flag;

int f1;
int f2;
int f3;
int f4;

int lcd_hold = 1;
int lcd_stop = 1;
int lcd_up = 1;
int lcd_down = 1;

int prev_floor;
int hold = 0;
int up = 0;
int down = 0;
int stop = 1;

// RPM (Revolutions per Minutes) : 분당 회전 수
// 1min : 60s : 1,000,000us(1초) * 60 = 60,000,000us
// 1,000,000us(1초)
// --> 1초(1000ms) --> 1ms(1000us) * 1000ms --> 1,000,000us
// 4096스텝 : 1바퀴(4096 스텝이동)
//---------------------------------------------------------
// 1바퀴 도는데 필요한 총 스텝 수 : 4096
// 4096 / 8 sequence (0.7도) ==> 512 sequence : 360도
// 1 sequence(8step) : 0.70312도
// 0.70312도 * 512 sequence = 360도

// direction == 0 : stop(idle)
// direction == 1 : 시계방향
// direction == 2 : 반시계방향
void step_motor_drive(int direction)
{
	static int step = 0;

	switch(step){
	case 0:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 1);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
		break;
	case 1:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 1);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 1);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
		break;
	case 2:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 1);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
		break;
	case 3:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 1);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 1);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
		break;
	case 4:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 1);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 0);
		break;
	case 5:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 1);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 1);
		break;
	case 6:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 0);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 1);
		break;
	case 7:
		HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, 1);
		HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, 0);
		HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, 0);
		HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, 1);
		break;
	default:
		break;
	}

	// 정회전 -> 올라가는 방향
	if (direction == 1)
	{
		step++;
		step %= 8;
	}
	// 역회전 -> 내려가는 방향
	else if (direction == 2)
	{
		step--;
		if (step < 0) step = 7;
	}
}

//--- set_rpm(13) 으로 지정시의 동작 상황 ---
// 60,000,000us(1분) / 4096 / 시간 = RPM
// 물리적으로 1 step 작동할 때 1126us 가 소요된다.
// 1126us(1스텝 idle time) * 4096 = 4,612,096us
// 								= 4612ms
//								= 4.6s
// 60초 / 4.6 ==> 13회전 (분당)
void set_rpm(int rpm)
{
	// 물리적으로 1rpm ~ 13rpm 까지 조절 가능
	delay_us((60000000/4096)/rpm);

	// 최대 speed 기준 => rpm = 13
	// delay_us(1126)
}

void step_motor_processing(void)
{
	if (hold)
	{
		lcd_stop = 1;
		lcd_up = 1;
		lcd_down = 1;

		if (lcd_hold)
		{
			lcd_hold = 0;
			move_cursor(0, 0);
			lcd_string("HOLD");
		}
		if (TIM10_10ms_stepmotor_delay >= 300)
		{
			hold = 0;
		}

		switch(up_down_flag)
		{
		case 1: // UP
			if (get_button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
			{
				if (e_floor < 4)
				{
					if (f4)
					{
						move_cursor(1, 0);
						lcd_string("    ");
						f4 = 0;
					}
					else
					{
						move_cursor(1, 0);
						lcd_string("   4");
						f4 = 1;
					}
				}
			}
			if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
			{
				if (e_floor < 3)
				{
					if (f3)
					{
						move_cursor(1, 4);
						lcd_string("    ");
						f3 = 0;
					}
					else
					{
						move_cursor(1, 4);
						lcd_string("   3");
						f3 = 1;
					}
				}
			}
			if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
			{
				if (e_floor < 2)
				{
					if (f2)
					{
						move_cursor(1, 8);
						lcd_string("    ");
						f2 = 0;
					}
					else
					{
						move_cursor(1, 8);
						lcd_string("   2");
						f2 = 1;
					}
				}
			}
			if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
			{
				if (e_floor < 1)
				{
					if (f1)
					{
						move_cursor(1, 12);
						lcd_string("    ");
						f1 = 0;
					}
					else
					{
						move_cursor(1, 12);
						lcd_string("   1");
						f1 = 1;
					}
				}
			}
			break;

		case 2: // DOWN
			if (get_button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
			{
				if (e_floor > 4)
				{
					if (f4)
					{
						move_cursor(1, 0);
						lcd_string("    ");
						f4 = 0;
					}
					else
					{
						move_cursor(1, 0);
						lcd_string("   4");
						f4 = 1;
					}
				}
			}
			if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
			{
				if (e_floor > 3)
				{
					if (f3)
					{
						move_cursor(1, 4);
						lcd_string("    ");
						f3 = 0;
					}
					else
					{
						move_cursor(1, 4);
						lcd_string("   3");
						f3 = 1;
					}
				}
			}
			if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
			{
				if (e_floor > 2)
				{
					if (f2)
					{
						move_cursor(1, 8);
						lcd_string("    ");
						f2 = 0;
					}
					else
					{
						move_cursor(1, 8);
						lcd_string("   2");
						f2 = 1;
					}
				}
			}
			if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
			{
				if (e_floor > 1)
				{
					if (f1)
					{
						move_cursor(1, 12);
						lcd_string("    ");
						f1 = 0;
					}
					else
					{
						move_cursor(1, 12);
						lcd_string("   1");
						f1 = 1;
					}
				}
			}
			break;

		case 0: // STOP
			break;

		default:
			break;
		}
	}

	else if (up && !step_stop_flag)
	{
		TIM10_10ms_stepmotor_delay = 0;

		lcd_hold = 1;
		lcd_stop = 1;
		lcd_down = 1;

		if (lcd_up)
		{
			lcd_up = 0;
			move_cursor(0, 0);
			lcd_string("UP!!");
		}

		if (prev_floor != e_floor)
		{
			switch (e_floor)
			{
			case 1:
				prev_floor = e_floor;
				if (f1)
				{
					f1 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 12);
					lcd_string("    ");
				}
				break;
			case 2:
				prev_floor = e_floor;
				if (f2)
				{
					f2 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 8);
					lcd_string("    ");
				}
				break;
			case 3:
				prev_floor = e_floor;
				if (f3)
				{
					f3 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 4);
					lcd_string("    ");
				}
				break;
			case 4:
				prev_floor = e_floor;
				if (f4)
				{
					f4 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 0);
					lcd_string("    ");
				}
				break;
			default:
				break;
			}
		}

		if (f1 || f2 || f3 || f4)
		{
			up_down_flag = UP;
			step_motor_drive(UP);
			set_rpm(MAX);
		}
		else
		{
			up_down_flag = 0;
			if (TIM10_10ms_wait >= 500)
			{
				up = 0;
				stop = 1;
			}
		}
		if (get_button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
		{
			if (e_floor < 4)
			{
				if (f4)
				{
					move_cursor(1, 0);
					lcd_string("    ");
					f4 = 0;
				}
				else
				{
					move_cursor(1, 0);
					lcd_string("   4");
					f4 = 1;
				}
			}
		}
		if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
		{
			if (e_floor < 3)
			{
				if (f3)
				{
					move_cursor(1, 4);
					lcd_string("    ");
					f3 = 0;
				}
				else
				{
					move_cursor(1, 4);
					lcd_string("   3");
					f3 = 1;
				}
			}
		}
		if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
		{
			if (e_floor < 2)
			{
				if (f2)
				{
					move_cursor(1, 8);
					lcd_string("    ");
					f2 = 0;
				}
				else
				{
					move_cursor(1, 8);
					lcd_string("   2");
					f2 = 1;
				}
			}
		}
		if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
		{
			if (e_floor < 1)
			{
				if (f1)
				{
					move_cursor(1, 12);
					lcd_string("    ");
					f1 = 0;
				}
				else
				{
					move_cursor(1, 12);
					lcd_string("   1");
					f1 = 1;
				}
			}
		}
		if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
		{
			if (!(f1 || f2 || f3 || f4))
			{
				up = 0;
				down = 1;
			}
		}
	}
	else if (down && !step_stop_flag)
	{
		TIM10_10ms_stepmotor_delay = 0;

		lcd_hold = 1;
		lcd_stop = 1;
		lcd_up = 1;

		if (lcd_down)
		{
			lcd_down = 0;
			move_cursor(0, 0);
			lcd_string("DOWN");
		}

		if (prev_floor != e_floor)
		{
			switch (e_floor)
			{
			case 1:
				prev_floor = e_floor;
				if (f1)
				{
					f1 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 12);
					lcd_string("    ");
				}
				break;
			case 2:
				prev_floor = e_floor;
				if (f2)
				{
					f2 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 8);
					lcd_string("    ");
				}
				break;
			case 3:
				prev_floor = e_floor;
				if (f3)
				{
					f3 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 4);
					lcd_string("    ");
				}
				break;
			case 4:
				prev_floor = e_floor;
				if (f4)
				{
					f4 = 0;
					TIM10_10ms_stepmotor_delay = 0;
					hold = 1;
					move_cursor(1, 0);
					lcd_string("    ");
				}
				break;
			default:
				break;
			}
		}

		if (prev_floor != e_floor)
		{
			prev_floor = e_floor;
			TIM10_10ms_stepmotor_delay = 0;
			hold = 1;
		}

		if (f1 || f2 || f3 || f4)
		{
			up_down_flag = DOWN;
			step_motor_drive(DOWN);
			set_rpm(MAX);
		}
		else
		{
			up_down_flag = 0;
			if (TIM10_10ms_wait >= 500)
			{
				down = 0;
				stop = 1;
			}
		}
		if (get_button(BUTTON5_GPIO_Port, BUTTON5_Pin, 5) == BUTTON_PRESS)
		{
			if (!(f1 || f2 || f3 || f4))
			{
				up = 1;
				down = 0;
			}
		}
		if (get_button(BUTTON4_GPIO_Port, BUTTON4_Pin, 4) == BUTTON_PRESS)
		{
			if (e_floor > 4)
			{
				if (f4)
				{
					move_cursor(1, 0);
					lcd_string("    ");
					f4 = 0;
				}
				else
				{
					move_cursor(1, 0);
					lcd_string("   4");
					f4 = 1;
				}
			}
		}
		if (get_button(BUTTON3_GPIO_Port, BUTTON3_Pin, 3) == BUTTON_PRESS)
		{
			if (e_floor > 3)
			{
				if (f3)
				{
					move_cursor(1, 4);
					lcd_string("    ");
					f3 = 0;
				}
				else
				{
					move_cursor(1, 4);
					lcd_string("   3");
					f3 = 1;
				}
			}
		}
		if (get_button(BUTTON2_GPIO_Port, BUTTON2_Pin, 2) == BUTTON_PRESS)
		{
			if (e_floor > 2)
			{
				if (f2)
				{
					move_cursor(1, 8);
					lcd_string("    ");
					f2 = 0;
				}
				else
				{
					move_cursor(1, 8);
					lcd_string("   2");
					f2 = 1;
				}
			}
		}
		if (get_button(BUTTON1_GPIO_Port, BUTTON1_Pin, 1) == BUTTON_PRESS)
		{
			if (e_floor > 1)
			{
				if (f1)
				{
					move_cursor(1, 12);
					lcd_string("    ");
					f1 = 0;
				}
				else
				{
					move_cursor(1, 12);
					lcd_string("   1");
					f1 = 1;
				}
			}
		}
	}
	else if (stop)
	{
		prev_floor = e_floor;

		lcd_hold = 1;
		lcd_down = 1;
		lcd_up = 1;

		if (lcd_stop)
		{
			lcd_stop = 0;
			move_cursor(0, 0);
			lcd_string("STOP");
		}

		if (get_button(BUTTON5_GPIO_Port, BUTTON5_Pin, 5) == BUTTON_PRESS)
		{
			up = 1;
			stop = 0;
			TIM10_10ms_wait = 0;
		}
		else if (get_button(BUTTON0_GPIO_Port, BUTTON0_Pin, 0) == BUTTON_PRESS)
		{
			down = 1;
			stop = 0;
			TIM10_10ms_wait = 0;
		}
	}
}
