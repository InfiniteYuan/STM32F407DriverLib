#include "SPI.h"
#include "stm32f4xx.h"
#include "USART.h"
#include "Interrupt.h"

SPI::SPI(SPI_TypeDef* SPI, bool useDMA, u8 remap, u8 Prioritygroup, uint8_t preemprionPriority, uint8_t subPriority, u8 dmaPriority)
{
	SPIx = SPI;
	isBusySend = 0;
	mUseDma = useDMA;
	GPIO_TypeDef* GPIOx;
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	uint16_t SCKPin = 0, CSNPin = 0, MISOPin = 0, MOSIPin = 0;
	uint8_t spiIrqChannel, dmaIrqChannel;//中断通道，SPI DMA通道

	//*RCC_Configuration*//
	RCC_Configuration();

	/*GPIO Configuration*/
	if (SPIx == SPI1){
		GPIOx = (remap ? GPIOB : GPIOA);
		mGPIOAF = GPIO_AF_SPI1;
		spiIrqChannel = SPI1_IRQn;
		mGPIORcc = (remap ? RCC_AHB1Periph_GPIOB : RCC_AHB1Periph_GPIOA);
		CSNPin = (remap ? GPIO_Pin_15 : GPIO_Pin_4);
		GPIO_InitStructure.GPIO_Pin = CSNPin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOx, &GPIO_InitStructure);

		SCKPin = (remap ? GPIO_Pin_3 : GPIO_Pin_5);
		MISOPin = (remap ? GPIO_Pin_4 : GPIO_Pin_6);
		MOSIPin = (remap ? GPIO_Pin_5 : GPIO_Pin_7);
		mSCKPinSource = (remap ? GPIO_PinSource3 : GPIO_PinSource5);
		mMISOPinSource = (remap ? GPIO_PinSource4 : GPIO_PinSource6);
		mMOSIPinSource = (remap ? GPIO_PinSource5 : GPIO_PinSource7);

#ifdef USE_SPI1_DMA
		dmaChannel = DMA_Channel_3;
		mDMA_Streamx = DMA2_Stream3;
		dmaIrqChannel = DMA2_Stream3_IRQn;
		dmaGLFlagChannel = DMA_IT_TCIF3 | DMA_IT_HTIF3 | DMA_IT_TEIF3 | DMA_IT_FEIF3;
		dmaTCFlagChannel = DMA_IT_TCIF3;
		dmaTEFlagChannel = DMA_IT_TEIF3;
		pSPI1 = this;
#endif

#ifdef USE_SPI1
		pSPI1 = this;
#endif
	}
	else if (SPIx == SPI2){
		GPIOx = GPIOB;
		mGPIOAF = GPIO_AF_SPI2;
		spiIrqChannel = SPI2_IRQn;
		mGPIORcc = RCC_AHB1Periph_GPIOB;
		CSNPin = (remap ? GPIO_Pin_9 : GPIO_Pin_12);
		GPIO_InitStructure.GPIO_Pin = CSNPin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOx, &GPIO_InitStructure);

		SCKPin = (remap ? GPIO_Pin_10 : GPIO_Pin_13);
		MISOPin = (remap ? GPIO_Pin_2 : GPIO_Pin_14);
		MOSIPin = (remap ? GPIO_Pin_3 : GPIO_Pin_15);
		mSCKPinSource = (remap ? GPIO_PinSource10 : GPIO_PinSource13);
		mMISOPinSource = (remap ? GPIO_PinSource2 : GPIO_PinSource14);
		mMOSIPinSource = (remap ? GPIO_PinSource3 : GPIO_PinSource15);

#ifdef USE_SPI2_DMA
		dmaChannel = DMA_Channel_0;
		mDMA_Streamx = DMA1_Stream4;
		dmaIrqChannel = DMA1_Stream4_IRQn;
		dmaGLFlagChannel = DMA_IT_TCIF4 | DMA_IT_HTIF4 | DMA_IT_TEIF4 | DMA_IT_FEIF4;
		dmaTCFlagChannel = DMA_IT_TCIF4;
		dmaTEFlagChannel = DMA_IT_TEIF4;
		pSPI2 = this;
