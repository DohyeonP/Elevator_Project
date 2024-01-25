#include "main.h"

#define FND_DATA_PORT GPIOB   // data port

#define FND_a  FND_7_Pin
#define FND_b  FND_6_Pin
#define FND_c  FND_4_Pin
#define FND_d  FND_2_Pin
#define FND_e  FND_1_Pin
#define FND_f  FND_9_Pin
#define FND_g  FND_10_Pin
#define FND_p  FND_5_Pin

extern int e_floor;

uint16_t FND_font[10] =
{
  FND_a|FND_b|FND_c|FND_d|FND_e|FND_f,   // 0
  FND_b|FND_c,                           // 1
  FND_a|FND_b|FND_d|FND_e|FND_g,         // 2
  FND_a|FND_b|FND_c|FND_d|FND_g,         // 3
  FND_b|FND_c|FND_f|FND_g,   // 4
  FND_a|FND_c|FND_d|FND_f|FND_g,  // 5
  FND_a|FND_c|FND_d|FND_e|FND_f|FND_g,  // 6
  FND_a|FND_b|FND_c,      // 7
  FND_a|FND_b|FND_c|FND_d|FND_e|FND_f|FND_g,   // 8
  FND_a|FND_b|FND_c|FND_d|FND_f|FND_g   // 9
};

uint16_t FND_all = FND_a|FND_b|FND_c|FND_d|FND_e|FND_f|FND_g|FND_p;

void fnd_processing(void)
{
	HAL_GPIO_WritePin(FND_DATA_PORT, FND_all, GPIO_PIN_SET);
	HAL_GPIO_WritePin(FND_DATA_PORT, FND_font[e_floor]|FND_p, GPIO_PIN_RESET);
}
