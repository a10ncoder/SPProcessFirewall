#pragma once

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
	WCHAR  ImageName[64];
}PROC_INSERT_ITEM, *LPPROC_INSERT_ITEM;

extern PROC_INSERT_ITEM g_ProcInsertItem;

//
// 进程全局
//
typedef struct _PROC_MONITOR_ITEM_
{
	LIST_ENTRY ListEntry;
	PROC_INSERT_ITEM  InsertItem;
}PROC_MONITOR_ITEM, *LPPROC_MONITOR_ITEM;

// 进程项
extern LPPROC_MONITOR_ITEM   g_ProcMonitorItem;
// 链表
extern LIST_ENTRY            g_ProcessMonitorList;
// 内存池
extern NPAGED_LOOKASIDE_LIST g_ProcessMonitorListPool;
// 初始化同步互斥对象
extern FAST_MUTEX            g_ProcessFastMutex;
//同步事件对象  
extern PRKEVENT              g_EventObject;

//////////////////////////////////////////////////////////////////////////
// 名称: InitializeListManager
// 说明: 初始化链表
// 返回: 初始化结构,返回TRUE表示初始化完成
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
BOOLEAN InitializeListManager();


//////////////////////////////////////////////////////////////////////////
// 名称: UninitializeListManager
// 说明: 删除所有链表对象
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID UninitializeListManager();


//////////////////////////////////////////////////////////////////////////
// 名称: MallocProcessMonitorItem
// 说明: 分配连接上下文结构体内存
// 返回: 分配的连接上下文结构体内存地址
// 备注: 如果分配失败,返回值为NULL
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
LPPROC_MONITOR_ITEM MallocProcessMonitorItem();


//////////////////////////////////////////////////////////////////////////
// 名称: FreeProcessMonitorItem
// 说明: 释放连接上下文相关结构体内存
// 入参: pProcWhiteItem 连接上下文结构体内存
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID FreeProcessMonitorItem(IN LPPROC_MONITOR_ITEM pProcMonitorItem);


//////////////////////////////////////////////////////////////////////////
// 名称: InsertProcessMonitorItemToList
// 说明: 插入到连接上下文链表中
// 入参: pProcWhiteItem 连接上下文结构体内存
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
VOID InsertProcessMonitorItemToList(IN LPPROC_MONITOR_ITEM pProcMonitorItem);


//////////////////////////////////////////////////////////////////////////
// 名称: DeleteProcessMonitorItemFromList
// 说明: 从链表中删除连接上下文结构体内存
// 入参: pProcWhiteItem 要删除的连接上下文指针
// 返回: 操作的结果
// 备注: 元素只有在链表中的时候才其效果
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
BOOLEAN DeleteProcessMonitorItemFromList(IN LPPROC_MONITOR_ITEM pItem);


//////////////////////////////////////////////////////////////////////////
// 名称: FindProcessMonitorItemByProcessName
// 说明: 根据FileObject查找对应的连接上下文结构体指针
// 入参: FileObject 连接上下文文件对象指针
// 返回: 与FileObject 对应的连接上下文结构体指针
// 备注: 
// email: hailunchina@hotmail.com
//////////////////////////////////////////////////////////////////////////
LPPROC_MONITOR_ITEM FindProcessMonitorItemByProcessName(IN PUNICODE_STRING pImageName);

