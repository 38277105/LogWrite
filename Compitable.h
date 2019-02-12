/***********************************************************************
 the head file for the interface,by using the Factory mode             *
 for example if you want to use mutex on windows,you need implement a  *
 new class may be CWinMutex which inherit from CCoMutex,then use the   *
 Factory::CreateCoMutex to produce this mutex 
 ***********************************************************************/

#ifndef _COMPITABLE_H
#define _COMPITABLE_H
#include"Comdef.h"
/**
 *the compitable mutex class
*/
class CCoMutex
{
public:
	virtual ~CCoMutex();
	//try to lock the mutex, wait certain time if it can't own the mutex
	virtual long long Lock(unsigned long dwMillionSeconds)=0;
	//give up the ownnership
	virtual int Unlock()=0;
    //release the lock
	virtual int Release()=0;
};
/** 
 * the compitable thread class
*/
class CCoThread
{
public:
	virtual ~CCoThread();
	virtual long long Start() = 0;
	virtual long long Wait() = 0;
	virtual long long Release() = 0;
	virtual void Getid(void *p) = 0;
};
class CCoDir
{
public:
	virtual ~CCoDir();
	virtual char *GetCurDir(char *szName, int nLen) = 0;
	virtual int CreateDir(const char *szName, void *pParam) = 0;
	virtual int DeleteDir(const char *szName) = 0;
};
class  CFactory
{
private:
	CFactory();
public:
	//create mutex base on platform,attr,initial state and name(some param 
	//will be used in certain platform
	static CCoMutex *CreateCoMutex(void *p, int nLocked, const char *szName);
	static CCoThread *CreateCoThread(void *pAttr, start_rtn pRoute, void *pParam, unsigned long dwStack);
	static CCoDir *CreateCoDir();
};

#endif