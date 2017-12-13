//-----------------------------------------------------------------------------
//
//	RadLight APE Decoder
//
//	Author : Igor Janos
//  Last Update : 21-nov-2003
//
//-----------------------------------------------------------------------------
#define PLATFORM_WINDOWS
#include "All.h"
#include "MACLib.h"

#include <streams.h>
#include <wxutil.h>
#include <initguid.h>
#include <stdio.h>

#include "main.h"

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpReserved)
{
	return DllEntryPoint(reinterpret_cast<HINSTANCE>(hDllHandle), dwReason, lpReserved);
} 

// {D042079E-8E02-418b-AE2F-F12E26704FCA}
DEFINE_GUID(CLSID_APEDecoder, 
0xd042079e, 0x8e02, 0x418b, 0xae, 0x2f, 0xf1, 0x2e, 0x26, 0x70, 0x4f, 0xca);

//-----------------------------------------------------------------------------
//
//	Registration Stuff
//
//-----------------------------------------------------------------------------

const AMOVIESETUP_MEDIATYPE sudAPEOutPinTypes =
								{
									&MEDIATYPE_Audio,		// Major type
									&MEDIASUBTYPE_PCM		// Minor type
								};

const AMOVIESETUP_PIN sudAPEOutPin =
								{
									L"PCM Out",				// Pin string name
									FALSE,                  // Is it rendered
									TRUE,                   // Is it an output
									FALSE,                  // Can we have none
									FALSE,                  // Can we have many
									&CLSID_NULL,            // Connects to filter
									NULL,                   // Connects to pin
									1,                      // Number of types
									&sudAPEOutPinTypes		// Pin details
								};       

const AMOVIESETUP_FILTER sudAPEDec =
								{
									&CLSID_APEDecoder,	// Filter CLSID
									L"APE DirectShow Filter",	// String name
									MERIT_NORMAL,			// Filter merit
									1,						// Number pins
									&sudAPEOutPin				// Pin details
								};


CFactoryTemplate g_Templates[] = {
								{ 
									L"APE DirectShow Filter",
									&CLSID_APEDecoder,
									CAPESource::CreateInstance,
									NULL,
									&sudAPEDec
								}
								};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	HRESULT hr = AMovieDllRegisterServer2(TRUE);
	if (FAILED(hr)) return hr;

	// register as APE Source
    TCHAR achTemp[MAX_PATH];

    OLECHAR szCLSID[CHARS_IN_GUID];
    hr = StringFromGUID2(CLSID_APEDecoder, szCLSID, CHARS_IN_GUID);

	ASSERT(SUCCEEDED(hr));

    // vytvorime registry keys pre .APE
    //
    HKEY hkey;

	// .ape
	LONG lr = RegCreateKey(HKEY_CLASSES_ROOT, _T("Media Type\\Extensions\\.ape"), &hkey);
	if (lr != ERROR_SUCCESS) return AmHresultFromWin32(lr);

	wsprintf(achTemp, _T("%ls"), szCLSID);
	lr = RegSetValueEx(hkey, _T("Source Filter"), 0, REG_SZ, (const BYTE *) achTemp, (2 + CHARS_IN_GUID) * sizeof(TCHAR));
	if (lr != ERROR_SUCCESS) return AmHresultFromWin32(lr);
    RegCloseKey(hkey);

	return NOERROR;
}

STDAPI DllUnregisterServer()
{
	HRESULT hr = AMovieDllRegisterServer2(FALSE);
	if (FAILED(hr)) return hr;

	// unregister
	RegDeleteKey(HKEY_CLASSES_ROOT, _T("Media Type\\Extensions\\.ape"));

	return NOERROR;
}


//-----------------------------------------------------------------------------
//
//	CAPESource Implementation
//
//-----------------------------------------------------------------------------

CUnknown *WINAPI CAPESource::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CAPESource(lpunk, phr);
    if (punk == NULL) *phr = E_OUTOFMEMORY;
    return punk;
}

