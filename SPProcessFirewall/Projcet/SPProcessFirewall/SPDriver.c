#include "stdafx.h"
#include "SPDriver.h"


//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, Unload)
#pragma alloc_text(PAGE, Dispatc)
#endif

NTSTATUS
DriverEntry(
_In_ PDRIVER_OBJECT DriverObject,
_In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("[Alan]Driver Loading...\r\n"));

	do
	{

//#ifndef DBG
	//	DriverObject->DriverUnload = NULL;
//#else
		DriverObject->DriverUnload = Unload;
//#endif

		for (size_t i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
			DriverObject->MajorFunction[i] = Dispatc;
		}

		/*************************************************************************/
		/*初始化设备                                                             */
		/*************************************************************************/
		if (!InitializeDeviceObject(DriverObject))
		{
			DbgMsg(__FILE__, __LINE__, "初始化设备失败.\n");
			break;
		}

		/*************************************************************************/
		/*初始化链表                                                             */
		/*************************************************************************/
		if (!InitializeListManager())
		{
			DbgMsg(__FILE__, __LINE__, "初始化进程监视列表失败.\n");
			break;
		}

		/*************************************************************************/
		/*初始化进程防火墙                                                       */
		/*************************************************************************/
		status = PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)ProcessFirewallCallBackEx, FALSE);
		if (!NT_SUCCESS(status))
		{
			break;
		}
		

	} while (FALSE);

    return status;
}

VOID Unload(
	_In_ struct _DRIVER_OBJECT *DriverObject
)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNICODE_STRING SymbolicLinkName = { 0 };
	PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
	PAGED_CODE();

	//
	// Delete the link from our device name to a name in the Win32 namespace.
	//
	RtlInitUnicodeString(&SymbolicLinkName, _Symbolic_Name_);
	IoDeleteSymbolicLink(&SymbolicLinkName);
	if (deviceObject != NULL)
	{
		IoDeleteDevice(deviceObject);
		deviceObject = NULL;
	}


	if (!NT_SUCCESS(PsSetCreateProcessNotifyRoutineEx(ProcessFirewallCallBackEx, TRUE)))
	{
		DbgMsg(__FILE__, __LINE__, "卸载进程监视回调失败.\n");
	}

	//卸载所有内存对象
	UninitializeListManager();

	DbgMsg(__FILE__, __LINE__, "[by:alan] driver uninstall Success !.\n");
}

//初始化设备
BOOLEAN InitializeDeviceObject(PDRIVER_OBJECT DriverObject)
{
	BOOLEAN IsSuccess = FALSE;
	PDEVICE_OBJECT DeviceObject = NULL;
	UNICODE_STRING DeviceName = { 0 };
	UNICODE_STRING SymbolicLinkName = { 0 };

	do
	{
		RtlInitUnicodeString(&DeviceName, _Device_Name_);
		if (!NT_SUCCESS(IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject)))
		{
			break;
		}
		else
		{
			RtlInitUnicodeString(&SymbolicLinkName, _Symbolic_Name_);
			if (!NT_SUCCESS(IoCreateSymbolicLink(&SymbolicLinkName, &DeviceName)))
			{
				IoDeleteDevice(DeviceObject);
				break;
			}
			else
			{
				IsSuccess = TRUE;
			}
		}

	} while (FALSE);

	return IsSuccess;
}

/** 向前声明 */
NTKERNELAPI
UCHAR * PsGetProcessImageFileName(__in PEPROCESS Process);
// 进程防火墙回调
VOID ProcessFirewallCallBackEx(PEPROCESS  EProcess, HANDLE  ProcessId, PPS_CREATE_NOTIFY_INFO  CreateInfo)
{
	UNREFERENCED_PARAMETER(EProcess);
	UNREFERENCED_PARAMETER(ProcessId);

	if (CreateInfo)
	{
		UNICODE_STRING uProcessName;
		ANSI_STRING    aProcessName;
		RtlInitString(&aProcessName, (PCSZ)PsGetProcessImageFileName(EProcess));
		RtlAnsiStringToUnicodeString(&uProcessName, &aProcessName, TRUE);
		LPPROC_MONITOR_ITEM pProcMonitorItem = FindProcessMonitorItemByProcessName(&uProcessName);
		if (NULL != pProcMonitorItem)
		{
			//设置事件为有信号，通知应用层
			//RtlZeroMemory(&g_ProcInsertItem, sizeof(PROC_INSERT_ITEM));
			//g_ProcInsertItem.nType = IOCT_PROCESS_CREATE;
			//RtlCopyMemory(&g_ProcInsertItem.ImageName, &pProcMonitorItem->InsertItem.ImageName, wcslen(pProcMonitorItem->InsertItem.ImageName) * sizeof(WCHAR));
			//KeSetEvent(g_EventObject, 0, FALSE);
			CreateInfo->CreationStatus = STATUS_ACCESS_DISABLED_NO_SAFER_UI_BY_POLICY;
		}
		RtlFreeUnicodeString(&uProcessName);
	}
}

