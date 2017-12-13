//-----------------------------------------------------------------------------
//
//	RadLight APE Decoder
//
//	Author : Igor Janos
//  Last Update : 21-nov-2003
//
//-----------------------------------------------------------------------------

#ifndef MAIN_H
#define MAIN_H

const int WAVE_BUFFER_SIZE = 4096; 

//-----------------------------------------------------------------------------
//
//	CAPESource Class
//
//-----------------------------------------------------------------------------

class CAPESource : public CSource,
				   public IFileSourceFilter
{
	friend class CAPEStream;
private:

	LPWSTR				m_pFileName;

public:
	CAPESource(LPUNKNOWN lpunk, HRESULT *phr);
	~CAPESource();
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);

	DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

	// --- IFileSourceFilter methods
    STDMETHODIMP Load(LPCOLESTR lpwszFileName, const AM_MEDIA_TYPE *pmt);
    STDMETHODIMP GetCurFile(LPOLESTR * ppszFileName, AM_MEDIA_TYPE *pmt);
};


//-----------------------------------------------------------------------------
//
//	CAPEStream Class
//
//-----------------------------------------------------------------------------

class CAPEStream :	public CSourceStream,
					public CSourceSeeking
{
	friend class CAPESource;
private:

	CAPESource		*m_pSource;
	APE::IAPEDecompress	*m_pDecoder;

	CMediaType		m_mtOut;


	// seeking stuff

	double			m_dDuration;
	unsigned int	m_iBlockSize;
	unsigned int	m_iBlocksDecoded;
	unsigned int	m_iTotalBlocks;
	long			m_lSampleRate;

	BOOL			m_bDiscontinuity;
	CCritSec		m_csSeeking;
	CCritSec		m_csDecoding;


public:

    CAPEStream(HRESULT *phr, CAPESource *pParent, LPCWSTR pPinName);
	~CAPEStream();

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, VOID **ppv);

	// opening the file ...
	HRESULT OpenAPEFile(LPWSTR pFileName);
	void ReleaseAPEObjects();

	// deal with the stream
	HRESULT FillBuffer(IMediaSample *pSample);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT CheckMediaType(const CMediaType *pmt);
	HRESULT GetMediaType(CMediaType *pmt);

	// --- CSourceSeeking methods
	HRESULT ChangeStart();
	HRESULT ChangeStop();
	HRESULT ChangeRate();
	void UpdateFromSeek();

	HRESULT OnThreadStartPlay();
};








#endif /* MAIN_H */