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

extern __IO uint16_t ADC_ConvertedValue[NOFCHANEL];// ADCת���ĵ�ѹֵͨ��MDA��ʽ����SRAM
float ADC_ConvertedValueLocal[NOFCHANEL];  // �ֲ����������ڱ���ת�������ĵ�ѹֵ 	  

uint8_t HeartRate,SPO2;
uint8_t string[100];
uint8_t MAX30100_HeartRate_Buf[200];//�����������ݣ�����Ϊ���ݿ��ܻ���һ���β���ȷ
uint8_t MAX30100_SPO2_Buf[200];
uint8_t	MAX30100_Shang[100];

void MAX30100_Calculate(void);
uint8_t MAX30100_Calculate_Flag = 0;
uint8_t	MAX30100_Success_Count = 0;
uint8_t	MAX30100_Fail_Count = 0;
int i = 0,j = 0;


float x_last=0;  //�������˲�
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
	
	
	delay_init();	    	//��ʱ������ʼ��	  
	uart1_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	uart2_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	LED_Init();				//��ʼ�� LED
	Timer2_Init(1000,72);	//��ʼ�� Timer2 1ms �ж�һ��
	TIM3_Int_Init(100-1,720-1);//1ms
//	Key_Init();
	IIC_OLED_Init();		//OLED IIC ��ʼ��
	OLED_Init();			//OLED ��ʼ��
	Ds1302_Gpio_Init();
	ADCx_Init();		  		//ADC��ʼ��
	
	IIC_Init();				//Ѫ��ģ��
	SPO2_Init();
	
	nvic_init();			//���� �жϵ� ���ȼ� �� �ų�ʼ�� ����
	
	printf("CPU Start !\r\n");
	
	OLED_ON();OLED_CLS();	OLED_CLS();OLED_CLS();//OLED ����
	OLED_ShowCHinese(0,2,0);OLED_ShowCHinese(16,2,1);OLED_ShowCHinese(32,2,2);
	OLED_ShowCHinese(48,2,3);OLED_ShowCHinese(64,2,4);OLED_ShowCHinese(80,2,5);
	OLED_ShowCHinese(96,2,6);//OLED��ʾ ������ʼ������			��ϵͳ���ڳ�ʼ����
	
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
	
	delay_precise_100ms(50);//��ȷ��ʱ	x*100ms

	
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
//			printf("\r\n���ʣ�%0.1f	Ѫ����%d\r\n",x_now,SPO2);
			
			Ds1302_Readtime();
//			printf("hour = %d ,min = %d ,sec = %d\r\n",ds1302Data.hour,ds1302Data.min,ds1302Data.sec);
		}
		

		//	����1 ���ܺ���
		if(USART1_RX_STA&0x8000){					   
			printf("%s \r\n" , USART1_RX_BUF);
			USART1_RX_STA=0;
			memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
		}
	}
}

void MAX30100_Calculate(void){
	POupdate(&HeartRate,&SPO2);//����FIFO���� Ѫ������ ��������
	
	x_mid=x_last;    //x_last=x(k-1|k-1),x_mid=x(k|k-1)					x_mid������һ��
	p_mid=p_last+Q;  //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=����			p_mid��һ�ε�Ԥ��ֵ
	kg=p_mid/(p_mid+R); //kgΪkalman filter��RΪ����
	z_measure=HeartRate;//����ֵ
	x_now=x_mid+kg*(z_measure-x_mid);//���Ƴ�������ֵ
	p_now=(1-kg)*p_mid;//����ֵ��Ӧ��covariance
	printf("\r\n���ʣ�%0.1f	Ѫ����%d\r\n",x_now,SPO2);
	p_last = p_now;  //����covarianceֵ
	x_last = x_now;  //����ϵͳ״ֵ̬
	delay_ms(20);
	
	if((HeartRate > 50)&&(HeartRate < 90)&&(SPO2>90)){	//������ݷ���Ҫ��
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

