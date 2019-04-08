#pragma once

/************************************************************************/
/* �ܿ��ƿ���                                                           */
/************************************************************************/
//�����¼�����  
#define IOCTL_START_MONITOR   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x810,METHOD_BUFFERED, FILE_ANY_ACCESS) 

//ֹͣ�¼�����
#define IOCTL_STOP_MONITOR    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x811,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//�������ؽ���  
#define IOCTL_ADD_RULE        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x812,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//ɾ�����ؽ���  
#define IOCTL_DEL_RULE        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x813,METHOD_BUFFERED, FILE_ANY_ACCESS)  

//��ȡ���ؽ�����Ϣ  
#define IOCTL_GET_INFO        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x814,METHOD_BUFFERED, FILE_ANY_ACCESS) 

/*************************************************************************
Prototypes
*************************************************************************/
#define _Device_Name_     L"\\Device\\SPProcessFirewall"
#define _Symbolic_Name_   L"\\DosDevices\\SPProcessFirewall"
#define _Win32_Link_Name_ "\\\\.\\SPProcessFirewall" // Win32 Link Name

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
);

//ж������
VOID Unload(
	_In_ struct _DRIVER_OBJECT *DriverObject
);

//Ĭ���ɷ�
NTSTATUS
Dispatc(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
);

//��ʼ���豸
BOOLEAN InitializeDeviceObject(PDRIVER_OBJECT DriverObject);

// ���̷���ǽ�ص�
VOID ProcessFirewallCallBackEx(PEPROCESS  EProcess, HANDLE  ProcessId, PPS_CREATE_NOTIFY_INFO  CreateInfo);

NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_ PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength
);

//����EPROCESS��ȡ������
PUNICODE_STRING GetProcessNameByEprocess(IN PEPROCESS EProcess);

NTSTATUS CtrlCode_StartMonitor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CtrlCode_StopMonitor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CtrlCode_AddRule(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CtrlCode_GetInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

