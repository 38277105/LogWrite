#include"Compitable.h"
#include "CoWindows.h"
#include "CoLinux.h"

CCoMutex::~CCoMutex()
{

}
CCoThread::~CCoThread()
{

}
CFactory::CFactory()
{

}
/**
* create mutex based on different platform by new corresponding mutex class
* @param[in] p: the mutex attributes
* @param[in] nLocked:the original state of this mutex
* @param[in] szName:the name of mutex(just in windows)
*/
CCoMutex *CFactory::CreateCoMutex( void *p, int nLocked, const char *szName)
{
	int nRslt = 0;
	
#ifdef _WINDOWS
		return new CWinMutex(p, nLocked, szName,nRslt);
#endif
	
#ifdef _linux_
		return new CLixMutex(p, nRslt);
#endif
		
	return nullptr;
}

CCoThread *CFactory::CreateCoThread(void *pAttr, void *pRoute, void *pParam, unsigned long dwStack)
{
#ifdef _WINDOWS
	return new CWinThread(pAttr, pRoute, pParam, dwStack);
#endif
#ifdef _linux_
	return new CLixThread(pAttr,pRoute,pParam);
#endif
	return nullptr;
}