#endif

#ifdef USE_SPI2
		pSPI2 = this;
#endif
	}
	else if (SPIx == SPI3){
		GPIOx = GPIOA;
		mGPIOAF = GPIO_AF_SPI3;
		spiIrqChannel = SPI3_IRQn;
		mGPIORcc = RCC_AHB1Periph_GPIOA;
		CSNPin = (remap ? GPIO_Pin_15 : GPIO_Pin_4);
		GPIO_InitStructure.GPIO_Pin = CSNPin;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_Init(GPIOx, &GPIO_InitStructure);

		GPIOx = (remap ? GPIOC : GPIOB);
		mGPIORcc = (remap ? RCC_AHB1Periph_GPIOC : RCC_AHB1Periph_GPIOB) | RCC_AHB1Periph_GPIOA;
		SCKPin = (remap ? GPIO_Pin_10 : GPIO_Pin_3);
		MISOPin = (remap ? GPIO_Pin_11 : GPIO_Pin_4);
		MOSIPin = (remap ? GPIO_Pin_12 : GPIO_Pin_5);
		mSCKPinSource = (remap ? GPIO_PinSource10 : GPIO_PinSource3);
		mMISOPinSource = (remap ? GPIO_PinSource11 : GPIO_PinSource4);
		mMOSIPinSource = (remap ? GPIO_PinSource12 : GPIO_PinSource5);

#ifdef USE_SPI3_DMA
		dmaChannel = DMA_Channel_0;
		mDMA_Streamx = DMA1_Stream5;
		dmaIrqChannel = DMA1_Stream5_IRQn;
		dmaGLFlagChannel = DMA_IT_TCIF5 | DMA_IT_HTIF5 | DMA_IT_TEIF5 | DMA_IT_FEIF5;
		dmaTCFlagChannel = DMA_IT_TCIF5;
		dmaTEFlagChannel = DMA_IT_TEIF5;
		pSPI3 = this;
#endif

#ifdef USE_SPI3
		pSPI3 = this;
#endif
	}

	RCC_AHB1PeriphClockCmd(mGPIORcc, ENABLE);

	/*  GPIO Init  SCKPin MOSIPin MISOPin*/
	GPIO_InitStructure.GPIO_Pin = SCKPin | MOSIPin | MISOPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOx, &GPIO_InitStructure);

	//*********************
	GPIO_PinAFConfig(GPIOx, mSCKPinSource, mGPIOAF); 	//AF SPI
	GPIO_PinAFConfig(GPIOx, mMISOPinSource, mGPIOAF); //AF SPI
	GPIO_PinAFConfig(GPIOx, mMOSIPinSource, mGPIOAF); //AF SPI

	GPIO_SetBits(GPIOx, SCKPin | MOSIPin | MISOPin);
	//CE IRQ Pin 没有配置

	/**SPI Configuration**/
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_Init(SPIx, &SPI_InitStructure);
	SPI_Cmd(SPIx, ENABLE);

	if (mUseDma)
	{
		SPI_I2S_DMACmd(SPIx, SPI_I2S_DMAReq_Tx, ENABLE);
		DMA_InitTypeDef DMA_InitStructure;
		/*开启DMA时钟*/
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_DMA2, ENABLE);
		//设置DMA源，SPI数据寄存器地址
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPIx->DR;
		//内存地址（要传输的变量指针）
		DMA_InitStructure.DMA_Memory0BaseAddr = (u32)bufferTxDma;/*dmaChannelTx->CMAR =  (u32)bufferTxDma;*/
		//方向：从内存到外设
		DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
		//传输DMA_BufferSize 初始值设置为1
		DMA_InitStructure.DMA_BufferSize = 1;
		//外设地址不增
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		//内存地址自增
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		//外设数据单位
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		//外设数据单位8bit
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		//DMA模式：不循环
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;         //DMA priority: low
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

		switch (dmaPriority){
		case 0:
			DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
			break;
		case 1:
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			break;
		case 2:
			DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
			break;
		default:
			DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
			break;
		}
		//配置DMA1的通道 Tx通道
		DMA_Init(mDMA_Streamx, &DMA_InitStructure);
		//使能DMA
		DMA_Cmd(mDMA_Streamx, DISABLE);
		//打开传输完成中断，打开错误中断
		DMA_ITConfig(mDMA_Streamx, DMA_IT_TC | DMA_IT_TE, ENABLE);
	}
	else {
		//使能SPI 接收缓冲区非空，和发送缓冲区空，错误中断
		SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE | SPI_I2S_IT_ERR, ENABLE);
		SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);
		//清除标志
		SPI_I2S_ClearFlag(SPIx, SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_RXNE);
		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_RXNE);
		SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE | SPI_I2S_IT_RXNE | SPI_I2S_IT_ERR);
	}

	//中断优先级设置
	NVIC_InitTypeDef NVIC_InitStructure;
	switch (Prioritygroup){
	case 0:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
		break;
	case 1:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		break;
	case 2:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		break;
	default:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
		break;
	case 4:
		NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
		break;
	}

	/******************SPI 中断注册************************/
	NVIC_InitStructure.NVIC_IRQChannel = spiIrqChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemprionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/******************DMA 中断注册************************/
