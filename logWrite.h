/** 
 *	class: CWriteLog 
 *	author: Juan Zhou
 *	date:   2017-09-28 2017-10-03
*/
#ifndef WRITE_LOG_H
#define WRITE_LOG_H
#include <fstream>
#include <string>
#include <queue>
#include "Compitable.h"


enum LogLevel
{
	ErrLog = 0,
	WarnLog = 1,
	InfoLog = 2,
	DebugLog = 3

};
class DLL_API CWriteLog
{
public:
	CWriteLog(const char *szDir, const char *szName);
	~CWriteLog();
	 //push log rec into log queue
	 int Log(LogLevel eLevel, const char *szFormat, ...);
	 //stop the log thread,used befor end any program who used the logdll
	 static int Stop();
private:
	bool OpenFile(std::string & strInf, bool bCheckHistory = false);
	void ReadLevelAndSize();
	void WriteLog(bool bAllRec = false);
	void WriteLog(std::string & strLogInf);
	void DelObjFromQueue(CWriteLog *pObj);
private:
	int m_nSize;					//file size
	int m_nLevel;					//the log level from .ini file(or by default)
	bool m_bAdd;
	char m_szCurDate[11];			//the date-string of current logfile(format YYYY-MM-DD)
	char* m_szPreName;				//the prefix for log file name
	char* m_szDir;					//the dir for log file
	std::ofstream *m_fLog;			//the log file

	CCoMutex *m_pMutxRec;

	std::queue< std::string > *m_quRec;//the information waitting to be writed to file

	static bool m_bRun;				//flag who mark the thread is run
	
	//static HANDLE m_hThreadLog;		//the handle for write log thread
	static CCoThread *m_pThread;    //the thread pointer that will be used in 
	                                //thread-related operation
	static std::queue<CWriteLog *> m_quLog;		//log object queue
	static void ThreadWriteLog(void *pParam);	//Thread to write log

	static CCoMutex *m_pMutxLog;    //to synchronize the logfile
	

};
#endif