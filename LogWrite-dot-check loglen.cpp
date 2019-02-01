// LogWrite.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <time.h>
#include <sys/timeb.h>
#include "logWrite.h"
#define ERRLOG  "EROR      "
#define WARLONG "WARN      "
#define INFOLOG "INFO      "
#define DEBUGLOG "DEBU     "
#define LOGPRELEN 7
bool CWriteLog::m_bRun = false;
#define MAX_FILE_SIZE 5*1024*1024 //5MB
#define SIZE "Size"
#define LEVEL "Level"
#define LOG "Log"
#define DIR "\\"
std::queue<CWriteLog *> CWriteLog::m_quLog;
HANDLE CWriteLog::m_hThreadLog = INVALID_HANDLE_VALUE;
CRITICAL_SECTION CWriteLog::m_crLog;

/** 
 *	constructor
 *	@param[in] szDir  the Directory for log file
 *             may be absolute or relative
 *	@param[in] szName prefix for log file name
*/
CWriteLog::CWriteLog(const char *szDir, const char *szName):
m_nSize(0)
,m_nLevel(-1)
, m_szPreName(NULL)
, m_szDir(NULL)
, m_quRec(NULL)
, m_fLog(NULL)
{
	memset(m_szCurDate, 0, sizeof(m_szCurDate));
	size_t sz;
	if (szDir)
	{
		//delete the first and last DIR if it like "\\Log\\" to "Log"
		if ('\\' == szDir[0])
		{
			szDir++;
		}
		sz = strlen(szDir);
		if (sz >0)
		{
			
			if ('\\' != szDir[sz-1])
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
	InitializeCriticalSection(&m_crRec);
	ReadLevelAndSize();
}
//------------------------------------------------------------
/** 
 *	stop the Thread and write all the log records remained
*/
CWriteLog::~CWriteLog()
{
	
	DelObjFromQueue(this);
	if (0 == m_quLog.size())
	{
		m_bRun = false;
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
	if (m_fLog)
	{
		if (m_quRec && m_quRec->size() > 0)
		{
			WriteLog(true);
		}
		if (m_fLog->is_open())
		{
			m_fLog->close();
		}
		delete m_fLog;
		m_fLog = NULL;
	}
	if (m_quRec)
	{
		delete m_quRec;
		m_quRec = NULL;
	}
	DeleteCriticalSection(&m_crRec);
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
	static bool bAddInQueue = false;
	if (false == bAddInQueue)
	{
		if (NULL == m_fLog)
		{
			m_fLog = new std::ofstream();
		}
		if (INVALID_HANDLE_VALUE == m_hThreadLog)
		{
			InitializeCriticalSection(&m_crLog);
			DWORD dwId = 0;
			m_bRun = true;
			m_hThreadLog = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadWriteLog, 0, 0, &dwId);
		}
		EnterCriticalSection(&m_crLog);
		m_quLog.push(this);
		LeaveCriticalSection(&m_crLog);
		bAddInQueue = true;
	
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
	
	EnterCriticalSection(&m_crRec);
	if (m_quRec->size() > 1000)
	{
		return -2;
	} 
	else
	{
		m_quRec->push(szWrite);
	}
	LeaveCriticalSection(&m_crRec);
	
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
	GetModuleFileNameA(NULL, szPath, sizeof(szPath));
	char *sz = strrchr(szPath, '\\');
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
	CreateDirectoryA(szPath, NULL);
	//the Date path
	strncat(szPath, strInf.c_str(), 10);
	strncat(szPath, DIR, 1);
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
				return true;
			}
		}	
	}
	
	CreateDirectoryA(szPath, NULL);
	if (m_szPreName)
	{
		strncat(szPath, m_szPreName, strlen(m_szPreName));
	}
	//time inf
	strncat(szPath, strInf.c_str() + 11, 8);

	strncat(szPath, ".txt", strlen(".txt"));
	m_fLog->open(szPath);
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
	char *sz = strrchr(szPath, '\\');
	if (sz)
	{
		sz[0] = 0;
	}
	strncat(szPath, "\\config.ini", strlen("\\config.ini"));
	char szApp[32] = { 0 };
	char szKey[128] = { 0 };
	//global config
	memcpy(szApp, LOG, strlen(LOG));
	//file size
	memcpy(szKey, SIZE, strlen(SIZE));
	m_nSize = GetPrivateProfileIntA(szApp, szKey, MAX_FILE_SIZE, szPath);
	//file level
	strncpy(szKey, LEVEL, strlen(LEVEL));
	m_nLevel = GetPrivateProfileIntA(szApp, szKey, WarnLog, szPath);
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
	EnterCriticalSection(&m_crRec);
	if (m_quRec->size()>0)
	{
		if (bAllRec)
		{
			while (m_quRec->size()>0)
			{
				WriteLog(m_quRec->front());
				m_quRec->pop();
			}
		} 
		else
		{
			WriteLog(m_quRec->front());
			m_quRec->pop();
		}
	}
	LeaveCriticalSection(&m_crRec);
}
//------------------------------------------------------------
void CWriteLog::WriteLog(std::string & strLogInf)
{
	if (m_fLog->is_open())
	{
		int n = strncmp(m_szCurDate, strLogInf.c_str(), sizeof(m_szCurDate) - 1);
		if (0 != n || m_fLog->tellp()>m_nSize)
		{
			OpenFile(strLogInf);
		}
	} 
	else
	{
		OpenFile(strLogInf, true);
	}
	if (m_fLog->is_open())
	{
		*m_fLog << strLogInf.c_str();
		m_fLog->flush();
	}
}
/**
 *	knick the logobj from the m_quLog
 *	@param[in] pObj the object which will be deleted from the queue
*/
void CWriteLog::DelObjFromQueue(CWriteLog *pObj)
{
	
	EnterCriticalSection(&m_crLog);
	int nSize = m_quLog.size();
	for (size_t i = 0; i < nSize; i++)
	{
		CWriteLog *p = m_quLog.front();
		m_quLog.pop();
		if (p == pObj)
		{
			return;
		}
		else
		{
			m_quLog.push(p);
		}
	}

	LeaveCriticalSection(&m_crLog);
}
//------------------------------------------------------------
/**
 *	
*/
void CWriteLog::ThreadWriteLog(void *pParam)
{
	while (true == m_bRun)
	{
		EnterCriticalSection(&m_crLog);
		if (0 == m_quLog.size())
		{
			LeaveCriticalSection(&m_crLog);
			return;
		}
		CWriteLog *pLog = m_quLog.front();
		if (pLog)
		{
			pLog->WriteLog();
			m_quLog.pop();
			m_quLog.push(pLog);
		}
		
		LeaveCriticalSection(&m_crLog);
	}
	DeleteCriticalSection(&m_crLog);
}
