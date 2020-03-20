#ifndef __OLED_H
#define __OLED_H

#include "sys.h"
 
#define READ_OLED_SDA  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)

#define IIC_OLED_SDA PBout(7)// PB5
#define IIC_OLED_SCL PBout(6)// PE5	


void IIC_OLED_Init(void);
void SDA_OLED_OUT(void);
void SDA_OLED_IN(void);
void IIC_OLED_Start(void);
void IIC_OLED_Stop(void);
u8 IIC_OLED_Wait_Ask(void);
void IIC_OLED_WriteByte(u8 data);
u8 IIC_OLED_ReadByte(void);
void WriteCmd(u8 command);
void WriteDat(u8 data);

void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_Init(void);
void OLED_ON(void);
void OLED_SetPos(unsigned char x, unsigned char y); //设置起始点坐标
void OLED_Fill(unsigned char fill_Data);//全屏填充
void OLED_CLS(void);//清屏
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);


#endif
