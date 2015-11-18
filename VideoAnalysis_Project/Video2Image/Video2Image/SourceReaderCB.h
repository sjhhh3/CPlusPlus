#include <shlwapi.h>

class SourceReaderCB : public IMFSourceReaderCallback
{
public:
	SourceReaderCB(HANDLE hEvent) :
		m_nRefCount(1), m_hEvent(hEvent), m_bEOS(FALSE), m_hrStatus(S_OK)
	{
		InitializeCriticalSection(&m_critsec);
	}

	// IUnknown methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(SourceReaderCB, IMFSourceReaderCallback),
			{ 0 },
		};
		return QISearch(this, qit, iid, ppv);
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_nRefCount);
	}
	STDMETHODIMP_(ULONG) Release()
	{
		ULONG uCount = InterlockedDecrement(&m_nRefCount);
		if (uCount == 0)
		{
			delete this;
		}
		return uCount;
	}

	// IMFSourceReaderCallback methods
	STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
		DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample)
	{
		EnterCriticalSection(&m_critsec);

		if (SUCCEEDED(hrStatus))
		{
			if (pSample)
			{
				// Do something with the sample.
				wprintf(L"Frame @ %I64d\n", llTimestamp);
			}
		}
		else
		{
			// Streaming error.
			NotifyError(hrStatus);
		}

		if (MF_SOURCE_READERF_ENDOFSTREAM & dwStreamFlags)
		{
			// Reached the end of the stream.
			m_bEOS = TRUE;
		}
		m_hrStatus = hrStatus;

		LeaveCriticalSection(&m_critsec);
		SetEvent(m_hEvent);
		return S_OK;
	}

	STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *)
	{
		return S_OK;
	}

	STDMETHODIMP OnFlush(DWORD)
	{
		return S_OK;
	}

public:
	HRESULT Wait(DWORD dwMilliseconds, BOOL *pbEOS)
	{
		*pbEOS = FALSE;

		DWORD dwResult = WaitForSingleObject(m_hEvent, dwMilliseconds);
		if (dwResult == WAIT_TIMEOUT)
		{
			return E_PENDING;
		}
		else if (dwResult != WAIT_OBJECT_0)
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		*pbEOS = m_bEOS;
		return m_hrStatus;
	}

private:

	// Destructor is private. Caller should call Release.
	virtual ~SourceReaderCB()
	{
	}

	void NotifyError(HRESULT hr)
	{
		wprintf(L"Source Reader error: 0x%X\n", hr);
	}

private:
	long                m_nRefCount;        // Reference count.
	CRITICAL_SECTION    m_critsec;
	HANDLE              m_hEvent;
	BOOL                m_bEOS;
	HRESULT             m_hrStatus;

};