#include "IAP.h"

//u8 APP_USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000.  
extern "C"
{
	void WFI_SET(void)
	{
		__ASM volatile("wfi");
	}
	//关闭所有中断
	void INTX_DISABLE(void)
	{
		__ASM volatile("cpsid i");
	}
	//开启所有中断
	void INTX_ENABLE(void)
	{
		__ASM volatile("cpsie i");
	}
	//设置栈顶地址
	//addr:栈顶地址
	__asm void MSR_MSP(u32 addr)
	{
		MSR MSP, r0 			//set Main Stack value
			BX r14
	}
};

void IAP_BootLoader::Put_APP(u8 *data, u8 length)
{
	u8 i;
	static u8 res;
	static u16 temp[1];
	static u8 flag = 0;
	static char Last_data;
	static u32 addr = FLASH_APP1_ADDR;
	Update_flag = true;
	for (i = 0; i < length; i++)
	{
		//		APP_USART_RX_BUF[RecvIndex++] = data[i];
		if (RecvIndex++ == 146){
			RecvIndex = RecvIndex;
		}
		res = data[i + 2];
		if (flag == 0){
			flag = 1;
			Last_data = res;
			temp[0] = 0;
		}
		else{
			flag = 0;
			temp[0] = (u16)res << 8;
			temp[0] += (u16)Last_data;
			STMFLASH_Write(addr, temp, 1);
			//			FLASH_ProgramHalfWord(addr,temp[0]);
			addr += 0x02;
		}
	}
}

extern "C"
{
	void temp(u32 appxaddr)
	{
		iapfun jump2app;
		if (((*(vu32*)appxaddr) & 0x2FFE0000) == 0x20000000)	//检查栈顶地址是否合法.
		{
			jump2app = (iapfun)*(vu32*)(appxaddr + 4);		//用户代码区第二个字为程序开始地址(复位地址)		
			MSR_MSP(*(vu32*)appxaddr);										//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
			jump2app();																		//跳转到APP.
		}
	}
}
void IAP_BootLoader::iap_load_app(u32 appxaddr)															//执行flash里面的app程序
{
	temp(appxaddr);
}

void IAP_BootLoader::iap_load_appsram(u32 appxaddr)													//执行sram里面的app程序
{
	/////////////////////////////////////////////////////
}

void IAP_BootLoader::iap_write_appbin(u32 appxaddr, u8 *appbuf, u32 appsize)	//在指定地址开始,写入bin
{
	u16 t;
	u16 iapbuf[1024];
	u16 i = 0;
	u16 temp;
	u32 fwaddr = appxaddr;//当前写入的地址
	u8 *dfu = appbuf;
	for (t = 0; t < appsize; t += 2)
	{
		temp = (u16)dfu[1] << 8;
		temp += (u16)dfu[0];
		dfu += 2;//偏移2个字节
		iapbuf[i++] = temp;
		if (i == 1024)
		{
			i = 0;
			STMFLASH_Write(fwaddr, iapbuf, 1024);
			fwaddr += 2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if (i)
	{
		STMFLASH_Write(fwaddr, iapbuf, i);//将最后的一些内容字节写进去.
	}
}

u16 IAP_BootLoader::STMFLASH_ReadHalfWord(u32 faddr)		  //读出半字  
{
	return *(vu16*)faddr;
}

void IAP_BootLoader::STMFLASH_WriteLenByte(u32 WriteAddr, u32 DataToWrite, u16 Len)	//指定地址开始写入指定长度的数据
{

}

u32 IAP_BootLoader::STMFLASH_ReadLenByte(u32 ReadAddr, u16 Len)											//指定地址开始读取指定长度数据
{
	return 0;
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{
	u16 i;
	for (i = 0; i < NumToWrite; i++)
	{
		FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
		WriteAddr += 2;//地址增加2.
	}
}
#endif
void IAP_BootLoader::STMFLASH_Write(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)			//从指定地址开始写入指定长度的数据
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
	u16 i;
	u32 offaddr;   //去掉0X08000000后的地址
	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE + 1024 * STM32_FLASH_SIZE)))return;//非法地址
	FLASH_Unlock();						//解锁
	offaddr = WriteAddr - STM32_FLASH_BASE;			//实际偏移地址.
	secpos = offaddr / STM_SECTOR_SIZE;					//扇区地址  0~127 for STM32F103RBT6
	secoff = (offaddr%STM_SECTOR_SIZE) / 2;			//在扇区内的偏移(2个字节为基本单位.)
	secremain = STM_SECTOR_SIZE / 2 - secoff;		//扇区剩余空间大小   
	if (NumToWrite <= secremain)secremain = NumToWrite;//不大于该扇区范围
	while (1)
	{
		STMFLASH_Read(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);//读出整个扇区的内容
		for (i = 0; i < secremain; i++)//校验数据
		{
			if (STMFLASH_BUF[secoff + i] != 0XFFFF)break;//需要擦除  	  
		}
		if (i < secremain)//需要擦除
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE);//擦除这个扇区
			for (i = 0; i<secremain; i++)//复制
			{
				STMFLASH_BUF[i + secoff] = pBuffer[i];
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2);//写入整个扇区  
			//			printf("download flash\r\n");
		}
		else STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if (NumToWrite == secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff = 0;				//偏移位置为0 	 
			pBuffer += secremain;  	//指针偏移
			WriteAddr += secremain * 2;	//写地址偏移	secremain = STM_SECTOR_SIZE / 2 - secoff;	这里除了一个2
			NumToWrite -= secremain;	//字节(16位)数递减
			if (NumToWrite>(STM_SECTOR_SIZE / 2))secremain = STM_SECTOR_SIZE / 2;//下一个扇区还是写不完
			else secremain = NumToWrite;//下一个扇区可以写完了
		}
	};
	FLASH_Lock();//上锁
}

void IAP_BootLoader::STMFLASH_Read(u32 ReadAddr, u16 *pBuffer, u16 NumToRead)   			//从指定地址开始读出指定长度的数据
{
	u16 i;
	for (i = 0; i < NumToRead; i++)
	{
		pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr += 2;//偏移2个字节.	
	}
}

