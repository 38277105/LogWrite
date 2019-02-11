/***********************************************************************
 * descriptor: implement the mutex in window platform                  *
 * author:    JuanZhou                                                 *
 * date:      2019-1-28                                                *
 ***********************************************************************/
#ifndef _COWINDOWS_H
#define _COWINDOWS_H

#include "Compitable.h"
#ifdef _WINDOWS
#include <SDKDDKVer.h>
#include <windows.h>

#include <stdio.h>
#include <tchar.h>
class CWinMutex: public CCoMutex
{
public:
	    CWinMutex(PVOID pAttr,BOOL bLock,LPCSTR pName,int &nRslt);
		virtual ~CWinMutex();
		virtual LONGLONG Lock(DWORD dwMillionSeconds);
	    virtual int Unlock();
		virtual int Release();
private:
	    HANDLE m_hMutex;
};

class CWinThread : public CCoThread
{
public:
	CWinThread(PVOID pAttr, PVOID pRoute, PVOID pParam, DWORD dwStack);
	~CWinThread();
	LONGLONG Start();
	LONGLONG Wait();
	LONGLONG Release();
	void Getid(PVOID pID);
private:
	DWORD m_dwID;
	HANDLE m_hThread;
	LPSECURITY_ATTRIBUTES m_pAttr;
	LPTHREAD_START_ROUTINE m_pRtn;
	LPVOID m_pParam;
	DWORD m_dwStack;

};
class CWinDir :public CCoDir
{
public:
	char *GetCurDir(char *szName, int nLen);
	BOOL CreateDir(const char *szName, void *pParam);
	BOOL DeleteDir(const char *szName);
};

#endif //end of ifdef _WINDOWS_


#endif