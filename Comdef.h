#ifndef _COMDEF_H
#define _COMDEF_H

/************************************************************************
 * include all head files platform-related
 * define all the macro and retype all the types                        *
 * which are differ in different platforms                              *
************************************************************************/
//head files for linux
#ifdef _linux_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#endif
//head files for windows
#ifdef _WINDOWS
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#endif
//for type and macro
#ifdef _linux_
#define DIR "/"
#define CDIR '/'
#define INVALID_HANDLE_VALUE   -1
#define _MAX_PATH           260 /* max. length of full pathname */


#define MAX_PATH            260
#define TRUE                true 
#define FALSE               false
//#define __stdcall
//#define __declspec(x)  
//#define __cdecl 
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#define DLL_API 
#define INFINITE 0xFFFFFF
#define RETURNVALUE 0

#else //windows
#define DIR "\\"	
#define CDIR '\\'
#define RETURNVALUE 0
typedef DWORD RETURNTYPE;
#ifdef DLLEXPORT
#define DLL_API   __declspec(dllexport)
#else
#define DLL_API   __declspec(dllimport)
#endif
#endif
//end for type and macro
#ifdef _linux_


typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef float               FLOAT;
typedef FLOAT               *PFLOAT;
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef unsigned char       *PUCHAR;
typedef short               SHORT;
typedef unsigned short      USHORT;
typedef unsigned short      *PUSHORT;
typedef long                LONG;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef ULONGLONG           *PULONGLONG;
typedef unsigned long       ULONG;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int        *PUINT;
typedef void                VOID;

typedef char                *LPSTR;
typedef const char          *LPCSTR;
typedef wchar_t             WCHAR;
typedef WCHAR               *LPWSTR;
typedef const WCHAR         *LPCWSTR;
typedef unsigned long       UINT_PTR;
typedef UINT_PTR            SIZE_T;
typedef LONGLONG            USN;
typedef BYTE                BOOLEAN;
typedef void                *PVOID;
typedef DWORD               *LPDWORD;
typedef void *RETURNTYPE;
#endif
typedef RETURNTYPE(*start_rtn)(void *);
enum EPlatform
{
	eWindows = 0,
	eLinux = 1,
	eMac = 2,
	eAndroid = 3,
	eIOS = 4,
	eUnknowPlatform = -1
};

enum EErr
{
	eSuccess = 0,
	eNomem = 1,
	eMutxIni = 2,
	eNullMutx = 3,
	eExistMutx = 4,
	eLockFail = 5,
	eUnlockFail = 6,
};
















#endif