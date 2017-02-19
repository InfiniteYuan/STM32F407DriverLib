#include "TaskManager.h"

#include "USART.h"
extern USART com;

double TaskManager::_it_time = 0;
u16 TaskManager::_lose_tick_cnt = 0;
double TaskManager::_old_time = 0;

//Constructor
TaskManager::TaskManager()
{
	SysTick->CTRL &= 0xFFFFFFFB; //168MHz Clock div 8 = 21M, disable INT, disable count
	SysTick->LOAD = 12600000;     //1.8s/3 = 0.6s
	SysTick->VAL = 0;
	SysTick->CTRL |= 0x00000003; //ENABLE count + INT
	for (volatile int i = 0; i < 1000; i++);
}

//get current time since power on
double TaskManager::Time()
{
	double curTime = _it_time + 0.6 - SysTick->VAL / 21000000.0; //update current time

	double dt = curTime - _old_time;

	if (dt < -1) //not INT, VAL reload(-1.8s)
	{
		curTime += 0.6;
	}
	else if (dt > 0.59)
	{
		curTime -= 0.6;
	}
	_old_time = curTime;            //update old time
	return curTime;
}

void TaskManager::Update()
{
	_it_time += 0.6;

	double dt = _it_time - Time();
	if (dt < -0.59)
	{
		_lose_tick_cnt++;
		_it_time += 0.6;
		//		com<<"Lost tick:"<<_lose_tick_cnt<<"\n";
	}
}

void TaskManager::DelayUs(u16 nus)
{
	double OldT = Time();
	while ((Time() - OldT) < double(nus) / 1000000.0);
}

void TaskManager::DelayMs(u16 nms)
{
	double OldT = Time();
	while ((Time() - OldT) < double(nms) / 1000.0);
}

void TaskManager::DelayS(u16 ns)
{
	double OldT = Time();
	while ((Time() - OldT) < double(ns));
}

//SysTick interrupt IRQ handler
extern "C"
{
	void SysTick_Handler(void)
	{
		TaskManager::Update();
	}
}


