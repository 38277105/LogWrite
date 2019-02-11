
#include "CoWindows.h"

#ifdef _WINDOWS
/**
 * constructor for CWinMutex,the original mutex will be created
 * @param[in] pattr: need to convert to type SECURITY_ATTRIBUTES *
 * @param[in] bLock: the inital owner of the mutex
 *            TRUE : the mutex will be ownned by the creator,any
 *            other thread can't own the mutex
 *            FALSE: one other thread can own this mutex
 * @param[in] pName: the name of this created mutex,could be NULL
 * @param[in out] nRslt: if create mutex success 
              0: success
*/
CWinMutex::CWinMutex(PVOID pattr,BOOL bLock,LPCSTR pName,int &nRslt)
{
	m_hMutex = CreateMutexA((LPSECURITY_ATTRIBUTES)pattr,bLock,pName);
	if(m_hMutex)
	{
		if(ERROR_ALREADY_EXISTS == GetLastError())
		{
			nRslt = eExistMutx;
		}
		else
		{
			nRslt = eSuccess;
		}
	}
	else
	{
		nRslt = GetLastError();
	}
	
}
//------------------------------------------------------------------
/**
 * destructor,the original mutex will be destroy
*/
CWinMutex::~CWinMutex()
{
	if(m_hMutex)
	{
		CloseHandle(m_hMutex);
	}
}
//------------------------------------------------------------------
/**
 * Get the ownnership of original mutex 
 * @param[in] dwMillionSeconds the wait time range while the 
 * mutex is used by other thread before return 
 * 0: judge the state and return immdediately
 * INFINITE: wait like forever,until the mutex is ownned by itself
*/
LONGLONG CWinMutex::Lock(DWORD dwMillionSeconds)
{
	if(m_hMutex)
	{
		return WaitForSingleObject(m_hMutex,dwMillionSeconds);
	}
	return eLockFail;
}
//------------------------------------------------------------------
/**
 * giveup the  ownnership
*/
int CWinMutex::Unlock()
{
	if(m_hMutex)
	{
		return ReleaseMutex(m_hMutex);
	}
	return eUnlockFail;
}
//------------------------------------------------------------------
/**
 * 
*/
int CWinMutex::Release()
{
	if(m_hMutex)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
	}
	return 0;
}
//------------------------------------------------------------------

/**
 * constructor for class CWinThread just save all the params which
 * will be used in Start function
 * @param[in] pAttr: the SECURITY_ATTRIBUTES
 * @param[in] pRoute:the thread exe func
 * @param[in] pParam:the param for exe func
 * @param[in] dwStack:the stack size for this new thread
*/
CWinThread::CWinThread(PVOID pAttr, PVOID pRoute, PVOID pParam, DWORD dwStack):
 m_hThread(0)
,m_dwID(0)
, m_dwStack(dwStack)
, m_pAttr(nullptr)
, m_pParam(m_pParam)
, m_pRtn(nullptr)
{
	m_pAttr = LPSECURITY_ATTRIBUTES(pAttr);
	m_pRtn = LPTHREAD_START_ROUTINE(pRoute);
}
//------------------------------------------------------------------
/**
 * destructor release the original thread
*/
CWinThread::~CWinThread()
{
	Release();
}
//------------------------------------------------------------------
/**
 * create the original thread and run it
*/
LONGLONG CWinThread::Start()
{
	Release();
	m_hThread = CreateThread(m_pAttr, m_dwStack, m_pRtn, m_pParam, 0, &m_dwID);
	if (m_hThread)
	{
		return eSuccess;
	} 
	else
	{
		return GetLastError();
	}
}
//------------------------------------------------------------------
/** 
 * wait untill the thread finish
 * this function is reserved, may be implement it latter
*/
LONGLONG CWinThread::Wait()
{
	
	return eSuccess;
}
//------------------------------------------------------------------
/**
 * Release the thread handle
*/
LONGLONG CWinThread::Release()
{
	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	return eSuccess;
}
//------------------------------------------------------------------
/**
 * Get the thread id
*/
void CWinThread::Getid(PVOID pID)
{
	if (nullptr == pID)
	{
		return;
	}
	DWORD *dwID = (DWORD *)pID;
	*dwID = m_dwID;

}
//------------------------------------------------------------------
/**
* Get directory
*/
char *CWinDir::GetCurDir(char *szName, int nLen)
{
	if (NULL == szName)
	{
		return nullptr;
	}
	GetModuleFileNameA(NULL, szName, nLen);
	return szName;
}
//------------------------------------------------------------------
/**
* Create directory
*/
BOOL CWinDir::CreateDir(const char *szName, void *pParam)
{

	return CreateDirectoryA(szName,(LPSECURITY_ATTRIBUTES)pParam);
}
//------------------------------------------------------------------
/**
* Remove directory
*/
BOOL CWinDir::DeleteDir(const char *szName)
{
	
	return RemoveDirectoryA(szName);
}
//------------------------------------------------------------------
#endif