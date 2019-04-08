#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0601	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{

#endif

#include <Ntifs.h>
#include <ntddk.h>
#include <ntddstor.h>
#include <mountdev.h>
#include <ntddvol.h>
#include <devioctl.h>
#include <ntstrsafe.h>
#include <WinDef.h>

#include "ListManager.h"

#ifdef __cplusplus

}
#endif

void DbgMsg(char *lpszFile, 
	int Line, 
	char *lpszMsg, ...);


//内存申请
#define kmalloc(size)         ExAllocatePoolWithTag( NonPagedPool, size, 'root' )
#define kfree(ptr)            ExFreePoolWithTag( ptr, 'root' )
