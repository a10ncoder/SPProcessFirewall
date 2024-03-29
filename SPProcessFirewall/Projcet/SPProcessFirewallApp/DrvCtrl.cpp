#include "DrvCtrl.h"

//初始化各个变量
CDrvCtrl::CDrvCtrl(void)
	: m_hSCManager(nullptr)
	, m_hService(nullptr)
	, m_hDriver(INVALID_HANDLE_VALUE)
{

}

//释放句柄
CDrvCtrl::~CDrvCtrl(void)
{
	if (m_hService)
	{
		CloseServiceHandle(m_hService);
		m_hService = nullptr;
	}
	if (m_hSCManager)
	{
		CloseServiceHandle(m_hSCManager);
		m_hSCManager = nullptr;
	}
	if (m_hDriver)
	{
		CloseHandle(m_hDriver);
		m_hDriver = nullptr;
	}
}

//打开已经存在的服务
bool CDrvCtrl::GetSvcHandle(std::string pServiceName)
{
	if (!pServiceName.empty())
	{
		//保存服务名称
		m_pServiceName = pServiceName;

		m_hSCManager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == m_hSCManager)
		{
			m_dwLastError = GetLastError();
			return false;
		}
		m_hService = ::OpenServiceA(m_hSCManager, m_pServiceName.c_str(), SERVICE_ALL_ACCESS);
		if (NULL == m_hService)
		{
			CloseServiceHandle(m_hSCManager);
			return false;
		}
		else
		{
			return true;
		}
	}
	return true;
}

//安装服务服务
bool CDrvCtrl::Install(std::string pSysPath, std::string pServiceName, std::string pDisplayName)
{
	if (!pSysPath.empty() &&
		!pServiceName.empty() &&
		!pDisplayName.empty()
		)
	{
		//保存驱动路径
		m_pSysPath = pSysPath;
		//保存服务名称
		m_pServiceName = pServiceName;
		//保存显示名称
		m_pDisplayName = pDisplayName;

		m_hSCManager = ::OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (NULL == m_hSCManager)
		{
			m_dwLastError = ::GetLastError();
			return false;
		}
		m_hService = ::CreateServiceA(m_hSCManager, m_pServiceName.c_str(), m_pDisplayName.c_str(),
			SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
			m_pSysPath.c_str(), NULL, NULL, NULL, NULL, NULL);
		if (NULL == m_hService)
		{
			m_dwLastError = ::GetLastError();
			if (ERROR_SERVICE_EXISTS == m_dwLastError)
			{
				m_hService = ::OpenServiceA(m_hSCManager, m_pServiceName.c_str(), SERVICE_ALL_ACCESS);
				if (NULL == m_hService)
				{
					CloseServiceHandle(m_hSCManager);
					return false;
				}
			}
			else
			{
				CloseServiceHandle(m_hSCManager);
				return false;
			}
		}
	}
	
	
	return true;
}

//启动服务
bool CDrvCtrl::Start()
{
	if (!StartServiceA(m_hService, NULL, NULL))
	{
		m_dwLastError = GetLastError();
		return false;
	}
	return true;
}

//停止服务
bool CDrvCtrl::Stop()
{
	SERVICE_STATUS ServiceStatus = {0};
	if (!::ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		m_dwLastError = ::GetLastError();
		return false;
	}
	return true;
}

//移除服务
bool CDrvCtrl::Remove()
{
	if (!::DeleteService(m_hService))
	{
		m_dwLastError = ::GetLastError();
		return false;
	}
	return true;
}

//打开驱动的符号链接
bool CDrvCtrl::Open(std::string pLinkName)
{
	if (!pLinkName.empty())
	{
		if (m_hDriver != INVALID_HANDLE_VALUE)
			return true;
		m_hDriver = CreateFileA(pLinkName.c_str(), 
			GENERIC_READ | GENERIC_WRITE, 
			0, 
			0, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, 
			0);
		if (m_hDriver != INVALID_HANDLE_VALUE)
			return true;
		else
			return false;
	}
	return false;
}

int CDrvCtrl::CTL_CODE_GEN(unsigned long lngFunction)
{
	return (FILE_DEVICE_UNKNOWN * 65536) | (FILE_ANY_ACCESS * 16384) | (lngFunction * 4) | METHOD_BUFFERED;
}

std::wstring StringToWString(const std::string& str) 
{
	int num = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	wchar_t *wide = new wchar_t[num];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wide, num);
	std::wstring w_str(wide);
	delete[] wide;
	return w_str;
}

BOOL CDrvCtrl::IoInsertMonitor(std::string pData)
{
	PROC_INSERT_ITEM pProcInsertItem;
	RtlZeroMemory(&pProcInsertItem, sizeof(PROC_INSERT_ITEM));
	std::wstring closeGamePro = StringToWString(pData);
	pProcInsertItem.nType = IOCT_PROCESS_INSERT_LIST;
	wmemcpy(pProcInsertItem.ImageName, closeGamePro.c_str(), closeGamePro.length());
	//模拟游戏创建时候，插入进程
	return IoControl(0x812, &pProcInsertItem, sizeof(PROC_INSERT_ITEM), NULL, NULL);
}

//和驱动实现通信的核心函数
BOOL CDrvCtrl::IoControl(unsigned long dwIoCode, PVOID InBuff, unsigned long InBuffLen, PVOID OutBuff, unsigned long OutBuffLen)
{
	unsigned long dw;
	return ::DeviceIoControl(m_hDriver, CTL_CODE_GEN(dwIoCode), InBuff, InBuffLen, OutBuff, OutBuffLen, &dw, NULL);
}