#include "key.h"

void Key_Init(void){
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

}

void key_scan(void){
	if(Touch_Sensor == 1){
		Delay_ms(5);
		if(Touch_Sensor == 1){
			printf("10\r\n");
		}
		while(Touch_Sensor);
	}
//	if(Key1 == 0){
//		Delay_ms(5);
//		if(Key1 == 0){
//			printf("1\r\n");
//		}
//		while(!Key1);
//	}
//	
//	if(Key0 == 0){
//		Delay_ms(5);
//		if(Key0 == 0){
//			printf("0\r\n");
//		}
//		while(!Key0);
//	}
	
}







