#include "stdafx.h"
#include "ListManager.h"

#define PROCESS_TAG 'proc'    //申请内存标记

// 进程 全局对象管理
LIST_ENTRY            g_ProcessMonitorList;      // 链表

// 内存池
NPAGED_LOOKASIDE_LIST g_ProcessMonitorListPool = {0};

// 初始化同步互斥对象
FAST_MUTEX            g_ProcessFastMutex = {0};

//同步事件对象  
PKEVENT               g_EventObject = NULL;

PROC_INSERT_ITEM      g_ProcInsertItem = {0};

//////////////////////////////////////////////////////////////////////////
// 名称: InitializeListManager
// 说明: 初始化链表
// 返回: 初始化结构,返回TRUE表示初始化完成
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
BOOLEAN InitializeListManager()
{
    BOOLEAN status = FALSE;

	do
	{
		// 初始进程全局项
		InitializeListHead(&g_ProcessMonitorList);

		// 初始化互斥同步对象
		ExInitializeFastMutex(&g_ProcessFastMutex);

		// 初始化进程全局项内存池链表
		ExInitializeNPagedLookasideList(&g_ProcessMonitorListPool, NULL, NULL, 0, sizeof(PROC_MONITOR_ITEM), PROCESS_TAG, 0);
		if (&g_ProcessMonitorListPool == NULL)
		{
			DbgMsg(__FILE__, __LINE__, "初始化进程全局项内存池失败.\n");
			break;
		}

		status = TRUE;

	} while (FALSE);
        

    return status;
}

//////////////////////////////////////////////////////////////////////////
// 名称: UninitializeListManager
// 说明: 删除所有链表对象
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID UninitializeListManager()
{
	
    PLIST_ENTRY pListEntry = NULL;
	LPPROC_MONITOR_ITEM pProcMonitorItem = NULL;

	ExAcquireFastMutex(&g_ProcessFastMutex);
    // 清空进程全局项链表
    while (!IsListEmpty(&g_ProcessMonitorList)) 
    {
        pListEntry = RemoveTailList(&g_ProcessMonitorList);
        pProcMonitorItem = CONTAINING_RECORD(pListEntry, PROC_MONITOR_ITEM, ListEntry);
        FreeProcessMonitorItem(pProcMonitorItem);
    }

    ExDeleteNPagedLookasideList(&g_ProcessMonitorListPool);

    if (RemoveHeadList(&g_ProcessMonitorList)) {
        DbgMsg(__FILE__, __LINE__, "移除进程全局列表成功.\n");
    }

	ExReleaseFastMutex(&g_ProcessFastMutex);
}

//////////////////////////////////////////////////////////////////////////
// 名称: MallocProcessMonitorItem
// 说明: 分配连接上下文结构体内存
// 返回: 分配的连接上下文结构体内存地址
// 备注: 如果分配失败,返回值为NULL
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
LPPROC_MONITOR_ITEM MallocProcessMonitorItem()
{
	ExAcquireFastMutex(&g_ProcessFastMutex);
	LPPROC_MONITOR_ITEM pProcMonitorItem = ExAllocateFromNPagedLookasideList(&g_ProcessMonitorListPool);
    if (NULL != pProcMonitorItem) {
        RtlZeroMemory(pProcMonitorItem, sizeof(PROC_MONITOR_ITEM));
        InitializeListHead(&pProcMonitorItem->ListEntry);
    }

	ExReleaseFastMutex(&g_ProcessFastMutex);

    return pProcMonitorItem;
}

//////////////////////////////////////////////////////////////////////////
// 名称: FreeProcessMonitorItem
// 说明: 释放连接上下文相关结构体内存
// 入参: pProcWhiteItem 连接上下文结构体内存
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID FreeProcessMonitorItem(IN LPPROC_MONITOR_ITEM pProcMonitorItem)
{
    ASSERT(NULL != pProcMonitorItem);
    ExFreeToNPagedLookasideList(&g_ProcessMonitorListPool, pProcMonitorItem);
}

//////////////////////////////////////////////////////////////////////////
// 名称: InsertProcessMonitorItemToList
// 说明: 插入到连接上下文链表中
// 入参: pProcWhiteItem 连接上下文结构体内存
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID InsertProcessMonitorItemToList(IN LPPROC_MONITOR_ITEM pProcMonitorItem)
{
	ASSERT(NULL != pProcMonitorItem);
	ExAcquireFastMutex(&g_ProcessFastMutex);
	
	InsertTailList(&g_ProcessMonitorList, &pProcMonitorItem->ListEntry);

	ExReleaseFastMutex(&g_ProcessFastMutex);
}

//////////////////////////////////////////////////////////////////////////
// 名称: DeleteProcessMonitorItemFromList
// 说明: 从链表中删除连接上下文结构体内存
// 入参: pProcWhiteItem 要删除的连接上下文指针
// 返回: 操作的结果
// 备注: 元素只有在链表中的时候才其效果
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
BOOLEAN DeleteProcessMonitorItemFromList(IN LPPROC_MONITOR_ITEM pItem)
{
    ASSERT(NULL != pItem);
	ExAcquireFastMutex(&g_ProcessFastMutex);
    if (IsListEmpty(&pItem->ListEntry)) 
	{
        DbgMsg(__FILE__, __LINE__, "删除连接对象从链表中为空...\n");
        return FALSE;
    }

    // 从链表中移除数据

    RemoveEntryList(&pItem->ListEntry);

    FreeProcessMonitorItem(pItem);

	ExReleaseFastMutex(&g_ProcessFastMutex);

    return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// 名称: FindProcessMonitorItemByProcessName
// 说明: 根据FileObject查找对应的连接上下文结构体指针
// 入参: FileObject 连接上下文文件对象指针
// 返回: 与FileObject 对应的连接上下文结构体指针
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
LPPROC_MONITOR_ITEM FindProcessMonitorItemByProcessName(IN PUNICODE_STRING pImageName)
{
    PLIST_ENTRY pLink = NULL;
    LPPROC_MONITOR_ITEM pResult = NULL;

    if (IsListEmpty(&g_ProcessMonitorList)) {
        DbgMsg(__FILE__, __LINE__, " 根据FileObject查找对应的连接上下文结构体指针 为空.\n");
        return NULL;
    }

    // 遍历整个连接上下文结构体链表

    for (pLink = g_ProcessMonitorList.Flink;
    pLink != &g_ProcessMonitorList;
        pLink = pLink->Flink) {
        LPPROC_MONITOR_ITEM pProcMonitorItem = CONTAINING_RECORD(pLink, PROC_MONITOR_ITEM, ListEntry);
		if(NULL != wcsstr(pImageName->Buffer,pProcMonitorItem->InsertItem.ImageName))
        {
            // 保存查找结果
            pResult = pProcMonitorItem;
            break;
        }
    }

    return pResult;
}