#ifdef USE_SPI_DMA
	NVIC_InitStructure.NVIC_IRQChannel = dmaIrqChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemprionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

void SPI::SetSpeed(u8 SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPIx->CR1 &= 0XFFC7;
	SPIx->CR1 |= SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPIx, ENABLE);
}

void SPI::RCC_Configuration()
{
	/**-------RCC Configuration--------**/
	if (SPIx == SPI1){
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	}
	else if (SPIx == SPI2){
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	}
	else if (SPIx == SPI3){
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
	}
}

u8 SPI::SPI_RW(u8 dat)
{
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET); //当 SPI发送缓冲器非空时等待 
	SPI_I2S_SendData(SPIx, dat);	                                  //通过 SPIx发送一字节数据	
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);//当SPI接收缓冲器为空时等待
	return SPI_I2S_ReceiveData(SPIx);                               //Return the byte read from the SPI bus
}

bool SPI::SendData(uint8_t *pbuffer, uint32_t size)
{
	isBusySend = false;
	SPIbufferTx.Puts(pbuffer, size);//将发送的数据放入发送缓冲区队列
	if (mUseDma && !isBusySend){//使用DMA发送数据
		if (SPIbufferTx.Size() > 0){
			isBusySend = true;
			if (SPIbufferTx.Size() <= SPI_DMA_TX_BUFFER_SIZE){
				mDMA_Streamx->NDTR = SPIbufferTx.Size(); //设置DMA待传输数据数目
				SPIbufferTx.Gets(bufferTxDma, SPIbufferTx.Size());//将剩下的数据放入DMA缓冲区
			}
			else{
				mDMA_Streamx->NDTR = SPI_DMA_TX_BUFFER_SIZE;
				SPIbufferTx.Gets(bufferTxDma, SPI_DMA_TX_BUFFER_SIZE);//传输数据
			}
			DMA_Cmd(mDMA_Streamx, ENABLE);
		}
	}
	else if (!mUseDma && !isBusySend){//使用中断发送
		if (SPIbufferTx.Size() > 0){
			isBusySend = true;
			static u8 dataToSend = 0;
			SPIbufferTx.Get(dataToSend);
			SPI_I2S_SendData(SPIx, dataToSend);
			SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE);
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, ENABLE);
			SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE);
		}
	}
	else if (isBusySend){//正在发送数据 或者发生错误
		return false;
	}
	return true;
}