CAPESource::CAPESource(LPUNKNOWN lpunk, HRESULT *phr) 
	:
    CSource(NAME("APE DirectShow Filter"), lpunk, CLSID_APEDecoder),
	m_pFileName(NULL)
{

    CAutoLock cAutoLock(&m_cStateLock);

	// output pins ...
    m_paStreams    = (CSourceStream **) new CAPEStream*[1];
    if (m_paStreams == NULL) {
        *phr = E_OUTOFMEMORY;
	return;
    }

    m_paStreams[0] = new CAPEStream(phr, this, L"PCM Out");
    if (m_paStreams[0] == NULL) {
        *phr = E_OUTOFMEMORY;
	return;
    }
}

CAPESource::~CAPESource()
{
	if (m_pFileName != NULL) {
		free(m_pFileName);
		m_pFileName = NULL;
	}
}

STDMETHODIMP CAPESource::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
	CheckPointer(ppszFileName, E_POINTER);
	*ppszFileName = NULL;

	if (m_pFileName != NULL) {
		DWORD n = sizeof(WCHAR)*(1+lstrlenW(m_pFileName));

		*ppszFileName = (LPOLESTR) CoTaskMemAlloc( n );
		if (*ppszFileName!=NULL) {
			CopyMemory(*ppszFileName, m_pFileName, n);
		}
	}

	if (pmt!=NULL) {
		CopyMediaType(pmt, &(((CAPEStream *)m_paStreams[0])->m_mtOut));
	}

	return NOERROR;
}


STDMETHODIMP CAPESource::Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt)
{
	CheckPointer(lpwszFileName, E_POINTER);

	// lstrlenW is one of the few Unicode functions that works on win95
	int cch = lstrlenW(lpwszFileName) + 1;
	m_pFileName = new WCHAR[cch];
	if (m_pFileName!=NULL) CopyMemory(m_pFileName, lpwszFileName, cch*sizeof(WCHAR));

	// otvorime zvoleny subor
	CAPEStream *pStream = (CAPEStream *)m_paStreams[0];
	HRESULT hr = pStream->OpenAPEFile(m_pFileName);
	
	return hr;
}

STDMETHODIMP CAPESource::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IFileSourceFilter) {
        return GetInterface((IFileSourceFilter *)this, ppv);
    } 

    return CSource::NonDelegatingQueryInterface(riid,ppv);
} 


//-----------------------------------------------------------------------------
//
//	CAPEStream Implementation
//
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Name : CAPEStream::CAPEStream
// Desc : constructor
//-----------------------------------------------------------------------------
CAPEStream::CAPEStream(HRESULT *phr, CAPESource *pParent, LPCWSTR pPinName) 
	:
    CSourceStream(NAME("PCM Out"),phr, pParent, pPinName),
	CSourceSeeking(NAME("APE Seeking"), NULL, phr, &m_csSeeking),
	m_pSource(pParent),
	m_pDecoder(NULL)
{
	// aby nas nereleasli...
	CSourceSeeking::AddRef();

}

//-----------------------------------------------------------------------------
// Name : CAPEStream::~CAPEStream
// Desc : destructor
//-----------------------------------------------------------------------------
CAPEStream::~CAPEStream()
{
	// zatvorime dekoder aj reader
	ReleaseAPEObjects();
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::NonDelegatingQueryInterface
// Desc : rozhadze dalsie interfacesy
//-----------------------------------------------------------------------------
STDMETHODIMP CAPEStream::NonDelegatingQueryInterface(REFIID riid, VOID **ppv)
{
	if (riid == IID_IMediaSeeking) {
		return CSourceSeeking::NonDelegatingQueryInterface(riid, ppv);
	}

	return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::DecideBufferSize
// Desc : urcime dlzku vystupneho bufferu
//-----------------------------------------------------------------------------
HRESULT CAPEStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	ASSERT(pAlloc);
	ASSERT(pRequest);

	// musime mat otvoreny subor
	if (m_pDecoder == NULL) return E_FAIL;

	// urcime dlzku bufferu
	WAVEFORMATEX *pwf = (WAVEFORMATEX *)m_mtOut.Format();
	if (pwf == NULL) return E_FAIL;

    pRequest->cbBuffer	=	pwf->nChannels * WAVE_BUFFER_SIZE;
    pRequest->cBuffers	=	(pwf->nChannels * pwf->nSamplesPerSec * pwf->wBitsPerSample) / 
							(pRequest->cbBuffer * 8); 

    // chcelo by to aspon 1 buffer
    if (pRequest->cBuffers < 1) pRequest->cBuffers = 1;

    // nechame alokator nech nam rezervuje nejaku pamat
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr)) return hr;

    // vratene properties sa mozu lisit...
    if (Actual.cbBuffer < pRequest->cbBuffer) return E_FAIL;

	return NOERROR;
}



