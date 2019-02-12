/***********************************************************************
 * descriptor: implement the mutex in linux platform                   *
 * author:    JuanZhou                                                 *
 * date:      2019-1-28                                                *
 ***********************************************************************/
#ifndef _COLINUX_H
#define _COLINUX_H

#ifdef _linux_
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include "Compitable.h"
typedef void *(*start_rtn) (void *);
class CLixMutex : public CCoMutex
{
	public:
	    CLixMutex(PVOID pattr,int &nRslt);
		virtual ~CLixMutex();
		virtual LONGLONG Lock(DWORD dwMillionSeconds);
	    virtual int Unlock();
		virtual int Release();
    private:
	    pthread_mutex_t *m_pMutex;
};
class CLixThread: public CCoThread
{
public:
	CLixThread(PVOID pAttr,PVOID pRoute,PVOID pParam);
	~CLixThread();
	LONGLONG Start();
	LONGLONG Wait();
	LONGLONG Release();
	void Getid(PVOID pID);
private:
	pthread_t m_id;
	pthread_attr_t *m_pAttr;
	start_rtn m_pRtn;
	PVOID m_pParam;
};


class CLixDir:public CCoDir
{
public:
	char *GetCurDir(char *szName, int nLen);
	BOOL CreateDir(const char *szName, void *pParam);
	BOOL DeleteDir(const char *szName);
};
#endif



#endif