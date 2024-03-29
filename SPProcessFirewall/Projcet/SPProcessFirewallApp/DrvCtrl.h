#pragma once
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//+                                                           +//
//+                          实现类                           +//
//+                                                           +//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
/////////////////////////////////////////////////////////////////
#include <string>
#include <Windows.h>
#include <winsvc.h>
#pragma comment(lib,"Advapi32.lib")

#include "SingletonHandler.h"

class CDrvCtrl
{
public:
	CDrvCtrl(void);
	virtual ~CDrvCtrl(void);
public:
	//最后的错误
	unsigned long                 m_dwLastError;
	//驱动路径
	std::string			  m_pSysPath;
	//服务名
	std::string           m_pServiceName;
	//显示名
	std::string           m_pDisplayName;
	//驱动句柄
	HANDLE m_hDriver;
	//SCM句柄
	SC_HANDLE m_hSCManager;
	//服务句柄
	SC_HANDLE m_hService;				
public:
	//安装驱动服务
	bool Install(std::string pSysPath, std::string pServiceName, std::string pDisplayName);
	//启动驱动服务
	bool Start();
	//停止驱动服务
	bool Stop();
	//移除驱动服务
	bool Remove();
	//打开驱动句柄
	bool Open(std::string pLinkName); 
	//IO控制
	BOOL IoControl(unsigned long dwIoCode, PVOID InBuff, unsigned long InBuffLen, PVOID OutBuff, unsigned long OutBuffLen); 
	//打开已经存在的服务
	bool GetSvcHandle(std::string pServiceName);
	int  CTL_CODE_GEN(unsigned long lngFunction);
public:
	BOOL IoInsertMonitor(std::string pData);
};

typedef CSingletonHandler<CDrvCtrl> CProcessMonitor;

//
// 进程全局
//
typedef enum _tag_PROCESS_EVENT_TYPE
{
	IOCT_PROCESS_CREATE = 0,
	IOCT_PROCESS_CLOSE,
	IOCT_PROCESS_INSERT_LIST,
}PROCESS_EVENT_TYPE;

typedef struct _PROC_INSERT_ITEM_
{
	PROCESS_EVENT_TYPE  nType;
	WCHAR         ImageName[64];
}PROC_INSERT_ITEM, *LPPROC_INSERT_ITEM;

/************************************************************************/
/* 总控制开关                                                           */
/************************************************************************/
//启动事件控制  
#define IOCTL_START_MONITOR   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810,METHOD_BUFFERED, FILE_ANY_ACCESS) 

//停止事件控制
#define IOCTL_STOP_MONITOR    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//增加拦截进程  
#define IOCTL_ADD_RULE        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//删除拦截进程  
#define IOCTL_DEL_RULE        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//获取拦截进程信息  
#define IOCTL_GET_INFO        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x814,METHOD_BUFFERED, FILE_ANY_ACCESS) 