bool SPI::GetReceivedData(u8* buffer, u16 number){
	if (SPIbufferRx.Size() < number){
		return false;
	}
	else {
		SPIbufferRx.Gets(buffer, number);
		return true;
	}
}

SPIIrqType SPI::SpiIrq(){

	SPIIrqType IrqType;
	if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_RXNE) == SET){//接收缓冲区非空

		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_RXNE);//清除中断标志
		SPIbufferRx.Put(SPI_I2S_ReceiveData(SPIx));
		IrqType = SPI_RXNE_IRQ;
	}
	else if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_TXE) == SET){//发送缓冲区空

		static u8 dataToSend = 0;
		SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_TXE);//清除中断标志
		if (SPIbufferTx.Size() > 0){
			SPIbufferTx.Get(dataToSend);
			SPI_I2S_SendData(SPIx, dataToSend);//发送缓冲区不为空，发送数据
		}
		else if (SPIbufferTx.Size() == 0){
			SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, DISABLE);//发送缓冲区空，关闭中断
			isBusySend = false;
		}
		IrqType = SPI_TXE_IRQ;
	}
	else if (SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_ERR) == SET){//错误中断

		SPI_I2S_GetITStatus(SPIx, SPI_I2S_IT_ERR);
		SPI_I2S_ClearITPendingBit(SPIx, SPI_I2S_IT_ERR);//清除中断标志
		IrqType = SPI_ERR_IRQ;
		isBusySend = false;
	}
	return IrqType;
}

DMAIrqType SPI::DmaIrq(){

	DMAIrqType IrqType;
	if (DMA_GetITStatus(mDMA_Streamx, dmaTCFlagChannel) == SET){//传输完成中断

		DMA_ClearITPendingBit(mDMA_Streamx, dmaTCFlagChannel);
		DMA_ClearFlag(mDMA_Streamx, dmaTCFlagChannel);
		IrqType = DMA_TE_IRQ;
		DMA_Cmd(mDMA_Streamx, DISABLE);
		if (SPIbufferTx.Size() > 0){
			if (SPIbufferTx.Size() <= SPI_DMA_TX_BUFFER_SIZE){
				mDMA_Streamx->NDTR = SPIbufferTx.Size(); //设置DMA待传输数据数目
				SPIbufferTx.Gets(bufferTxDma, SPIbufferTx.Size());
			}
			else{
				mDMA_Streamx->NDTR = SPI_DMA_TX_BUFFER_SIZE;
				SPIbufferTx.Gets(bufferTxDma, SPI_DMA_TX_BUFFER_SIZE);//传输数据
			}
		}
		else{
			isBusySend = false;
		}
	}
	else if (DMA_GetITStatus(mDMA_Streamx, dmaTEFlagChannel) == SET){//错误中断

		DMA_ClearITPendingBit(mDMA_Streamx, dmaTEFlagChannel);
		DMA_ClearFlag(mDMA_Streamx, dmaTEFlagChannel);
		IrqType = DMA_TE_IRQ;
		isBusySend = false;  //暂时清除总线忙标志
	}
	return IrqType;
}

u16 SPI::ReceiveBufferSize(){
	return SPIbufferRx.Size();
}

u16 SPI::SendBufferSize(){
	return SPIbufferTx.Size();
}

void SPI::ClearReceiveBuffer(){
	SPIbufferRx.Clear();
}

void SPI::ClearSendBuffer(){
	SPIbufferTx.Clear();
}

bool SPI::UseDma(){
	return mUseDma;
}

SPI_TypeDef* SPI::getSPI(){
	return SPIx;
}

void SPI::SetCSNPin(u8 value){
	//	mCSN.SetLevel(value);
}

//SPI::~SPI(){
//	if (SPI1 == SPIx){
//#ifdef USE_SPI1
//		pSPI1 = 0;
//#endif
//	}
//	else if (SPI2 == SPIx){
//#ifdef USE_SPI2
//		pSPI2 = 0;
//#endif
//	}
//}

