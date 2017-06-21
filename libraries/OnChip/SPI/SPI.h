#ifndef _SPI_H
#define _SPI_H


#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "FIFOBuffer.h"


/******************************************************************************************************/
/****configuration  使用前自行配置****/
#define SPI_TX_BUFFER_SIZE     200              //SPI BUFFER FIFO SIZE
#define SPI_RX_BUFFER_SIZE     200              //SPI BUFFER FIFO SIZE
#define SPI_DMA_TX_BUFFER_SIZE 200              //SPI DMA BUFFER SIZE

/*******************************************************************************************************/

typedef enum
{
	SPI_RXNE_IRQ,				//SPI接收缓存区非空中断
	SPI_TXE_IRQ,				//SPI发送缓冲区空中断
	SPI_ERR_IRQ					//SPI错误中断
} SPIIrqType;

typedef enum
{
	DMA_HT_IRQ,					//DMA传输过半中断
	DMA_TC_IRQ,					//DMA传输完成中断
	DMA_TE_IRQ					//DMA错误中断
} DMAIrqType;

class SPI{

private:

	FIFOBuffer<u8, SPI_TX_BUFFER_SIZE>  SPIbufferTx;
	FIFOBuffer<u8, SPI_RX_BUFFER_SIZE>  SPIbufferRx;
	u8 bufferTxDma[SPI_DMA_TX_BUFFER_SIZE];
	u32  dmaChannel;				//dma通道
	DMA_Stream_TypeDef*  mDMA_Streamx;//DMA data stream
	uint32_t dmaTCFlagChannel;								//dma传输完成中断标志位
	uint32_t dmaGLFlagChannel;								//dma全局中断标志位
	uint32_t dmaTEFlagChannel;								//dma错误中断标志位
	bool isBusySend;
	bool mUseDma;
	unsigned char mPrecision;
	/*-------SPI Pin Configuration------*/
	SPI_TypeDef*	 SPIx;
	u32            mGPIORcc;  				//GPIO Clock
	u16            mMISOPinSource;    //MISO gpio pinSource
	u16            mMOSIPinSource;    //MOSI gpio pinSource
	u16            mSCKPinSource;     //MOSI gpio pinSource
	u8             mGPIOAF;   				//GPIO AF

	void RCC_Configuration(void);

public:

	SPI(SPI_TypeDef* SPI, bool useDMA = false, u8 remap = 0, u8 Prioritygroup = 3, uint8_t preemprionPriority = 7, uint8_t subPriority = 1, u8 dmaPriority = 3);

	u8 SPI_RW(u8 dat);
	bool SendData(uint8_t *pbuffer, uint32_t size);
	bool GetReceivedData(u8* buffer, u16 number);

	void SetSpeed(u8 SPI_BaudRatePrescaler);

	u16 ReceiveBufferSize();
	u16 SendBufferSize();

	void ClearReceiveBuffer();
	void ClearSendBuffer();

	SPI_TypeDef* getSPI();
	bool UseDma();

	SPIIrqType SpiIrq();
	DMAIrqType DmaIrq();

	void SetCSNPin(u8 value);

};


#endif

