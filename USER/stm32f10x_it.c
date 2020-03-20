#include "stm32f10x_it.h" 
#include "sys.h"
#include "usart.h"
#include "led.h"

uint8_t updata_flag = 0;
uint8_t main_updata = 0;
uint16_t precise_delay_100ms = 0;
extern uint8_t MAX30100_Calculate_Flag,MAX30100_Success_Count,MAX30100_Fail_Count;

//TIM2中断服务函数
void TIM2_IRQHandler(void){	
	static int count1 ,count2,count3,count4,count5;
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET){//判断是否进入TIM2更新中断
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除TIM2更新中断
		if(++count1 == 50){
			count1 = 0;
			LED0 = !LED0;
		}
	}
	
	if(++count3 == 100){
		count3 = 0;
		main_updata = 1;
	}
	
	if(precise_delay_100ms > 0){
		if(++count2 == 100){
			count2 = 0;
			precise_delay_100ms --;
//			printf("%d \r\n",precise_delay_100ms);
		}
	}
	if(MAX30100_Calculate_Flag == 1){
		if(++count4 == 1000){
			count4 = 0;
			MAX30100_Success_Count++;
		}
	}
	if(++count5 == 1000){
		count5 = 0;
		updata_flag = 1;
	}
}


