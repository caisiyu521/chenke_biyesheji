#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "nvic.h"
#include "led.h"
#include "timer.h"
#include "string.h"
#include "NetWork.h"
#include "MAX30100.h"
#include "MAX30100_PulseOximeter.h"
#include "MAX30100_SpO2Calculator.h"
#include "MAX30100_Filters.h"
#include "timer3.h"
#include "myiic.h"
#include "oled.h"
#include "adc.h"
#include "key.h"
#include "ds1302.h"

extern uint16_t precise_delay_ms;
extern uint8_t RXBuffer[RXBUFFER_LEN];
extern uint8_t updata_flag;
extern uint8_t main_updata;

extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];// ADC转换的电压值通过MDA方式传到SRAM
float ADC_ConvertedValueLocal[NOFCHANEL];  // 局部变量，用于保存转换计算后的电压值 	  

uint8_t HeartRate,SPO2;
uint8_t string[100];
uint8_t MAX30100_HeartRate_Buf[200];//用来缓存数据，，因为数据可能会有一两次不正确
uint8_t MAX30100_SPO2_Buf[200];
uint8_t	MAX30100_Shang[100];

void MAX30100_Calculate(void);
uint8_t MAX30100_Calculate_Flag = 0;
uint8_t	MAX30100_Success_Count = 0;
uint8_t	MAX30100_Fail_Count = 0;
int i = 0,j = 0;


float x_last=0;  //卡尔曼滤波
float p_last=0.02;
float Q=0.018;
float R=0.542;
float kg;
float x_mid;
float x_now;
float p_mid;
float p_now;
float z_real=0.56;//0.56
float z_measure;

int guangzhao;
int guangzhao1;
int main(void){	
	
	
	delay_init();	    	//延时函数初始化	  
	uart1_init(115200);	 	//串口初始化为115200
	uart2_init(115200);	 	//串口初始化为115200
	LED_Init();				//初始化 LED
	Timer2_Init(1000,72);	//初始化 Timer2 1ms 中断一次
	TIM3_Int_Init(100-1,720-1);//1ms
//	Key_Init();
	IIC_OLED_Init();		//OLED IIC 初始化
	OLED_Init();			//OLED 初始化
	Ds1302_Gpio_Init();
	ADCx_Init();		  		//ADC初始化
	
	IIC_Init();				//血氧模块
	SPO2_Init();
	
	nvic_init();			//配置 中断的 优先级 ， 放初始化 后面
	
	printf("CPU Start !\r\n");
	
	OLED_ON();OLED_CLS();	OLED_CLS();OLED_CLS();//OLED 清屏
	OLED_ShowCHinese(0,2,0);OLED_ShowCHinese(16,2,1);OLED_ShowCHinese(32,2,2);
	OLED_ShowCHinese(48,2,3);OLED_ShowCHinese(64,2,4);OLED_ShowCHinese(80,2,5);
	OLED_ShowCHinese(96,2,6);//OLED显示 开机初始化界面			“系统正在初始化”
	
	printf("ESP_INIT Connect,Just a moment, please\r\n");
	
//	if(initESP8266()!=0){
//		printf("ESP8266 Init ok\r\n");
//    }else {
//		printf("ESP8266 Init error\r\n");
//    }
//    if(connectAP((unsigned char *)SSID,(unsigned char *)PWD)!=0){
//		printf("ESP8266 ConnectAP ok\r\n");
//    }else {
//		printf("ESP8266 ConnectAP error\r\n");
//    }
//    if(connectServer((unsigned char *)TCP,(unsigned char *)IP,PORT)!=0){
//		printf("ESP8266 ConnectServer ok\r\n");
//    }else {
//		printf("ESP8266 ConnectServer error\r\n");
//    }
//	
	Usart_SendString(USART2,"Hello Servor , I am MCU!\r\n");
	
	delay_precise_100ms(50);//精确延时	x*100ms

	
//	OLED_CLS();

	while(1){
		
//		Ds1302_Readtime();
//		printf("hour = %d ,min = %d ,sec = %d\r\n",ds1302Data.hour,ds1302Data.min,ds1302Data.sec);
		
		
//		key_scan();
		
		ADC_ConvertedValueLocal[0] =(float) ADC_ConvertedValue[0]/4096*100;
		ADC_ConvertedValueLocal[1] =(float) ADC_ConvertedValue[1]/4096*100;
//		guangzhao = ADC_ConvertedValueLocal[0];
//		guangzhao1 = ADC_ConvertedValueLocal[1];
//		sprintf((char*)string,"%d",guangzhao);
//		OLED_ShowStr(35,2,string,2);
//		sprintf((char*)string,"%d",guangzhao1);
//		OLED_ShowStr(35,4,string,2);
		
//		printf("%d \r\n" , guangzhao);
		
//		delay_ms(100);
		
		MAX30100_Calculate();
		
		if(main_updata == 1){
			main_updata = 0;
//			printf("\r\n心率：%0.1f	血氧：%d\r\n",x_now,SPO2);
			
			Ds1302_Readtime();
//			printf("hour = %d ,min = %d ,sec = %d\r\n",ds1302Data.hour,ds1302Data.min,ds1302Data.sec);
		}
		

		//	串口1 接受函数
		if(USART1_RX_STA&0x8000){					   
			printf("%s \r\n" , USART1_RX_BUF);
			USART1_RX_STA=0;
			memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
		}
	}
}

void MAX30100_Calculate(void){
	POupdate(&HeartRate,&SPO2);//更新FIFO数据 血氧数据 心率数据
	
	x_mid=x_last;    //x_last=x(k-1|k-1),x_mid=x(k|k-1)					x_mid——上一次
	p_mid=p_last+Q;  //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声			p_mid上一次的预测值
	kg=p_mid/(p_mid+R); //kg为kalman filter，R为噪声
	z_measure=HeartRate;//测量值
	x_now=x_mid+kg*(z_measure-x_mid);//估计出的最优值
	p_now=(1-kg)*p_mid;//最优值对应的covariance
	printf("\r\n心率：%0.1f	血氧：%d\r\n",x_now,SPO2);
	p_last = p_now;  //更新covariance值
	x_last = x_now;  //更新系统状态值
	delay_ms(20);
	
	if((HeartRate > 50)&&(HeartRate < 90)&&(SPO2>90)){	//如果数据符合要求
		MAX30100_Calculate_Flag = 1; 
		if(MAX30100_Success_Count == 3){
			MAX30100_Success_Count = 0;
			sprintf((char*)string,"(#xue_%d_#xin_%d_#wen_100_)",HeartRate,SPO2);
			Usart_SendString(USART2,(char *)string);
		}
	}
	else {
		MAX30100_Calculate_Flag = 0;
		MAX30100_Success_Count = 0;
		
		if(updata_flag == 1){
			updata_flag = 0;
			sprintf((char*)string,"(#xue_0_#xin_0_#wen_0_)");
			Usart_SendString(USART2,(char *)string);
		}
//		MAX30100_Calculate_Flag = 1; 
//		if(MAX30100_Success_Count == 3){
//			MAX30100_Success_Count = 0;
//			sprintf((char*)string,"(#xue_%d_#xin_%d_)",HeartRate,SPO2);
//			Usart_SendString(USART2,(char *)string);
//		}
	}
}

