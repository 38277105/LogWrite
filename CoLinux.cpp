#ifdef _linux_
#include "CoLinux.h"
/**
 * constructor for CLixMutex,first alloc mem for pthread_mutex_t
 * then use pthread_mutex_init func to initialize it
 * @param[in] pattr: need to convert to type pthread_mutexattr_t *
 * @param[in out] nRslt: if success for creating and initialization
 *       original mutex
 *       0: success
*/
CLixMutex::CLixMutex(PVOID pattr,int &nRslt):m_pMutex(NULL)
{
	m_pMutex = alloc(sizeof(pthread_mutex_t));
	if(NULL == m_pMutex)
	{
		return eNomem;
	}
	pthread_mutexattr_t *patr = (pthread_mutexattr_t*)pattr;
	nRslt = pthread_mutex_init(m_pMutex,patr);
}
//------------------------------------------------------------------
/**
 * destructor needs to destroy and free the space for original mutex
 *
*/
CLixMutex::~CLixMutex()
{
	if(m_pMutex)
	{
		pthread_mutex_destroy(m_pMutex);
		free(m_pMutex);
		m_pMutex = NULL;
	}
}
//------------------------------------------------------------------
/**
 * lock the original mutex
 * @param[in] dwMillionSeconds will not be used,just used in windows
*/
LONGLONG CLixMutex::Lock(DWORD dwMillionSeconds)
{
	if(m_pMutex)
	{
		return pthread_mutex_lock(m_pMutex);
	}
	return eNullMutx;
}
//------------------------------------------------------------------
/**
 * unlock the original mutex
*/
int CLixMutex::Unlock()
{
	if(m_pMutex)
	{
		return pthread_mutex_unlock(m_pMutex);
	}
	return eNullMutx;
}
//------------------------------------------------------------------
/**
 * destroy the original mutex
*/
int CLixMutex::Release()
{
	if(m_pMutex)
	{
		return pthread_mutex_destroy(m_pMutex);
	}
	return eNullMutx;
}
//------------------------------------------------------------------
/**
* constructor for class CWinThread just save all the params which
* will be used in Start function
* @param[in] pAttr: the thread attributes
* @param[in] pRoute:the thread exe func
* @param[in] pParam:the param for exe func
*/
CLixThread::CLixThread(PVOID pAttr,PVOID pRoute,PVOID pParam):
m_id(0)
,m_pAttr(NULL)
,m_pParam(pParam)
,m_pRtn(NULL)
{
	m_pAttr = (pthread_attr_t *)pAttr;
	m_pRtn = (start_rtn)pRoute;
}
CLixThread::~CLixThread()
{
	Release();
}
/**
* create the original thread and run it
*/
LONGLONG CLixThread::Start()
{
	Release();
	return pthread_create(&m_id,m_pAttr,m_pRtn,m_pParam);
}
//------------------------------------------------------------------
/** 
* wait untill the thread finish
*/
LONGLONG CLixThread::Wait()
{
	if (m_id)
	{
		return pthread_join(m_id,NULL);
	}
	return eSuccess;
}
//------------------------------------------------------------------
/**
* Release the thread handle
*/
LONGLONG CLixThread::Release()
{
	if (m_id)
	{
		int nRslt = pthread_cancel(m_id);
		m_id = 0;
		return nRslt;
	}
	return eSuccess;
}
//------------------------------------------------------------------
/**
* Get the thread id
*/
void CLixThread::Getid(PVOID pID)
{
	if (pID)
	{
		pthread_t * p = (pthread_t *)pID;
		*p = m_id;
	}
	
}
//------------------------------------------------------------------
/**
* Get directory
*/
char *CLixDir::GetCurDir(char *szName, int nLen)
{
	return getcwd(szName,nLen);
}
//------------------------------------------------------------------
/**
* Create directory
*/
BOOL CLixDir::CreateDir(const char *szName, void *pParam)
{
	if (0 == mkdir(szName,S_IRWXU | S_IRWXG | S_IRWXO))
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}
//------------------------------------------------------------------
/**
* Remove directory
*/
BOOL CLixDir::DeleteDir(const char *szName)
{
	if (0 == rmdir(szName))
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}
//------------------------------------------------------------------
#endif