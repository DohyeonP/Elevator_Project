#include "main.h"

int dotmatrix_processing(int flag);
void init_dormatrix(void);

uint8_t up_data[8] =
{
	0b00001110,
	0b00011100,
	0b00111000,
	0b01110000,
	0b01110000,
	0b00111000,
	0b00011100,
	0b00001110
};

uint8_t down_data[8] =
{
	0b01110000,
	0b00111000,
	0b00011100,
	0b00001110,
	0b00001110,
	0b00011100,
	0b00111000,
	0b01110000
};

GPIO_TypeDef *col_port[] =
{
		COL1_GPIO_Port, COL2_GPIO_Port, COL3_GPIO_Port, COL4_GPIO_Port,
		COL5_GPIO_Port, COL6_GPIO_Port, COL7_GPIO_Port, COL8_GPIO_Port
};

GPIO_TypeDef *row_port[] =
{
		ROW1_GPIO_Port, ROW2_GPIO_Port, ROW3_GPIO_Port, ROW4_GPIO_Port,
		ROW5_GPIO_Port, ROW6_GPIO_Port, ROW7_GPIO_Port, ROW8_GPIO_Port
};

uint16_t col_pin[] =
{
		COL1_Pin, COL2_Pin, COL3_Pin, COL4_Pin,
		COL5_Pin, COL6_Pin, COL7_Pin, COL8_Pin
};

uint16_t row_pin[] =
{
		ROW1_Pin, ROW2_Pin, ROW3_Pin, ROW4_Pin,
		ROW5_Pin, ROW6_Pin, ROW7_Pin, ROW8_Pin
};


// 8 x 8 dot matrix 를 출력할 데이터
unsigned char display_data[8];

void write_column_data(int column)
{
	for (int i=0; i<8; i++)
	{
		if (i == column)
			HAL_GPIO_WritePin(col_port[i], col_pin[i], 0); // on
		else
			HAL_GPIO_WritePin(col_port[i], col_pin[i], 1); // off
	}
}

void write_row_data(unsigned char data)
{
	unsigned char d = data;

	for (int i=0; i<8; i++)
	{
		if ( d & (1 << i))
			HAL_GPIO_WritePin(row_port[i], row_pin[i], 1);
		else
			HAL_GPIO_WritePin(row_port[i], row_pin[i], 0);
	}
}

void dotmatrix_clear(void)
{
	for (int i=0; i<8; i++)
	{
		HAL_GPIO_WritePin(col_port[i], col_pin[i], 1); // off
	}
}

// scroll 문자 출력 프로그램
int dotmatrix_processing(int flag)
{
	// column count
	static int count = 0;

	// 이전 tick 값 저장
	static uint32_t past_time=0;

	if (flag == 1) // UP!
	{
		uint32_t now = HAL_GetTick(); // 1ms

		// 처음 시작 시 past time = 0
		// polling 방식으로 작동하기 때문에 500 이 넘을 때 조건을 검사할 경우도 있다.
		// 그렇기 때문에 500 보다 크거나 같은 조건을 줘야 한다.
		// 500 과 같은 조건을 체크하면 안된다!
		if (now - past_time >= 100)
		{
			past_time = now;

			for (int i=0; i<8; i++)
			{
				display_data[i] = (up_data[i] << count) | (up_data[i] >> (8 - count));
			}

			if (++count >= 8)
			{
				count = 0;
			}
		}

		for(int i=0; i<8; i++)
		{
			// common anode 방식
			// column 에는 low, row 에는 high 를 출력해야 해당 LED 가 켜진다.
			write_column_data(i);
			write_row_data(display_data[i]);

			HAL_Delay(1);
		}
	}
	else if (flag == 2) // DOWN!
	{
		uint32_t now = HAL_GetTick(); // 1ms

		// 처음 시작 시 past time = 0
		// polling 방식으로 작동하기 때문에 500 이 넘을 때 조건을 검사할 경우도 있다.
		// 그렇기 때문에 500 보다 크거나 같은 조건을 줘야 한다.
		// 500 과 같은 조건을 체크하면 안된다!
		if (now - past_time >= 100)
		{
			past_time = now;

			for (int i=0; i<8; i++)
			{
				display_data[i] = (down_data[i] >> count) | (down_data[i] << (8 - count));
			}

			if (++count >= 8)
			{
				count = 0;
			}
		}

		for(int i=0; i<8; i++)
		{
			// common anode 방식
			// column 에는 low, row 에는 high 를 출력해야 해당 LED 가 켜진다.
			write_column_data(i);
			write_row_data(display_data[i]);

			HAL_Delay(1);
		}
	}
	else
	{
		dotmatrix_clear();
	}

	return 0;
}
