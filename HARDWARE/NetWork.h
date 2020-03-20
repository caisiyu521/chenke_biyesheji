/******************************************************************
 * 文件：NetWork.c
 * 功能：声明TCP、UDP通信相关函数
 * 日期：2018-04-06
 * 作者：zx
 * 版本：Ver.1.0 | 最初版本
 * 
 * Copyright (C) 2018 zx. All rights reserved.
*******************************************************************/
#ifndef __NETWORK_H
#define __NETWORK_H

#include "sys.h"
#include "usart.h"
#include "delay.h"

/*连接AP宏定义*/
//#define SSID "iPhone"
//#define PWD  "12345678--"
#define SSID "ipone"
#define PWD  "123456321"

/*连接服务器宏定义*/
#define TCP "TCP"
#define UDP "UDP"
//#define IP  "172.20.10.5"//花生壳 IP	caisiyu.wicp.vip:56695
//#define PORT 56695
//#define IP  "103.46.128.43"//花生壳 IP	2u7919q843.zicp.vip:11967
//#define PORT 56695
//#define IP  "10.101.77.111"//本地 ：内网主机
//#define PORT 8888

#define IP  "192.168.0.104"//花生壳 IP	2u7919q843.zicp.vip:11967
#define PORT 12345


/*发送接收缓冲区长度宏定义*/
#define TXBUFFER_LEN 50
#define RXBUFFER_LEN 30

u8 checkESP8266(void);
u8 initESP8266(void);
void restoreESP8266(void);
u8 connectAP(u8* ssid,u8* pwd);
u8 connectServer(u8* mode,u8* ip,u16 port);
void sendBuffertoServer(u8* buffer);
void processServerBuffer(void);
u8 disconnectServer(void);

#endif
