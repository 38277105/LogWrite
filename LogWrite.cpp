
#include <time.h>
#include <sys/timeb.h>
#include <iostream>
#include "LogWrite.h"
#define ERRLOG  "EROR      "
#define WARLONG "WARN      "
#define INFOLOG "INFO      "
#define DEBUGLOG "DEBU     "
#define LOGPRELEN 7
#define DATELEN 10
#define TIMELEN 8
bool CWriteLog::m_bRun = false;
#define MAX_FILE_SIZE 5*1024*1024 //5MB
#define SIZE "Size"
#define LEVEL "Level"
#define LOG "Log"

std::queue<CWriteLog *> CWriteLog::m_quLog;
//HANDLE CWriteLog::m_hThreadLog = INVALID_HANDLE_VALUE;
CCoThread *CWriteLog::m_pThread = nullptr;

CCoMutex *CWriteLog::m_pMutxLog = NULL;

/** 
 *	constructor
 *	@param[in] szDir  the Directory for log file
 *             may be absolute or relative
 *	@param[in] szName prefix for log file name
*/
CWriteLog::CWriteLog(const char *szDir, const char *szName):
m_nSize(0)
,m_nLevel(-1)
, m_bAdd(false)
, m_szPreName(NULL)
, m_szDir(NULL)
, m_quRec(NULL)
, m_fLog(NULL)
, m_pMutxRec(NULL)
{
	memset(m_szCurDate, 0, sizeof(m_szCurDate));
	size_t sz;
	if (szDir)
	{
		//delete the first and last DIR if it like "\\Log\\" to "Log"
		if (CDIR == szDir[0])
		{
			szDir++;
		}
		sz = strlen(szDir);
		if (sz >0)
		{
			
			if (CDIR != szDir[sz-1])
			{
				sz++;
			}
			m_szDir = new char[sz];
			if (m_szDir)
			{
				memset(m_szDir, 0, sz);
				strncpy(m_szDir, szDir, sz - 1);
			}
		}
			
	}
	if (szName)
	{
		sz = strlen(szName) + 1;
		m_szPreName = new char[sz];
		if (m_szPreName)
		{
			memset(m_szPreName, 0, sz);
			strncpy(m_szPreName, szName, sz - 1);
		}
	}
	static bool bInitLogCr = false;
	if (false == bInitLogCr)
	{
		//InitializeCriticalSection(&m_crLog);
		bInitLogCr = true;
		m_pMutxLog = CFactory::CreateCoMutex(NULL, FALSE, NULL);
	}
	//InitializeCriticalSection(&m_crRec);
	m_pMutxRec = CFactory::CreateCoMutex(NULL, FALSE, NULL);
	ReadLevelAndSize();
}
//------------------------------------------------------------
/** 
 *	stop the Thread and write all the log records remained
*/
CWriteLog::~CWriteLog()
{
	
	DelObjFromQueue(this);
	Stop();
	//
	if (m_fLog)
	{
		WriteLog(true);
		if (m_fLog->is_open())
		{
			m_fLog->close();
		}
		delete m_fLog;
		m_fLog = NULL;
	}
	//EnterCriticalSection(&m_crRec);
	if (m_pMutxRec)
	{
		m_pMutxRec->Lock(INFINITE);
	}
	if (m_szDir)
	{
		delete[] m_szDir;
		m_szDir = NULL;
	}
	if (m_szPreName)
	{
		delete[] m_szPreName;
		m_szPreName = NULL;
	}
	
	if (m_quRec)
	{
		delete m_quRec;
		m_quRec = NULL;
	}
	//LeaveCriticalSection(&m_crRec);
	if (m_pMutxRec)
	{
		m_pMutxRec->Unlock();
		delete m_pMutxRec;
		m_pMutxRec = NULL;
	}
	if (m_pThread)
	{
		m_pThread->Wait();
		m_pThread->Release();
		delete m_pThread;
		m_pThread = nullptr;
	}
	
}
//------------------------------------------------------------
/**
 *	put the log information in the log queue
 *	@param[in] eLevel  the log level of current record
 *  @param[in] szFormat the sprintf format of log information
*/
int CWriteLog::Log(LogLevel eLevel, const char *szFormat, ...)
{
	if (eLevel > m_nLevel)
	{
		return 0;
	}
	if (NULL == m_quRec)
	{
		m_quRec = new std::queue< std::string >;
		if (NULL == m_quRec)
		{
			return -1;
		}
	}
	//join to the log-queue 
	if (false == m_bAdd)
	{
		m_bAdd = true;
		if (NULL == m_fLog)
		{
			m_fLog = new std::ofstream();
		}
		//EnterCriticalSection(&m_crLog);
		if (m_pMutxLog)
		{
			m_pMutxLog->Lock(INFINITE);
		}
		/*if (INVALID_HANDLE_VALUE == m_hThreadLog)





















































































		{
			DWORD dwId = 0;
			m_bRun = true;
			m_hThreadLog = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadWriteLog, 0, 0, &dwId);
		}*/
		if (nullptr == m_pThread)
		{
			m_pThread = CFactory::CreateCoThread(NULL, ThreadWriteLog, nullptr, 0);
			LONGLONG lRst = 0;
			if (m_pThread)
			{
				lRst = m_pThread->Start();
			}
			if (eSuccess != lRst)
			{
				delete m_pThread;
				m_pThread = nullptr;
				
			}
			else
			{
				m_bRun = true;
			}
		}
		
		m_quLog.push(this);
		//LeaveCriticalSection(&m_crLog);
		if (m_pMutxLog)
		{
			m_pMutxLog->Unlock();
		}
	}
	//create the loginf and put it in rec-que
	char szBuf[512] = { 0 };
	switch (eLevel)
	{
	case ErrLog:
	{
		strncpy(szBuf, ERRLOG, LOGPRELEN);
		break;
	}
	case WarnLog:
	{
		strncpy(szBuf, WARLONG, LOGPRELEN);
		break;
	}
	case InfoLog:
	{
		strncpy(szBuf, INFOLOG, LOGPRELEN);
		break;
	}
	case DebugLog:
	{
		strncpy(szBuf, DEBUGLOG, LOGPRELEN);
		break;
	}
	default:
	{
		strncpy(szBuf, INFOLOG, LOGPRELEN);
		break;
	}
	}
	va_list argptr;
	va_start(argptr, szFormat);
	vsnprintf(szBuf + LOGPRELEN, sizeof(szBuf), szFormat, argptr);
	va_end(argptr);
	timeb t;
	ftime(&t);
	struct tm* ptm = localtime(&t.time);
	char szTime[32] = { 0 };
	char szWrite[512] = { 0 };
	_snprintf(szWrite, sizeof(szWrite), "%04d-%02d-%02d %02d-%02d-%02d-%04d   %s\n", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, t.millitm, szBuf);
	strncpy(m_szCurDate, szWrite, DATELEN);
	//EnterCriticalSection(&m_crRec);
	if (m_pMutxRec)
	{
		m_pMutxRec->Lock(INFINITE);
	}
	m_quRec->push(szWrite);
	//LeaveCriticalSection(&m_crRec);
	if (m_pMutxRec)
	{
		m_pMutxRec->Unlock();
	}
	return 1;
}
//------------------------------------------------------------
/**
 *	open one log file for writing
 *  @param[in] bCheckHistory  
 *		true: check the last file in current date if
 *             it's size leagal open the last file
 *      false: just open a new name with current time
*/
bool CWriteLog::OpenFile(std::string & strInf, bool bCheckHistory)
{
	char szPath[512] = { 0 };
	//GetModuleFileNameA(NULL, szPath, sizeof(szPath));
	

	CCoDir *pDir = CFactory::CreateCoDir();
	if (!pDir)
	{
		return false;
	}
	pDir->GetCurDir(szPath, sizeof(szPath));
	//GetModuleFileNameA(NULL, szPath, sizeof(szPath));
	char *sz = strrchr(szPath, CDIR);

	if (sz)
	{
		sz[0] = 0;
	}
	strncat(szPath, DIR, 1);
	
	//Log path
	if (m_szDir)
	{
		strncat(szPath, m_szDir, strlen(m_szDir));
	}
	else
	{
		strncat(szPath, LOG, strlen(LOG));
	}
	strncat(szPath, DIR, 1);
	//CreateDirectoryA(szPath, NULL);
	pDir->CreateDir(szPath, NULL);
	char szDate[DATELEN+1] = { 0 };
	char szTime[TIMELEN+1] = { 0 };
	//the Date path
	if (strlen(strInf.c_str()) > DATELEN)
	{
		strncpy(szDate, strInf.c_str(), DATELEN);
	} 
	else
	{
		time_t t = time(NULL);
		tm *ptm = localtime(&t);
		_snprintf(szDate, sizeof(szDate), "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
		_snprintf(szTime, sizeof(szTime), "%02d-%02d-%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	}
	if (strlen(strInf.c_str()) > DATELEN + TIMELEN +1)
	{
		strncpy(szTime, strInf.c_str() + DATELEN + 1,TIMELEN);
	}
	strncat(szPath, szDate, DATELEN);
	strncat(szPath, DIR, 1);
	/*
	if (bCheckHistory)
	{
		char szLogFile[512] = { 0 };
		memcpy(szLogFile, szPath, strlen(szPath));
		//the log file name
		if (m_szPreName)
		{
			strncat(szLogFile, m_szPreName, strlen(m_szPreName));
		}
		strncat(szLogFile, "??-??-??.txt", strlen("??-??-??.txt"));
		WIN32_FIND_DATAA  fData;
		HANDLE h = FindFirstFileA(szLogFile, &fData);
		if (INVALID_HANDLE_VALUE != h)
		{
			char szFile[124] = { 0 };
			memset(szLogFile, 0, sizeof(szLogFile));
			strncpy(szLogFile, szPath, strlen(szPath));
			while (FindNextFileA(h,&fData))
			{
				memset(szFile, 0, sizeof(szFile));
				strncpy(szFile, fData.cFileName, strlen(fData.cFileName));
			}
			strncat(szLogFile, szFile, strlen(szFile));
			m_fLog->open(szLogFile, std::ios::app);
			m_fLog->seekp(0, std::ios::end);
			size_t s = m_fLog->tellp();
			if (s < m_nSize)
			{
				//std::cout << "check last file is valid" << std::endl;
				return true;
			}
			else
			{
				m_fLog->close();
			}
		}	
	}
	*/
	//CreateDirectoryA(szPath, NULL);
	pDir->CreateDir(szPath, NULL);
	if (m_szPreName)
	{
		strncat(szPath, m_szPreName, strlen(m_szPreName));
	}
	//time inf
	strncat(szPath, szTime, TIMELEN);

	strncat(szPath, ".txt", strlen(".txt"));
	m_fLog->open(szPath,std::ios::app);
	
	return true;
	
}
//------------------------------------------------------------
/**
 *	read the log level and file size from config.ini
*/
void CWriteLog::ReadLevelAndSize()
{
	char szPath[512] = { 0 };
	GetModuleFileNameA(NULL, szPath, sizeof(szPath));
	char *sz = strrchr(szPath, CDIR);
	if (sz)
	{
		sz[0] = 0;
	}
	strncat(szPath, DIR, strlen(DIR));
	strncat(szPath, "config.ini", strlen("config.ini"));
	char szApp[32] = { 0 };
	char szKey[128] = { 0 };
	//global config
	memcpy(szApp, LOG, strlen(LOG));
	//file size
	memcpy(szKey, SIZE, strlen(SIZE));
	m_nSize = GetPrivateProfileIntA(szApp, szKey, MAX_FILE_SIZE, szPath);
	//file level
	strncpy(szKey, LEVEL, strlen(LEVEL));
	m_nLevel = GetPrivateProfileIntA(szApp, szKey, DebugLog, szPath);
	//-----------end of global config
	//certain config
	if (m_szPreName)
	{
		//file size
		memset(szKey, 0, sizeof(szKey));
		strncpy(szKey, m_szPreName, strlen(m_szPreName));
		strncat(szKey, SIZE, strlen(SIZE));
		m_nSize = GetPrivateProfileIntA(szApp, szKey, m_nSize, szPath);
		//file level
		memset(szKey, 0, sizeof(szKey));
		strncpy(szKey, m_szPreName, strlen(m_szPreName));
		strncat(szKey, LEVEL, strlen(LEVEL));
		m_nLevel = GetPrivateProfileIntA(szApp, szKey, m_nLevel, szPath);
	}
	//-----------end of certain config
}
//------------------------------------------------------------
/**
 *	write the rec to log file(need to sure the logfile is opened)
 *	@param[in] bAllRec 
 *			   false:just write the first rec in the queue
 *			   true :write all the rec
*/
void CWriteLog::WriteLog(bool bAllRec)
{
	//EnterCriticalSection(&m_crRec);
	if (m_pMutxRec)
	{
		m_pMutxRec->Lock(INFINITE);
	}
	if (NULL == m_quRec)
	{
		//LeaveCriticalSection(&m_crRec);
		if (m_pMutxRec)
		{
			m_pMutxRec->Unlock();
		}
		return;
	}
	if (0 == m_quRec->size())
	{
		//LeaveCriticalSection(&m_crRec);
		if (m_pMutxRec)
		{
			m_pMutxRec->Unlock();
		}
		return;
	}
	
	if (bAllRec)
	{
		//LeaveCriticalSection(&m_crRec);
		if (m_pMutxRec)
		{
			m_pMutxRec->Unlock();
		}
		int nSize = 0;
		std::string str("");
		do
		{
			//EnterCriticalSection(&m_crRec);
			if (m_pMutxRec)
			{
				m_pMutxRec->Lock(INFINITE);
			}
			str = m_quRec->front();
			m_quRec->pop();
			nSize = m_quRec->size();
			//LeaveCriticalSection(&m_crRec);
			if (m_pMutxRec)
			{
				m_pMutxRec->Unlock();
			}
			WriteLog(str);
		} while (nSize > 0);
	} 
	else
	{
		std::string str =m_quRec->front();
		m_quRec->pop();
		//LeaveCriticalSection(&m_crRec);
		if (m_pMutxRec)
		{
			m_pMutxRec->Unlock();
		}
		WriteLog(str);	
	}
	
	
}
//------------------------------------------------------------
void CWriteLog::WriteLog(std::string & strLogInf)
{
	if (m_fLog->is_open())
	{
		int n = strncmp(m_szCurDate, strLogInf.c_str(), DATELEN);
		if (0 != n || m_fLog->tellp()>m_nSize)
		{
			
			m_fLog->close();
			OpenFile(strLogInf);
		}
	} 
	else
	{
		OpenFile(strLogInf, true);
	}
	if (m_fLog->is_open())
	{
		*m_fLog << strLogInf.c_str()+DATELEN+1;
		m_fLog->flush();
	}
}
/**
 *	knick the logobj from the m_quLog
 *	@param[in] pObj the object which will be deleted from the queue
*/
void CWriteLog::DelObjFromQueue(CWriteLog *pObj)
{
	
	//EnterCriticalSection(&m_crLog);
	if (m_pMutxLog)
	{
		m_pMutxLog->Lock(INFINITE);
	}
	int nSize = m_quLog.size();
	for (size_t i = 0; i < nSize; i++)
	{
		CWriteLog *p = m_quLog.front();
		m_quLog.pop();
		if (p == pObj)
		{
			//LeaveCriticalSection(&m_crLog);
			if (m_pMutxLog)
			{
				m_pMutxLog->Unlock();
			}
			return;
		}
		else
		{
			m_quLog.push(p);
		}
	}

	//LeaveCriticalSection(&m_crLog);
	if (m_pMutxLog)
	{
		m_pMutxLog->Unlock();
	}
}
//------------------------------------------------------------
/**
 *	
*/
void CWriteLog::ThreadWriteLog(void *pParam)
{
	while (true == m_bRun)
	{
		//EnterCriticalSection(&m_crLog);
		if (m_pMutxLog)
		{
			m_pMutxLog->Lock(INFINITE);
		}
		if (0 != m_quLog.size())
		{
			CWriteLog *pLog = m_quLog.front();
			m_quLog.pop();
			m_quLog.push(pLog);
			if (pLog)
			{
				pLog->WriteLog();
			}		
		}
		
		//LeaveCriticalSection(&m_crLog);
		if (m_pMutxLog)
		{
			m_pMutxLog->Unlock();
		}
		
	}
	//EnterCriticalSection(&m_crLog);
	if (m_pMutxLog)
	{
		m_pMutxLog->Lock(INFINITE);
	}
	//CloseHandle(m_hThreadLog);
	//m_hThreadLog = INVALID_HANDLE_VALUE;
	if (m_pThread)
	{
		m_pThread->Release();
	}
	//LeaveCriticalSection(&m_crLog);
	if (m_pMutxLog)
	{
		m_pMutxLog->Unlock();
	}
}
//------------------------------------------------------------
/**
*
*/
int CWriteLog::Stop()
{
	m_bRun = false;
	return 1;
}
