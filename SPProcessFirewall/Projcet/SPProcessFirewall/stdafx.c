//This file is used to build a precompiled header
#include "stdafx.h"

#define _countof(array) (sizeof(array) / sizeof(array[0]))

void DbgMsg(char *lpszFile, int Line, char *lpszMsg, ...)
{
	UNREFERENCED_PARAMETER(lpszFile);
	UNREFERENCED_PARAMETER(Line);
	UNREFERENCED_PARAMETER(lpszMsg);
#ifdef DBG
	char szBuff[0x100], szOutBuff[0x100];
	va_list mylist;

	va_start(mylist, lpszMsg);
	RtlStringCbVPrintfA( szBuff, _countof(szBuff), lpszMsg, mylist);
	va_end(mylist);

	RtlStringCbPrintfA( szOutBuff, _countof(szOutBuff), "%s(%d) : %s", lpszFile, Line, szBuff);

	DbgPrint(szOutBuff);
#endif
}