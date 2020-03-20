#include "ds1302.h"
#include "delay.h"

struct DS1302DATA ds1302Data = {0,0,0,0,0,0,0};  
u8 ascii_time[7] = {0};     //����ascii��ʽ����  
u8 bcd_time[7] = {0};       //����bcd������  
  
static u8 AsciiToBcd(u8 asciiData)  
{  
    u8 bcdData = 0;  
    bcdData = (((asciiData/10)<<4)|((asciiData%10)));  
    return bcdData;  
}

static u8 BcdToAscii(u8 bcdData)  
{  
    u8 asciiData = 0;  
    asciiData = (((bcdData&0xf0)>>4)*10 + (bcdData&0x0f));  
    return asciiData;  
}  
  
//IO��ʼ��
void Ds1302_Gpio_Init(void)  
{     
    //RST  //�������C0
        RCC->APB2ENR|=1<<3;//ʹ��PORTCʱ�� 
        GPIOB->CRL&=0xFFFFFFF0;        //C0�������
        GPIOB->CRL|=0x00000003;
    //CLK  //�������A5
        RCC->APB2ENR|=1<<2;//ʹ��PORTAʱ�� 
        GPIOA->CRL&=0xFF0FFFFF;        //A5�������
        GPIOA->CRL|=0x00300000;  
    //IO  //�������A8
        RCC->APB2ENR|=1<<2;//ʹ��PORTAʱ�� 
        GPIOA->CRH&=0xFFFFFFF0;        //A8�������
        GPIOA->CRH|=0x00000003;
}  
  
//��ȡһ���ֽڵ�ʱ�����½��ؽ��ж�
u8 Ds1302_ReadByte(void)  
{  
    u8 i = 0, dat = 0;  
    DS1302_DAT_INPUT();  
    delay_us(5);  
    for(i = 0; i <8; i++)  
    {  
        dat >>= 1;  
        if(DS1302_DATIN == 1)dat |= 0x80;//�͵�ƽ��ȡ  
        DS1302_CLK = 1;//�����߻����һ�ֽ�����  
        delay_us(2);  
        DS1302_CLK = 0;  
        delay_us(2);  
    }  
    return dat;  
}  
  
//д��һ���ֽڵ�ʱ���������ؽ���д
void Ds1302_WriteByte(u8 dat)  
{  
    u8 i = 0, data = dat;  
    DS1302_DAT_OUTPUT();   
    DS1302_CLK = 0;  
    delay_us(2);  
    for(i = 0; i < 8; i++)  
    {  
        DS1302_DATOUT = data&0x01;  
        delay_us(2);//��ʱ�ȴ��͵�ƽ�ȶ�  
        DS1302_CLK = 1;//����д��  
        delay_us(2);  
        DS1302_CLK = 0;  
        data >>= 1;  
    }  
}  
  
//д��һ���Ĵ���һ���ֽ�����
void Ds1302_Write(u8 address,u8 dat)  
{  
    DS1302_RST = 0;  
    DS1302_CLK = 0;  //ֻ����clk�͵�ƽʱ������rst������
    DS1302_RST = 1;  
    Ds1302_WriteByte(address);  
    Ds1302_WriteByte(dat);  
    DS1302_CLK = 1;  
    DS1302_RST = 0;  
} 

//��ȡһ���ֽ�
u8 Ds1302_Read(u8 address)  
{  
    u8 data = 0;  
    DS1302_RST = 0;  
    DS1302_CLK = 0; //ֻ����clk�͵�ƽʱ������rst������
    DS1302_RST = 1;  
    Ds1302_WriteByte(address|0x01); //��ȡ��ַ��Ҫ��0x01������Ϊ���1  
    data = Ds1302_ReadByte();  
    DS1302_CLK = 1;  
    DS1302_RST = 0;  
    return data;  
}  
  
//����д��ʱ��
void Ds1302_Write_Time_Singel(u8 address,u8 dat)  
{
    Ds1302_Write(DS1302_CONTROL_REG,0x00);  //ȡ��д����  
    Ds1302_Write(address,dat);  
    Ds1302_Write(DS1302_CONTROL_REG,0x80);  //��д����  
}  
  
//һ���������ʱ��ĸ���
//start��ǰʱ�����л���ֹͣ
void Ds1302_Write_Time_All(u8 start)  
{  
    Ds1302_Write(DS1302_CONTROL_REG,0x00);      //ȡ��д���� 
    Ds1302_Write(DS1302_SEC_REG,(AsciiToBcd(ds1302Data.sec)|start));  
    Ds1302_Write(DS1302_MIN_REG,AsciiToBcd(ds1302Data.min));  
    Ds1302_Write(DS1302_HR_REG,AsciiToBcd(ds1302Data.hour));  
    Ds1302_Write(DS1302_DATE_REG,AsciiToBcd(ds1302Data.day));  
    Ds1302_Write(DS1302_MONTH_REG,AsciiToBcd(ds1302Data.month));  
    Ds1302_Write(DS1302_DAY_REG,AsciiToBcd(ds1302Data.week));  
    Ds1302_Write(DS1302_YEAR_REG,AsciiToBcd(ds1302Data.year));  
    Ds1302_Write(DS1302_CONTROL_REG,0x80);  //��д����  
}  

//��ȡʵʱ���ʱ��Ĭ����ʱ��������  
void Ds1302_Readtime(void)  
{  
    ds1302Data.sec = BcdToAscii(Ds1302_Read(DS1302_SEC_REG));  //�� 
    ds1302Data.min = BcdToAscii(Ds1302_Read(DS1302_MIN_REG));  //�� 
    ds1302Data.hour = BcdToAscii(Ds1302_Read(DS1302_HR_REG));   //Сʱ  
    ds1302Data.day = BcdToAscii(Ds1302_Read(DS1302_DATE_REG)); //��
    ds1302Data.month = BcdToAscii(Ds1302_Read(DS1302_MONTH_REG)); //��  
    ds1302Data.week = BcdToAscii(Ds1302_Read(DS1302_DAY_REG));  //���ڼ� 
    ds1302Data.year = BcdToAscii(Ds1302_Read(DS1302_YEAR_REG)); //��
} 