NTSTATUS
Dispatc(
	_In_ struct _DEVICE_OBJECT *DeviceObject,
	_Inout_ struct _IRP *Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);
	NTSTATUS            status = 0;
	ULONG               ulCtrlCode = 0;
	PIO_STACK_LOCATION  irpSp = NULL;
	status = STATUS_SUCCESS;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	ulCtrlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
	PAGED_CODE();

	do
	{
		switch (irpSp->MajorFunction)
		{
		case IRP_MJ_DEVICE_CONTROL:
		case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		{
			switch (ulCtrlCode)
			{
			case IOCTL_START_MONITOR:
			{
				if (!NT_SUCCESS(CtrlCode_StartMonitor(DeviceObject, Irp)))
				{
					DbgMsg(__FILE__, __LINE__, "IoProcessStartMonitor Error~!\n");
					break;
				}
			}break;
			case IOCTL_STOP_MONITOR:
			{
				if (!NT_SUCCESS(CtrlCode_StopMonitor(DeviceObject, Irp)))
				{
					DbgMsg(__FILE__, __LINE__, "IoProcessStopMonitor Error~!\n");
					break;
				}

			}break;
			case IOCTL_ADD_RULE:
			{
				if (!NT_SUCCESS(CtrlCode_AddRule(DeviceObject, Irp)))
				{
					DbgMsg(__FILE__, __LINE__, "IoProcessInsert Error~!\n");
					break;
				}
			}break;
			case IOCTL_GET_INFO:
			{
				if (!NT_SUCCESS(CtrlCode_GetInfo(DeviceObject, Irp)))
				{
					DbgMsg(__FILE__, __LINE__, "IoProcessInsert Error~!\n");
					break;
				}
			}
			default:
				break;
			}
			break;
		}
		default:
			Irp->IoStatus.Information = 0;
			break;
		}

	} while (FALSE);

	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

PUNICODE_STRING GetProcessNameByEprocess(IN PEPROCESS EProcess)
{
	NTSTATUS NtStatus;
	HANDLE hProc = NULL;
	PBYTE pBuf = NULL;
	ULONG ulSize = 0;
	PAGED_CODE();

	//
	// 1. pEproc --> handle
	//
	NtStatus = ObOpenObjectByPointer(EProcess,OBJ_KERNEL_HANDLE,NULL,0,NULL,KernelMode,&hProc);
	if (!NT_SUCCESS(NtStatus))
	{
		return NULL;
	}
		
	//
	// 2. ZwQueryInformationProcess
	//
	while (TRUE)
	{
		pBuf = ExAllocatePoolWithTag(NonPagedPool, ulSize, 'proc');
		if (!pBuf)
		{
			ZwClose(hProc);
			return NULL;
		}

		NtStatus = ZwQueryInformationProcess(hProc,ProcessImageFileName,pBuf,ulSize,&ulSize);
		if (NtStatus != STATUS_INFO_LENGTH_MISMATCH)
		{
			break;
		}
			
		ExFreePool(pBuf);
	}

	ZwClose(hProc);

	if (!NT_SUCCESS(NtStatus))
	{
		ExFreePool(pBuf);
		return NULL;
	}

	return (PUNICODE_STRING)pBuf;
}

