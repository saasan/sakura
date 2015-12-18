// exception.h
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

template <class N, class S>
class CException
{
public:
	CException(N nCode, S strMessage)
	{
		m_nCode = nCode;
		m_strMessage = strMessage;
	}

	virtual ~CException()
	{
	}

	virtual N GetCode() const
	{
		return m_nCode;
	}

	virtual void GetMessage(S &strMessage) const
	{
		strMessage = m_strMessage;
	}

private:
	N m_nCode;
	S m_strMessage;
};

DWORD GetLastErrorMessage(CString &str)
{
	LPTSTR lpBuffer;
	DWORD ret;

	ret = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &lpBuffer, NULL);

	if (ret)
	{
		str = lpStr;
		LocalFree(lpBuffer);
	}
	return ret;
}

#endif // __EXCEPTION_H__
