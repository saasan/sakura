// logoutput.h
#ifndef __LOGOUTPUT_H__
#define __LOGOUTPUT_H__

#include <time.h>
#include <fstream>

using namespace std;

class CLogOutput
{
public:
	CLogOutput(const char *lpszFileName)
	{
		m_strFileName = lpszFileName;
	}

	virtual ~CLogOutput()
	{
	}

	virtual void Output(const char *lpszOutputString)
	{
		if (!m_strFileName.empty())
		{
			string strTime;

			GetTimeString(strTime);

			m_ofs.open(m_strFileName.c_str(), ios::out | ios::app | ios::ate);
			m_ofs << "[" << strTime.c_str() << "] : " << lpszOutputString << endl;
			m_ofs.close();
		}
	}

	virtual void SetFileName(const char *lpszFileName)
	{
		m_strFileName = lpszFileName;
	}

	virtual const char *GetFileName() const
	{
		return m_strFileName.c_str();
	}

private:
	string m_strFileName;
	ofstream m_ofs;

	void GetTimeString(string &strTime)
	{
		const BUF_SIZE = 256;
		time_t now;
		struct tm *sLocal;
		char strBuffer[BUF_SIZE];

		now = time(NULL);
		sLocal = localtime(&now);

		memset(strBuffer, '\0', BUF_SIZE);

		strftime(strBuffer, (BUF_SIZE - 1), "%Y/%m/%d %H:%M:%S", sLocal);
		strTime = strBuffer;
	}
};

#endif // __LOGOUTPUT_H__
