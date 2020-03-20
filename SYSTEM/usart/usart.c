#include "sys.h"
#include "usart.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE { 
	int handle; 
}; 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) { 
	x = x; 
} 

//USART1    �ض���fputc���� 
int fputc(int ch, FILE *f){      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

 
#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART1_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
u8 USART2_RX_BUF[USART_REC_LEN];
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART1_RX_STA=0;       //����״̬���	  
u16 USART2_RX_STA=0;       //����״̬���	
u16 len ,t;
void uart1_init(u32 bound){
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9

	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  
	
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}

void uart2_init(u32 bound){
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	//Tx �� �������� ���
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//Rx �� ���� ����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������1
	
	USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);	
}

void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
		if((USART1_RX_STA&0x8000)==0){//����δ���
			if(USART1_RX_STA&0x4000){//���յ���0x0d
				if(Res!=0x0a)USART1_RX_STA=0;//���մ���,���¿�ʼ
				else USART1_RX_STA|=0x8000;	//��������� 
			}
			else {//��û�յ�0X0D	
				if(Res==0x0d)USART1_RX_STA|=0x4000;
				else{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res ;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART_REC_LEN-1))USART1_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}	
			}				
		}
	}   		 
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
#include "NetWork.h"
extern uint8_t RXBuffer[RXBUFFER_LEN];

void USART2_IRQHandler(void)
{
    static u8 i = 0;

    if(USART_GetITStatus(USART2, USART_IT_RXNE)) //�жϽ������ݼĴ����Ƿ�������
    {
        RXBuffer[i++] = USART_ReceiveData(USART2);
        if(i==RXBUFFER_LEN)                     //�������ջ��巶Χ,���ܵ�������ESP8266��λ,Ϊ��ֹ���������������                  
        {
            i = RXBUFFER_LEN-1;
        }
    }

    if(USART_GetITStatus(USART2, USART_IT_IDLE))
    {
        USART_ReceiveData(USART2);              //��һ��UART����������б�־λ
//		printf("RXBuffer = %s\r\n",RXBuffer);
        i = 0;
//      processServerBuffer();
//		memset(RXBuffer,0,sizeof(RXBuffer));
    } 
}

void sendString(USART_TypeDef *USARTx, char *str)
{
    while (*str)
    {
        sendByte(USARTx,*str++);
    }
}

void sendByte(USART_TypeDef *USARTx, u16 byte)
{
	USART_ClearFlag(USARTx, USART_FLAG_TC);             //������������ɱ�־λ
    USART_SendData(USARTx, byte);                       //����һ���ֽ�
    while (!USART_GetFlagStatus(USARTx, USART_FLAG_TC));//�ȴ��������
    USART_ClearFlag(USARTx, USART_FLAG_TC);             //������������ɱ�־λ
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//����һ���ֽ�
void Usart_SendChar(USART_TypeDef * pUSARTx,  uint8_t ch){
	/* ����һ���ֽ����ݵ� USART */
	USART_SendData (pUSARTx ,ch);
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}
//����һ���ַ���
void Usart_SendString(USART_TypeDef * pUSARTx, char *str){
	unsigned int k=0;
	do {
		Usart_SendChar( pUSARTx, *(str + k) );
		k++;
	} while (*(str + k)!='\0');
}

  
#endif	

//		//������     ����1 ���ܺ���
//		if(USART1_RX_STA&0x8000){					   
//			printf("%s \r\n" , USART1_RX_BUF);
//			USART1_RX_STA=0;
//			memset(USART1_RX_BUF,0,sizeof(USART1_RX_BUF));
//		}

//void USART1_IRQHandler(void)                	//����1�жϷ������
//{
//	u8 Res;
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//	{
//		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
////		printf("%c\r\n",Res);
//		if((USART1_RX_STA&0x8000)==0){//����δ���
//			if(USART1_RX_STA&0x4000){//���յ���0x0d
//				if(Res!=0x0a)USART1_RX_STA=0;//���մ���,���¿�ʼ
//				else USART1_RX_STA|=0x8000;	//��������� 
//			}
//			else {//��û�յ�0X0D	
//				if(Res==0x0d)USART1_RX_STA|=0x4000;
//				else{
//					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res ;
//					USART1_RX_STA++;
//					if(USART1_RX_STA>(USART_REC_LEN-1))USART1_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//				}	
//			}				
//		}
//	}   		 
//}
//void USART2_IRQHandler(void)                	//����1�жϷ������
//{
//	u8 Res;
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//	{
//		Res =USART_ReceiveData(USART2);	//��ȡ���յ�������
//		
//		if((USART2_RX_STA&0x8000)==0){//����δ���
//			if(USART2_RX_STA&0x4000){//���յ���0x0d
//				if(Res!=0x0a)USART2_RX_STA=0;//���մ���,���¿�ʼ
//				else USART2_RX_STA|=0x8000;	//��������� 
//			}
//			else //��û�յ�0X0D{	
//				if(Res==0x0d)USART2_RX_STA|=0x4000;
//				else{
//					USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res ;
//					USART2_RX_STA++;
//					if(USART2_RX_STA>(USART_REC_LEN-1))USART2_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//				}		 
//			}
//		}   		 
//} 
////�������ַ�����  
//void USART1_IRQHandler(void)                	//����1�жϷ������
//{
//	u8 Res;
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//	{
//		Res =USART_ReceiveData(USART1);	//��ȡ���յ�������
//		if((USART1_RX_STA&0x8000)==0){//����δ���
//			if(Res==0x3b)USART1_RX_STA|=0x8000;
//			else{
//				USART1_RX_BUF[USART1_RX_STA&0X3FFF]=Res ;
//				USART1_RX_STA++;
//				if(USART1_RX_STA>(USART_REC_LEN-1))USART1_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//			}			
//		}
//	}   		 
//} 

////	����1 ���ܺ���
//		if(USART1_RX_STA&0x8000){					   
//			len=USART1_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//			for(t=0;t<len;t++){
//				USART_SendData(USART1, USART1_RX_BUF[t]);//�򴮿�1��������
//				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//			}
//			USART1_RX_STA=0;
//		}

////	����2 ���ܺ���
//		if(USART2_RX_STA&0x8000){					   
//			len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
//			for(t=0;t<len;t++){
//				USART_SendData(USART2, USART2_RX_BUF[t]);//�򴮿�1��������
//				while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
//			}
//			USART2_RX_STA=0;
//			Usart_SendString(USART2,"\r\n");
//		}

