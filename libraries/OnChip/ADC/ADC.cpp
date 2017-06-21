#include "ADC.h"

//  ADC Table Initialize:           CH0           CH1        CH2         CH3         CH4         CH5         CH6         CH7         CH8         CH9					CH10				CH11				CH12				CH13					CH14			CH15
u16  MADC::_pin[MAX_ADC_CHANNEL] = { GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_13, GPIO_Pin_4, GPIO_Pin_5 };
bool MADC::_enCh[MAX_ADC_CHANNEL] = { false, false, false, false, false, false, false, false, false, false };
u8   MADC::_chMap[MAX_ADC_CHANNEL] = { 0 };     //map channel number to the index of _adcValue
u16  MADC::_adcValue[MAX_ADC_CHANNEL] = { 0 }; //adc value for each channel

//constructor: ADC and DMA initialize
MADC::MADC(u8 ch0Num, u8 ch1Num, u8 ch2Num, u8 ch3Num, u8 ch4Num, u8 ch5Num, u8 ch6Num, u8 ch7Num, u8 ch8Num, u8 ch9Num, u8 ch10Num, u8 ch11Num, u8 ch12Num, u8 ch13Num, u8 ch14Num, u8 ch15Num)
{
	//Step 1: set adc channel enable flag
	if (ch0Num < MAX_ADC_CHANNEL) _enCh[ch0Num] = true;
	if (ch1Num < MAX_ADC_CHANNEL) _enCh[ch1Num] = true;
	if (ch2Num < MAX_ADC_CHANNEL) _enCh[ch2Num] = true;
	if (ch3Num < MAX_ADC_CHANNEL) _enCh[ch3Num] = true;
	if (ch4Num < MAX_ADC_CHANNEL) _enCh[ch4Num] = true;
	if (ch5Num < MAX_ADC_CHANNEL) _enCh[ch5Num] = true;
	if (ch6Num < MAX_ADC_CHANNEL) _enCh[ch6Num] = true;
	if (ch7Num < MAX_ADC_CHANNEL) _enCh[ch7Num] = true;
	if (ch8Num < MAX_ADC_CHANNEL) _enCh[ch8Num] = true;
	if (ch9Num < MAX_ADC_CHANNEL) _enCh[ch9Num] = true;
	if (ch10Num < MAX_ADC_CHANNEL) _enCh[ch10Num] = true;
	if (ch11Num < MAX_ADC_CHANNEL) _enCh[ch11Num] = true;
	if (ch12Num < MAX_ADC_CHANNEL) _enCh[ch12Num] = true;
	if (ch13Num < MAX_ADC_CHANNEL) _enCh[ch13Num] = true;
	if (ch14Num < MAX_ADC_CHANNEL) _enCh[ch14Num] = true;
	if (ch15Num < MAX_ADC_CHANNEL) _enCh[ch15Num] = true;

	//Step 2: initialize rcc and gpio
	u16 pinA = 0, pinB = 0, pinC = 0, chCnt = 0;

	for (int i = 0; i < MAX_ADC_CHANNEL; i++)
	{
		if (_enCh[i])//enabled adc channel
		{
			chCnt++;                 //count the total number of enabled adc channel
			if (i < 8)	pinA |= _pin[i]; //all enabled adc pin in GPIOA
			else if ((i > 7) && (i < 10))    pinB |= _pin[i]; //all enabled adc pin in GPIOB
			else pinC |= _pin[i];
		}
	}
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //gpio pin mode: anology input
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	if (pinA) //initialize rcc and gpio in GPIOA
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//open the PinA clock
		GPIO_InitStructure.GPIO_Pin = pinA;									//Pin
		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
	if (pinB) //initialize rcc and gpio in GPIOB
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin = pinB;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
	}
	if (pinC)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		GPIO_InitStructure.GPIO_Pin = pinC;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
	}

	//Step 3: Initialize DMA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                         //DMA1 clock
	DMA_InitTypeDef  DMA_InitStructure;
	DMA_DeInit(DMA2_Stream0);                                                  //default set DMA1
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = u32(&(ADC1->DR));							 //ADC1->DR address
	DMA_InitStructure.DMA_Memory0BaseAddr = u32(_adcValue);	                   //first address for save adc value
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;										 //以外设为DMA源
	DMA_InitStructure.DMA_BufferSize = chCnt;                                  //数据转移量为mAdcTotal（1路AD）
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;	         //外设地址不递增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  			    				 //内存地址不递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//接下来的跟采集一路AD一样
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);            //Initialize DMA channel
	DMA_Cmd(DMA2_Stream0, ENABLE);                         //enable DMA channel

	//Step 4: Initialize ADC
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);		//ADC1 clock
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);	  //ADC1复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);	//复位结束

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;//独立模式
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //DMA失能
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
	ADC_CommonInit(&ADC_CommonInitStructure);//初始化

	ADC_InitTypeDef  ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 									//Continuous Convertion
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = chCnt;											    //total adc channel	
	ADC_Init(ADC1, &ADC_InitStructure);

	for (int i = MAX_ADC_CHANNEL - 1; i >= 0; i--)
	{
		if (_enCh[i])
		{
			ADC_RegularChannelConfig(ADC1, i, chCnt--, ADC_SampleTime_480Cycles);
			_chMap[i] = chCnt;  //map adc channel number to adc value array index _adcValue[_chMap[ADC Channel Number]]
		}
	}
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);//数据源变化时传送DMA
	ADC_DMACmd(ADC1, ENABLE);                   			//enable DMA for ADC1
	ADC_Cmd(ADC1, ENABLE);                     		  //enable ADC1
	ADC_SoftwareStartConv(ADC1);     					 			//start adc1
}
//get adc channel votage value
double MADC::operator[](u8 chNum)
{
	if (chNum >= MAX_ADC_CHANNEL)//adc channel number check
		return 0;
	if (_enCh[chNum]) //if adc channel enabled
		return _adcValue[_chMap[chNum]] * 0.0008056640625;// 0.0008056640625 = 3.3v / 4096
	else             //adc channel not enabled
		return 0;
}