//-----------------------------------------------------------------------------
// Name : CAPEStream::CheckMediaType
// Desc : povolime iba nami preferovany typ
//-----------------------------------------------------------------------------
HRESULT CAPEStream::CheckMediaType(const CMediaType *pmt)
{
	if (m_pDecoder == NULL || *pmt != m_mtOut) return E_FAIL;

	return NOERROR;
}


//-----------------------------------------------------------------------------
// Name : CAPEStream::GetMediaType
// Desc : vracia nami preferovany typ
//-----------------------------------------------------------------------------
HRESULT CAPEStream::GetMediaType(CMediaType *pmt)
{
	*pmt = m_mtOut;
	return NOERROR;
}


//-----------------------------------------------------------------------------
// Name : CAPEStream::ReleaseAPEObjects
// Desc : uvolni objekty
//-----------------------------------------------------------------------------
void CAPEStream::ReleaseAPEObjects()
{
	if (m_pDecoder != NULL) {
		delete m_pDecoder;
		m_pDecoder = NULL;
	}
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::OpenAPEFile
// Desc : pokusime sa otvorit subor
//-----------------------------------------------------------------------------
HRESULT CAPEStream::OpenAPEFile(LPWSTR pFileName)
{
	// file already opened ?
	if (m_pDecoder != NULL) return S_FALSE;

	int iRet;

	m_pDecoder = CreateIAPEDecompress(pFileName, &iRet);

	// ak sa nepodarilo otvorit, koncime...
	if (m_pDecoder == NULL) return E_FAIL;
	

	m_dDuration = m_pDecoder->GetInfo(APE::APE_INFO_LENGTH_MS); 


	m_rtDuration = (__int64) ( (double)m_dDuration * (double)10000 );
	m_iBlockSize = m_pDecoder->GetInfo(APE::APE_INFO_CHANNELS) * (m_pDecoder->GetInfo(APE::APE_INFO_BITS_PER_SAMPLE) >> 3);
	m_iTotalBlocks = m_pDecoder->GetInfo(APE::APE_INFO_TOTAL_BLOCKS);


	// Display Information message
//#define DISPLAY_MESSAGE
#ifdef DISPLAY_MESSAGE
	char *str = (char *)malloc(512);
	ZeroMemory(str, 512);

	wsprintf(str, "Length in ms : %d\nBlockSize : %d\nTotal Blocks : %d\n", 
			 m_pDecoder->GetInfo(APE_INFO_LENGTH_MS),
			 m_iBlockSize,
			 m_iTotalBlocks);
	MessageBox(NULL, str, "Info", 0);

	free(str);
#endif /* DISPLAY_MESSAGE */


	// nastavime spravny media type a sme hotovi :)
	m_mtOut.SetType(&MEDIATYPE_Audio);
	m_mtOut.SetSubtype(&MEDIASUBTYPE_PCM);

	WAVEFORMATEX *pwf = (WAVEFORMATEX *)m_mtOut.AllocFormatBuffer(sizeof(WAVEFORMATEX));

	// nastavime wave format
	ZeroMemory(pwf, sizeof(WAVEFORMATEX));

	pwf->nChannels		= m_pDecoder->GetInfo(APE::APE_INFO_CHANNELS);
	pwf->wBitsPerSample = m_pDecoder->GetInfo(APE::APE_INFO_BITS_PER_SAMPLE);
	pwf->nSamplesPerSec	= m_lSampleRate = m_pDecoder->GetInfo(APE::APE_INFO_SAMPLE_RATE);
	pwf->nBlockAlign	= m_pDecoder->GetInfo(APE::APE_INFO_BLOCK_ALIGN);
	pwf->wFormatTag		= WAVE_FORMAT_PCM;
	pwf->nAvgBytesPerSec= pwf->nChannels * (pwf->wBitsPerSample>>3) * pwf->nSamplesPerSec;

	m_mtOut.SetFormatType(&FORMAT_WaveFormatEx);

	m_iBlocksDecoded = 0;

	return NOERROR;
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::FillBuffer
// Desc : vypustime dekodovany zvuk
//-----------------------------------------------------------------------------
HRESULT CAPEStream::FillBuffer(IMediaSample *pSample)
{
	ASSERT(pSample);
	ASSERT(m_pDecoder);

	PBYTE	pData;
	long	lSize;

	pSample->GetPointer(&pData);
	lSize = pSample->GetSize();

	int iBlocksDecoded;

	{
		CAutoLock	Lck(&m_csDecoding);

		m_pDecoder->GetData((char *)pData, lSize / m_iBlockSize, &iBlocksDecoded);

		lSize = iBlocksDecoded * m_iBlockSize;	

		pSample->SetActualDataLength(lSize);
		pSample->SetDiscontinuity(m_bDiscontinuity);
		m_bDiscontinuity = FALSE;

		// time stamps
		CRefTime	rtStart;
		CRefTime	rtStop;

		rtStart = (__int64)( (double) m_iBlocksDecoded * ( (double)10000000 / (double)m_lSampleRate ) );
		rtStart -= m_rtStart;
		rtStop = rtStart + 1;
		pSample->SetTime((REFERENCE_TIME *)&rtStart, (REFERENCE_TIME *)&rtStop);
		m_iBlocksDecoded += iBlocksDecoded;
		
	}

	return (iBlocksDecoded == 0 ? S_FALSE : NOERROR);
	
}


//-----------------------------------------------------------------------------
// Name : CAPEStream::UpdateFromSeek
// Desc : spamata sa po seekovani
//-----------------------------------------------------------------------------
void CAPEStream::UpdateFromSeek()
{
    if (ThreadExists()) {
        DeliverBeginFlush();
        Stop();
        DeliverEndFlush();
        Run();
    }
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::OnThreadStartPlay
// Desc : zavolame new segment
//-----------------------------------------------------------------------------
HRESULT CAPEStream::OnThreadStartPlay()
{
    m_bDiscontinuity = TRUE;
    return DeliverNewSegment(m_rtStart, m_rtStop, m_dRateSeeking);
}


//-----------------------------------------------------------------------------
// Name : CAPEStream::ChangeStart
// Desc : menime start poziciu
//-----------------------------------------------------------------------------
HRESULT CAPEStream::ChangeStart()
{
	if (m_pDecoder == NULL) return S_FALSE;

	unsigned int iBlock = (unsigned int)(((double)m_rtStart / (double)m_rtDuration) * m_iTotalBlocks);
	{
		CAutoLock	lck(&m_csDecoding);
		m_pDecoder->Seek(iBlock);
		m_iBlocksDecoded = iBlock;
	}

    UpdateFromSeek();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name : CAPEStream::ChangeStop
// Desc : menime stop poziciu
//-----------------------------------------------------------------------------
HRESULT CAPEStream::ChangeStop()
{
    UpdateFromSeek();
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name : CAPEStream::ChangeRate
// Desc : menime playrate
//-----------------------------------------------------------------------------
HRESULT CAPEStream::ChangeRate()
{
    {   // Scope for critical section lock.
        CAutoLock cAutoLockSeeking(CSourceSeeking::m_pLock);

        m_dRateSeeking = 1.0;  // zatial nepodporujeme ine playraty ako 1.0
    }
    UpdateFromSeek();
    return S_OK;
}

