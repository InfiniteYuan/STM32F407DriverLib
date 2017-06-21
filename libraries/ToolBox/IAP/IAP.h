#ifndef _IAP_H

#define _IAP_H
#include "stm32f4xx.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
//用户根据自己的需要设置
#define STM32_FLASH_SIZE 64 	 					//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1              //使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址
//FLASH解锁键值

//////////////////////////////////////////////////////////////////////////////////	 
extern "C"{
	typedef  void(*iapfun)(void);				//定义一个函数类型的参数.
}

#define FLASH_APP1_ADDR		0x08008000  	//第一个应用程序起始地址(存放在FLASH)
//保留0X08000000~0X08000FFF的空间为IAP使用

#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //字节
#else 
#define STM_SECTOR_SIZE	2048
#endif

#define USART_REC_LEN  		10*1024 //定义最大接收字节数 15K 
extern u8 APP_USART_RX_BUF[USART_REC_LEN];

class IAP_BootLoader{

private:
	u16 STMFLASH_BUF[STM_SECTOR_SIZE / 2];//最多是2K字节

public:
	u32 RecvIndex;
	bool Update_flag;
	IAP_BootLoader(u8 i)
	{
		Update_flag = false;
		RecvIndex = 0;
	}
	void Put_APP(u8 *data, u8 length);

	void iap_load_app(u32 appxaddr);															//执行flash里面的app程序
	void iap_load_appsram(u32 appxaddr);													//执行sram里面的app程序
	void iap_write_appbin(u32 appxaddr, u8 *appbuf, u32 appsize);	//在指定地址开始,写入bin

	u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
	void STMFLASH_WriteLenByte(u32 WriteAddr, u32 DataToWrite, u16 Len);	//指定地址开始写入指定长度的数据
	u32 STMFLASH_ReadLenByte(u32 ReadAddr, u16 Len);											//指定地址开始读取指定长度数据
	void STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite);			//从指定地址开始写入指定长度的数据
	void STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead);   			//从指定地址开始读出指定长度的数据

};
#endif
