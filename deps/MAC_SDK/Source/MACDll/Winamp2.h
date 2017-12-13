#pragma once

class CAPEWinampPlugin
{
public:

    static int Play(char * pFilename);
    static void Stop();
    static int GetFileLength();
    static int GetOutputTime();
    static void SetOutputTime(int nNewPositionMS);
    static int ShowFileInformationDialog(char * pFilename, HWND hwnd);
    static void GetFileInformation(char * pFilename, char * pTitle, int * pLengthMS);
    static void ShowConfigurationDialog(HWND hwndParent);
    static void ShowAboutDialog(HWND hwndParent);
    static void SetVolume(int volume);
    static void Pause();
    static void Unpause();
    static int IsPaused();
    static void SetPan(int pan);
    static void InitializePlugin();
    static void UninitializePlugin();
    static int IsOurFile(char * pFilename);

    static CSmartPtr<IAPEDecompress> m_spAPEDecompress;
    static TCHAR m_cCurrentFilename[MAX_PATH];                // the name of the currently playing file

protected:
    
    static DWORD WINAPI __stdcall DecodeThread(void *Kill_Switch);
    static BOOL CheckBufferForSilence(void * pBuffer, const unsigned __int32 nSamples);
    static long ScaleBuffer(IAPEDecompress *pAPEDecompress, unsigned char *pBuffer, long nBlocks);
    static void BuildDescriptionStringFromFilename(CString & strBuffer, const str_utfn * pFilename);
    static void BuildDescriptionString(CString & strBuffer, CAPETag * pAPETag, const str_utfn * pFilename);
    static CWinampSettingsDlg * GetSettings();

    static int m_nDecodePositionMS;                            // the current decoding position, in milliseconds
    static int m_nPaused;                                    // paused
    static int m_nSeekNeeded;                                // if != -1, it's the point (ms) to seek to
    static CSmartPtr<CAPELink> m_spAPELink;
    static int m_nKillDecodeThread;                            // the kill switch for the decode thread
    static HANDLE m_hDecodeThread;                            // the handle to the decode thread
    static long m_nScaledBitsPerSample;
    static long m_nScaledBytesPerSample;
    static long m_nScaledPeakLevel;
    static long m_nLengthMS;
};
