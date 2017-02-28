#include "GPRS.h"

GPRS::GPRS(USART& usart):mUsart(usart),mIsConnected(false)
{
	mTimeOut = 3;
}
bool GPRS::Kick(void)
{
	mUsart<<"AT\r\n";
	return RecvFind("OK");
}
bool GPRS::Restart(void)
{
	mUsart<<"ATZ\r\n";
	return RecvFind("OK");
}
bool GPRS::GetVersion(char*)
{
	mUsart<<"AT+GSV\r\n";
	return RecvFind("OK");
}
bool GPRS::SetEcho(bool echo)
{
	mUsart<<"ATE"<<(echo?1:0)<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::Restore(void)
{
	mUsart<<"ATZ0\r\n";
	return RecvFind("OK");
}
bool GPRS::SetUart(uint32_t baudrate,GPRS_pattern pattern)
{
	if(pattern>GPRS_PATTERN_DEF)
		return false;
	if(pattern == GPRS_PATTERN_CUR)
	{
		mUsart<<"AT+UART_CUR=";
	}
	else if(pattern == GPRS_PATTERN_DEF)
	{
		mUsart<<"AT+UART_DEF=";
	}
	else
	{
		mUsart<<"AT+UART=";
	}
	mUsart<<(int)baudrate<<",8,1,0,0\r\n";
	if(RecvFind("OK"))
		return true;
	return false;
}
void GPRS::ClearBuffer()
{
	unsigned int i=0;
	for(;i<GPRS_RECEIVE_BUFFER_SIZE;++i)
		mReceiveBuffer[i]=0;
}
/**********Wait Regis*******/
bool GPRS::init(void)
{
	u8 i;
	u8 k;
	i = 0;
	ClearBuffer();
  while(i == 0)        			
	{
		ClearBuffer();        
		mUsart<<"AT+CREG?\r\n";
		tskmgr.DelayMs(5000);
		if(RecvFind(",1",",5"))
		{
			i = 1;
			break;
		}
	}
	return true;
}
bool GPRS::AttachGPRSService(bool IsAttached)
{
	mUsart<<"AT+CGATT="<<(IsAttached?1:0)<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::PDPContextActivation(bool IsActive)
{
	mUsart<<"AT+CGACT="<<(IsActive?1:0)<<",1"<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::CloseConnected(bool IsFast)
{
	mUsart<<"AT+CIPCLOSE="<<(IsFast?1:0)<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::Add_an_IP_Head(bool IsHead)
{
	mUsart<<"AT+CIPHEAD="<<(IsHead?1:0)<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::Define_PDP_Context(void)
{
	mUsart<<"AT+CGDCONT=1,\"IP\",\"CMNET\""<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::GPRS_Mobile_Station_Class(char* cla)
{
	mUsart<<"AT+CGCLASS=\""<<cla<<"\""<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::Deactivate_GPRS_PDP_Context(void)
{
	mUsart<<"AT+CIPSHUT"<<"\r\n";
	return RecvFind("SHUT OK");
}
bool GPRS::Set_CSD_or_GPRS_for_Connection_Mode(void)
{
	mUsart<<"AT+CIPCSGP=1,\"CMNET\""<<"\r\n";
	return RecvFind("OK");
}
bool GPRS::SetMUX(bool isEnableMUX)
{
	mIsEnableMUX = isEnableMUX;
	mUsart<<"AT+CIPMUX="<<(isEnableMUX?1:0)<<"\r\n";							
	return RecvFind("OK");
}
bool GPRS::CreateConnectMutipleMode(char* ipAddr,short port,Socket_Type socketType, signed char muxID)
{
	char type[4]="TCP";
	if(socketType==Socket_Type_Dgram)
	{
		type[0]='U';
		type[1]='D';
	}
	if(muxID!=-1)
		mUsart<<"AT+CIPSTART="<<muxID<<",\""<<type<<"\",\""<<ipAddr<<"\","<<port<<"\r\n";							
	else
		mUsart<<"AT+CIPSTART="<<"\""<<type<<"\",\""<<ipAddr<<"\","<<port<<"\r\n";							
	return RecvFind("OK","ERROR","ALREAY CONNECT");
}
bool GPRS::Connect(char* ipAddr, short port, Socket_Type socketType, Socket_Protocol socketProtocol)
{
	if(CreateConnectMutipleMode(ipAddr,port,socketType))
		mIsConnected = true;
	else
		mIsConnected = false;
	return mIsConnected;
}
bool GPRS::SendMultipleMode(char* data, unsigned int num,signed char muxID)
{
	if(muxID!=-1)
		mUsart<<"AT+CIPSEND="<<muxID<<","<<(int)num<<"\r\n";
	else
		mUsart<<"AT+CIPSEND="<<(int)num<<"\r\n";
	if(!RecvFind("OK\r\n> "))
		return false;
	mUsart.SendBytes((uint8_t*)data,num);
	return RecvFind("OK\r\n","ERROR");
}
bool GPRS::Write(char* data,unsigned int num)
{
	return SendMultipleMode(data,num);
}
unsigned int GPRS::Read(char* data)
{
	float starttime = TaskManager::Time();
	if(!mIsConnected)
		return 0;
	unsigned char temp[13];
	unsigned short bufferSize=0;
	unsigned short dataLength=0;
	unsigned short count=0;
	bool flag = false;
	while(TaskManager::Time()-starttime<mTimeOut){
		bufferSize = mUsart.RxSize();
		if(bufferSize>6)
		{
			for(count=0;count<bufferSize&&count<GPRS_RECEIVE_BUFFER_SIZE;++count)
			{
				mUsart.GetBytes(temp,1);
				if(temp[0]=='+')
				{
					if(!mIsEnableMUX)
						RecvFindAndFilter(":",",",":",(char*)temp,mTimeOut);
					else
					{
						if(RecvFind(":",mTimeOut))
						{
							char* index1=strstr(mReceiveBuffer,",");
							index1=strstr(index1,",");
							char* index2=strstr(mReceiveBuffer,":");

							if(index1&&index2)
							{
								index1+=strlen(",");
								*index2='\0';
								strcpy((char*)temp,index1);
								return true;
							}
						}
					}
					dataLength = atoi((char*)temp);
					flag = true;
					break;
				}
			}
		}
		if(flag)
			break;
	}
	count=0;
	while(TaskManager::Time()-starttime<mTimeOut){
		if(mUsart.RxSize()>0)
		{
			mUsart.GetBytes((unsigned char*)(data+count),1);
			++count;
			if(count==dataLength || count==GPRS_RECEIVE_BUFFER_SIZE)
				break;
		}
	}
	return count;
}
unsigned int GPRS::Read(char* data,unsigned int num)
{
	float starttime = TaskManager::Time();
	if(!mIsConnected)
		return 0;
	unsigned int count=0;
	while(TaskManager::Time()-starttime<mTimeOut&&count<num){
		if(mUsart.RxSize()>0)
		{
			mUsart.GetBytes((unsigned char*)(data+count),1);
			++count;
		}
	}
	return count;
}
bool GPRS::IsAlive()
{
	if(!mIsConnected)
		return false;
	if(GetStatus()==3)
		return true;
	return false;
}
bool GPRS::Close(signed char muxID)
{
	if(muxID!=-1)
		mUsart<<"AT+CIPCLOSE="<<muxID<<"\r\n";
	else
		mUsart<<"AT+CIPCLOSE\r\n";
	return RecvFind("OK","ERROR");
}
void GPRS::SetTimeOut(float timetOut)
{
	mTimeOut = timetOut;
}
bool GPRS::Call(char* phone)
{
	mUsart<<"ATD"<<phone<<";\r\n";
	return RecvFind("OK","ERROR");
}
bool GPRS::SetMsgMode(bool IsText)
{
	mUsart<<"AT+CMGF="<<(IsText?1:0)<<"\r\n"; //1:Text 只能是英文字符，数字 0 PDU 中文短信
	return RecvFind("OK");
}
bool GPRS::SetTECharset(char* charset)
{
	mUsart<<"AT+CSCS=\""<<charset<<"\"\r\n";
	return RecvFind("OK");
}
bool GPRS::SendMsg(char* phone, char* msg, bool IsPDU, u8 length)
{
	if(IsPDU)
		mUsart<<"AT+CMGS="<<length<<"\r\n";
	else
		mUsart<<"AT+CMGS=\""<<phone<<"\"\r\n";
	if(!RecvFind(">"))
		mUsart<<"AT+CMGS=\""<<phone<<"\"\r\n";
	if(!RecvFind(">"))
		mUsart<<msg;
	mUsart.SendByte(0x1A);
	return RecvFind("OK");
}
char GPRS::GetStatus(char* muxID,char* type,char* ipAddr,short remotePort,short localPort)
{
	char status=0;
	bool result=false;
	mUsart<<"AT+CIPSTATUS\r\n";
	result = RecvFindAndFilter("+CIPSTATU", "STATUS:", "\r\n", &status);
	status-='0';
	return result;
}
bool GPRS::ReceiveAndWait(const char* targetString,unsigned char timeOut)
{
		u8 temp;
		mReceiveBufferIndex = 0;
		ClearBuffer();
		double tartTime=TaskManager::Time();
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			while(mUsart.RxSize()>0)
			{
				mUsart.GetBytes(&temp,1);
				if(temp=='\0')
					continue;
				mReceiveBuffer[mReceiveBufferIndex++] = temp;
			}
			
			if(strstr(mReceiveBuffer,targetString))
				return true;	
				
		}
		if(mReceiveBufferIndex>0)
		{
			mReceiveBuffer[mReceiveBufferIndex] = '\0';
		}
		return false;
}
bool GPRS::ReceiveAndWait(char const* targetString,const char* targetString2,unsigned char timeOut)
{
		u8 temp;
		mReceiveBufferIndex = 0;
		ClearBuffer();
		double tartTime=TaskManager::Time();
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			while(mUsart.RxSize()>0)
			{
				mUsart.GetBytes(&temp,1);
				if(temp=='\0')
					continue;
				mReceiveBuffer[mReceiveBufferIndex++] = temp;
				
			}
			
			if(strstr(mReceiveBuffer,targetString) ||
			   strstr(mReceiveBuffer,targetString2))
				return true;	
				
		}
		if(mReceiveBufferIndex>0)
		{
			mReceiveBuffer[mReceiveBufferIndex] = '\0';
		}
		return false;
}
bool GPRS::ReceiveAndWait(char const* targetString,const char* targetString2,const char* targetString3,unsigned char timeOut)
{
		u8 temp;
		mReceiveBufferIndex = 0;
		ClearBuffer();
		double tartTime=TaskManager::Time();
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			while(mUsart.RxSize()>0)
			{
				mUsart.GetBytes(&temp,1);
				if(temp=='\0')
					continue;
				mReceiveBuffer[mReceiveBufferIndex++] = temp;
				
			}
		
			if(strstr(mReceiveBuffer,targetString) ||
			   strstr(mReceiveBuffer,targetString2)||
			   strstr(mReceiveBuffer,targetString3))
				return true;	
		}
		if(mReceiveBufferIndex>0)
		{
			mReceiveBuffer[mReceiveBufferIndex] = '\0';
		}
		return false;
}
bool GPRS::RecvFind(char const *target, unsigned char timeout)
{
	if(!ReceiveAndWait((char*)target,timeout))
		return false;
	return true;
}
bool GPRS::RecvFind(char const *target, char const *target2,unsigned char timeout)
{
	if(!ReceiveAndWait((char*)target,target2,timeout))
		return false;
	return true;
}
bool GPRS::RecvFind(char const *target, char const *target2,char const *target3,unsigned char timeout)
{
	if(!ReceiveAndWait((char*)target,target2,target3,timeout))
		return false;
	return true;
}
bool GPRS::RecvFindAndFilter(char const *target, char const * begin, char const * end, char* Data, float timeout)
{
	if(!ReceiveAndWait((char*)target,timeout))
		return false;
	char* index1=strstr(mReceiveBuffer,begin);
	char* index2=strstr(mReceiveBuffer,end);

	if(index1&&index2)
	{
		index1+=strlen(begin);
		*index2='\0';
		strcpy(Data,index1);
		return true;
	}
	return false;
}