NTSTATUS CtrlCode_StartMonitor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	NTSTATUS            status = 0;
	PIO_STACK_LOCATION  irpSp = NULL;
	PVOID               InputBuffer = NULL;
	ULONG               ulInputBufferSize = 0;
	status = STATUS_SUCCESS;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	InputBuffer = Irp->AssociatedIrp.SystemBuffer;
	ulInputBufferSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	PAGED_CODE();

	do
	{
		//设置同步事件  
		if (InputBuffer == NULL || ulInputBufferSize < sizeof(HANDLE))
		{
			DbgMsg(__FILE__, __LINE__, "Set Event Error~!\n");
			break;
		}

		//取得句柄对象  
		HANDLE hEvent = *(HANDLE*)InputBuffer;
		status = ObReferenceObjectByHandle(hEvent, GENERIC_ALL, *ExEventObjectType, KernelMode, &g_EventObject, NULL);
		if (!NT_SUCCESS(status))
		{
			DbgMsg(__FILE__, __LINE__, "ObReferenceObjectByHandle Error~!\n");
			break;
		}


	} while (FALSE);

	return status;
}


NTSTATUS CtrlCode_AddRule(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	NTSTATUS            status = 0;
	PIO_STACK_LOCATION  irpSp = NULL;
	PVOID               InputBuffer = NULL;
	ULONG               ulInputBufferSize = 0;
	UNICODE_STRING      uniProcessName = { 0 };
	LPPROC_MONITOR_ITEM pProcMonitorItem = NULL;
	LPPROC_INSERT_ITEM  pProcInsertItem = NULL;
	status = STATUS_SUCCESS;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	InputBuffer = Irp->AssociatedIrp.SystemBuffer;
	ulInputBufferSize = irpSp->Parameters.DeviceIoControl.InputBufferLength;
	PAGED_CODE();

	do
	{
		//设置同步事件  
		if (InputBuffer == NULL || ulInputBufferSize < sizeof(PROC_INSERT_ITEM))
		{
			DbgMsg(__FILE__, __LINE__, "Set ImageName struct size Error~!\n");
			break;
		}

		pProcInsertItem = (LPPROC_INSERT_ITEM)InputBuffer;
		RtlInitUnicodeString(&uniProcessName, pProcInsertItem->ImageName);
		pProcMonitorItem = FindProcessMonitorItemByProcessName(&uniProcessName);
		if (NULL == pProcMonitorItem)
		{
			// 把进程名转换成UNICODE格式
			pProcMonitorItem = MallocProcessMonitorItem();
			if (NULL == pProcMonitorItem)
			{
				pProcMonitorItem = MallocProcessMonitorItem();
				if (NULL == pProcMonitorItem)
				{
					break;
				}
			}

			pProcMonitorItem->InsertItem.nType = pProcInsertItem->nType;
			RtlCopyMemory(pProcMonitorItem->InsertItem.ImageName, pProcInsertItem->ImageName, wcslen(pProcInsertItem->ImageName) * sizeof(WCHAR));

			//插入到白名单链表
			InsertProcessMonitorItemToList(pProcMonitorItem);
		}

	} while (FALSE);

	return status;
}

NTSTATUS CtrlCode_GetInfo(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	NTSTATUS            status = 0;
	PIO_STACK_LOCATION  irpSp = NULL;
	PVOID               OutputBuffer = NULL;
	ULONG               ulOutputBufferSize = 0;
	status = STATUS_SUCCESS;
	irpSp = IoGetCurrentIrpStackLocation(Irp);
	OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
	ulOutputBufferSize = irpSp->Parameters.DeviceIoControl.OutputBufferLength;
	PAGED_CODE();

	do
	{
		PROC_INSERT_ITEM pProcInsertItem = { 0 };
		pProcInsertItem.nType = g_ProcInsertItem.nType;
		RtlCopyMemory(&pProcInsertItem.ImageName, &g_ProcInsertItem.ImageName, wcslen(g_ProcInsertItem.ImageName) * sizeof(WCHAR));
		RtlCopyMemory(OutputBuffer, &pProcInsertItem, sizeof(PROC_INSERT_ITEM));
		ulOutputBufferSize = sizeof(PROC_INSERT_ITEM);
		Irp->IoStatus.Information = ulOutputBufferSize;

	} while (FALSE);

	return status;

}

NTSTATUS CtrlCode_StopMonitor(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	NTSTATUS status = 0;
	status = STATUS_SUCCESS;
	PAGED_CODE();

	do
	{
		//释放对象引用  
		if (g_EventObject != NULL)
		{
			ObDereferenceObject(g_EventObject);
			g_EventObject = NULL;
		}

	} while (FALSE);

	return status;

}