/************************************************************************************
Includes
************************************************************************************/
#include "stdafx.h"
#include "MACDllApp.h"
#include "APETag.h"
#include "APELink.h"
#include "Winamp2.h"
#include "WinampSettingsDlg.h"
#include "in2.h"
#include "APEInfoDialog.h"
#include "APELink.h"
#include "CharacterHelper.h"
#define IO_USE_WIN_FILE_IO
#include "WinFileIO.h"

/************************************************************************************
Defines
************************************************************************************/
// post this to the main window at end of file (after playback has stopped)
#define WM_WA_MPEG_EOF    (WM_USER + 2)

// scaled bits
#define SCALED_BITS        16

// extended info structure
struct extendedFileInfoStruct
{ 
    char * pFilename;
    char * pMetaData; 
    char * pReturn;
    int nReturnBytes;
};

/************************************************************************************
The input module (publicly defined)
************************************************************************************/
In_Module g_APEWinampPluginModule = 
{
    IN_VER,                                                        // the version (defined in in2.h)
    PLUGIN_NAME,                                                // the name of the plugin (defined in all.h)
    0,                                                            // handle to the main window
    0,                                                            // handle to the dll instance
    "APE\0Monkey's Audio File (*.APE)\0"                        // the file type(s) supported
    "MAC\0Monkey's Audio File (*.MAC)\0"
    "APL\0Moneky's Audio File (*.APL)\0",                                        
    1,                                                            // seekable
    1,                                                            // uses output
    CAPEWinampPlugin::ShowConfigurationDialog,                    // all of the functions...
    CAPEWinampPlugin::ShowAboutDialog,
    CAPEWinampPlugin::InitializePlugin,
    CAPEWinampPlugin::UninitializePlugin,
    CAPEWinampPlugin::GetFileInformation,
    CAPEWinampPlugin::ShowFileInformationDialog,
    CAPEWinampPlugin::IsOurFile,
    CAPEWinampPlugin::Play,
    CAPEWinampPlugin::Pause,
    CAPEWinampPlugin::Unpause,
    CAPEWinampPlugin::IsPaused,
    CAPEWinampPlugin::Stop,
    CAPEWinampPlugin::GetFileLength,
    CAPEWinampPlugin::GetOutputTime,
    CAPEWinampPlugin::SetOutputTime,
    CAPEWinampPlugin::SetVolume,
    CAPEWinampPlugin::SetPan,
    0, 0, 0, 0, 0, 0, 0, 0, 0,                                    // vis stuff
    0, 0,                                                        // dsp stuff
    0,                                                            // Set_EQ function
    NULL,                                                        // setinfo
    0                                                            // out_mod
};

/************************************************************************************
Global variables -- shoot me now
************************************************************************************/
TCHAR CAPEWinampPlugin::m_cCurrentFilename[MAX_PATH] = { 0 };
int CAPEWinampPlugin::m_nDecodePositionMS = -1;
int CAPEWinampPlugin::m_nPaused = 0;
int CAPEWinampPlugin::m_nSeekNeeded = -1;
CSmartPtr<CAPELink> CAPEWinampPlugin::m_spAPELink;
int CAPEWinampPlugin::m_nKillDecodeThread = 0;
HANDLE CAPEWinampPlugin::m_hDecodeThread = INVALID_HANDLE_VALUE;
long CAPEWinampPlugin::m_nScaledBitsPerSample = 0;
long CAPEWinampPlugin::m_nScaledBytesPerSample = 0;
long CAPEWinampPlugin::m_nScaledPeakLevel = 0;
CSmartPtr<IAPEDecompress> CAPEWinampPlugin::m_spAPEDecompress;
long CAPEWinampPlugin::m_nLengthMS = 0;

/************************************************************************************
Plays a file (called once on the start of a file)
************************************************************************************/
int CAPEWinampPlugin::Play(char * pFilename) 
{
    // reset or initialize any public variables
    CSmartPtr<str_utfn> spFilename(CAPECharacterHelper::GetUTF16FromANSI(pFilename), TRUE);
    _tcscpy_s(m_cCurrentFilename, MAX_PATH, spFilename);

    m_nPaused = 0;
    m_nDecodePositionMS = 0;
    m_nSeekNeeded = -1;
    
    // open the file
    int nErrorCode = 0;
    m_spAPEDecompress.Assign(CreateIAPEDecompress(m_cCurrentFilename, &nErrorCode));
    if ((m_spAPEDecompress == NULL) || (nErrorCode != ERROR_SUCCESS))
        return -1;

    // quit if it's a zero length file
    if (m_spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS) == 0) 
    {
        m_spAPEDecompress.Delete();
        return -1;
    }

    // version check
    if (m_spAPEDecompress->GetInfo(APE_INFO_FILE_VERSION) > MAC_FILE_VERSION_NUMBER)
    {
        TCHAR cAPEFileVersion[32]; _stprintf_s(cAPEFileVersion, 32, _T("%.2f"), float(m_spAPEDecompress->GetInfo(APE_INFO_FILE_VERSION)) / float(1000));
        
        TCHAR cMessage[1024];
        _stprintf_s(cMessage, 1024, _T("You are attempting to play an APE file that was encoded with a version of Monkey's Audio which is newer than the installed APE plug-in.  There is a very high likelyhood that this will not work properly.  Please download and install the newest Monkey's Audio plug-in to remedy this problem.\r\n\r\nPlug-in version: %s\r\nAPE file version: %s"), MAC_VERSION_STRING, cAPEFileVersion);
        ::MessageBox(g_APEWinampPluginModule.hMainWindow, cMessage, _T("Update APE Plugin"), MB_OK | MB_ICONERROR);
    }

    // see if it's a stream
    g_APEWinampPluginModule.is_seekable = TRUE;
    
    // set the "scaled" bps
    if (GetSettings()->m_bScaleOutput == TRUE) 
    {
        m_nScaledBitsPerSample = SCALED_BITS;
        m_nScaledBytesPerSample = (SCALED_BITS / 8);
    }
    else 
    {
        m_nScaledBitsPerSample = m_spAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
        m_nScaledBytesPerSample = m_spAPEDecompress->GetInfo(APE_INFO_BYTES_PER_SAMPLE);
    }

    // adjust the peak level to account for bps scaling
    if ((GetSettings()->m_bScaleOutput == TRUE) && (m_spAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE) != SCALED_BITS)) 
    {
        if (m_spAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE) == 8) 
            m_nScaledPeakLevel = m_spAPEDecompress->GetInfo(APE_INFO_PEAK_LEVEL) << 8;
        else if (m_spAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE) == 24) 
            m_nScaledPeakLevel = m_spAPEDecompress->GetInfo(APE_INFO_PEAK_LEVEL) >> 8;
    }
    else
    {
        m_nScaledPeakLevel = m_spAPEDecompress->GetInfo(APE_INFO_PEAK_LEVEL);
    }

    // set the length
    double dBlocks = double(m_spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS));
    double dMilliseconds = (dBlocks * double(1000)) / double(m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE));
    m_nLengthMS = long(dMilliseconds);

    // open the output module
    int nMaxLatency = g_APEWinampPluginModule.outMod->Open(m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE), m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS), m_nScaledBitsPerSample, -1,-1);
    if (nMaxLatency < 0)
    {
        m_spAPEDecompress.Delete();
        return -1;
    }
    
    // initialize the visualization stuff
    g_APEWinampPluginModule.SAVSAInit(nMaxLatency, m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE));
    g_APEWinampPluginModule.VSASetInfo(m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE), m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS));

    // set the default volume
    g_APEWinampPluginModule.outMod->SetVolume(-666);

    // set the Winamp info (bitrate, channels, etc.)
    g_APEWinampPluginModule.SetInfo(m_spAPEDecompress->GetInfo(APE_DECOMPRESS_AVERAGE_BITRATE), m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE) / 1000, m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS), 1);
    
    // create the new thread
    m_nKillDecodeThread = 0;
    unsigned long nThreadID;
    m_hDecodeThread = (HANDLE) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DecodeThread, (void *) &m_nKillDecodeThread, 0, &nThreadID);

    // set the thread priority
    if (SetThreadPriority(m_hDecodeThread, GetSettings()->m_nThreadPriority) == 0) 
    {
        m_spAPEDecompress.Delete();
        return -1;
    }

    return 0; 
}

/************************************************************************************
Stops the file (called anytime a file is stopped, or restarted)
************************************************************************************/
void CAPEWinampPlugin::Stop() 
{ 
    // if the decode thread is active, kill it
    if (m_hDecodeThread != INVALID_HANDLE_VALUE)
    {
        // set the flag to kill the thread and then wait
        m_nKillDecodeThread = 1;
        if (WaitForSingleObject(m_hDecodeThread, INFINITE) == WAIT_TIMEOUT) 
        {
            MessageBox(g_APEWinampPluginModule.hMainWindow, _T("Error asking decode thread to die."), _T("Error Killing Decode Thread"), 0);
            TerminateThread(m_hDecodeThread, 0);
        }
        
        CloseHandle(m_hDecodeThread);
        m_hDecodeThread = INVALID_HANDLE_VALUE;
    }
    
    // cleanup global objects
    m_spAPEDecompress.Delete();
    m_spAPEDecompress.Delete();
    m_spAPELink.Delete();
    
    // close the output module
    g_APEWinampPluginModule.outMod->Close();

    // uninitialize the visualization stuff
    g_APEWinampPluginModule.SAVSADeInit();
}

/************************************************************************************
Check a buffer for silence
************************************************************************************/
BOOL CAPEWinampPlugin::CheckBufferForSilence(void * pBuffer, const unsigned __int32 nSamples) 
{
    unsigned __int32 nSum = 0;

    if (m_nScaledBitsPerSample == 8) 
    {
        unsigned __int8 * pData = (unsigned __int8 *) pBuffer;

        for (unsigned __int32 z = 0; z < nSamples; z++, pData++) 
            nSum += abs(*pData - 128);

        nSum <<= 8;
    }
    else if (m_nScaledBitsPerSample == 16) 
    {
        __int16 * pData = (__int16 *) pBuffer;
        for (unsigned __int32 z = 0; z < nSamples; z++, pData++) 
            nSum += abs(*pData);
    }

    nSum /= ape_max(nSamples, 1);
    
    if (nSum > 64)
        return FALSE;
    else
        return TRUE;
}

/************************************************************************************
Scale a buffer
************************************************************************************/
long CAPEWinampPlugin::ScaleBuffer(IAPEDecompress * pAPEDecompress, unsigned char * pBuffer, long nBlocks)
{
    if (pAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE) == 8) 
    {
        unsigned char *pBuffer8 = &pBuffer[nBlocks * pAPEDecompress->GetInfo(APE_INFO_CHANNELS) - 1];
        __int16 *pBuffer16 = (__int16 *) &pBuffer[nBlocks * pAPEDecompress->GetInfo(APE_INFO_CHANNELS) * 2 - 2];
        while (pBuffer8 >= pBuffer)
        {
            *pBuffer16-- = (__int16) ((long(*pBuffer8--) - 128) << 8);
        }
    }
    else if (pAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE) == 24) 
    {
        unsigned char * pBuffer24 = (unsigned char *) pBuffer;
        __int16 * pBuffer16 = (__int16 *) pBuffer;
        long nElements = nBlocks * pAPEDecompress->GetInfo(APE_INFO_CHANNELS);
        for (long z = 0; z < nElements; z++, pBuffer16++, pBuffer24 += 3)
        {
            *pBuffer16 = (__int16) (*((long *) pBuffer24) >> 8);
        }
    }

    return 0;
}

/************************************************************************************
The decode thread
************************************************************************************/
DWORD WINAPI __stdcall CAPEWinampPlugin::DecodeThread(void *bKillSwitch) 
{
    // variable declares
    BOOL bDone = FALSE;
    long nSilenceMS = 0;
    
    // the sample buffer...must be able to hold twice the original 1152 samples for DSP
    CSmartPtr<unsigned char> spSampleBuffer(new unsigned char [1152 * 2 * m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS) * m_nScaledBytesPerSample], TRUE);

    // start the decoding loop
    while (! *((int *) bKillSwitch))
    {
        // seek if necessary
        if (m_nSeekNeeded != -1) 
        {
            // update the decode position and reset the seek needed flag
            m_nDecodePositionMS = m_nSeekNeeded;
            m_nSeekNeeded = -1;

            // need to use doubles to avoid overflows (at around 10 minutes)
            double dLengthMS = (double(m_spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS)) * double(1000)) / double(m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE));
            double dSeekPercentage = double(m_nDecodePositionMS) / dLengthMS;
            double dSeekBlock = double(m_spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS)) * dSeekPercentage;
            
            // seek
            long nSeekBlock = (long) (dSeekBlock + 0.5);
            m_spAPEDecompress->Seek(nSeekBlock);
            
            // flush out the output module of data already in it
            g_APEWinampPluginModule.outMod->Flush(m_nDecodePositionMS);

            Sleep(20);
        }
        
        // quit if the 'bDone' flag is set
        if (bDone)
        {
            g_APEWinampPluginModule.outMod->CanWrite();
            if (!g_APEWinampPluginModule.outMod->IsPlaying()) 
            {
                PostMessage(g_APEWinampPluginModule.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
                return 0;
            }

            Sleep(10);
        }        
        // write data into the output stream if there is enough room for one full sample buffer
        else if (g_APEWinampPluginModule.outMod->CanWrite() >= (((576 * m_nScaledBytesPerSample) * m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS)) << (g_APEWinampPluginModule.dsp_isactive() ? 1 : 0))) 
        {
            // decompress the data
            intn nBlocksDecoded = 0;
            BOOL bSynched = TRUE;
            try
            {
                // get the data
                if (m_spAPEDecompress->GetData((char *) spSampleBuffer.GetPtr(), 576, &nBlocksDecoded) != ERROR_SUCCESS)
                    throw(1);
            }
            catch(...)
            {
                bSynched = FALSE;
                
                if (GetSettings()->m_bIgnoreBitstreamErrors == FALSE) 
                {
                    TCHAR cErrorTime[64];
                    int nSeconds = m_spAPEDecompress->GetInfo(APE_DECOMPRESS_CURRENT_MS) / 1000; int nMinutes = nSeconds / 60; nSeconds = nSeconds % 60; int nHours = nMinutes / 60; nMinutes = nMinutes % 60;
                    if (nHours > 0)    _stprintf_s(cErrorTime, 64, _T("%d:%02d:%02d"), nHours, nMinutes, nSeconds);
                    else if (nMinutes > 0) _stprintf_s(cErrorTime, 64, _T("%d:%02d"), nMinutes, nSeconds);
                    else _stprintf_s(cErrorTime, 64, _T("0:%02d"), nSeconds);

                    TCHAR cErrorMessage[1024];
					_stprintf_s(
                        cErrorMessage, 
						1024,
                        _T("Monkey's Audio encountered an error at %s while decompressing the file '%s'.\r\n\r\n")
                        _T("Please ensure that you are using the latest version of Monkey's Audio.  ")
                        _T("If this error persists using the latest version of Monkey's Audio, it is likely that the file has become corrupted.\r\n\r\n")
                        _T("Use the option 'Ignore Bitstream Errors' in the plug-in settings to not recieve this warning when Monkey's Audio encounters an error while decompressing."),
                        cErrorTime, m_cCurrentFilename
                    );

                    MessageBox(g_APEWinampPluginModule.hMainWindow, cErrorMessage, _T("Monkey's Audio Decompression Error"), MB_OK | MB_ICONERROR);

                    bDone = TRUE;
                    continue;
                }
            }
            
            // set the done flag if there was nothing decompressed
            if (nBlocksDecoded == 0)
            {
                bDone = TRUE;
                continue;
            }

            // do any MAC processing
            if (GetSettings()->m_bScaleOutput == TRUE)
            {
                ScaleBuffer(m_spAPEDecompress, spSampleBuffer, nBlocksDecoded);
            }

            long nBytesDecoded = nBlocksDecoded * m_nScaledBytesPerSample * m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS);

            // pass the samples through the dsp if it's running
            if (g_APEWinampPluginModule.dsp_isactive())
            {
                nBlocksDecoded = g_APEWinampPluginModule.dsp_dosamples((short *) spSampleBuffer.GetPtr(), nBlocksDecoded, m_nScaledBitsPerSample, m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS), m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE));
                nBytesDecoded = nBlocksDecoded * m_nScaledBytesPerSample * m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS);
            }                

            if (GetSettings()->m_bSuppressSilence == TRUE) 
            {
                if (CheckBufferForSilence(spSampleBuffer, nBytesDecoded / m_nScaledBytesPerSample) == FALSE)
                    nSilenceMS = 0;
                else
                    nSilenceMS += (nBlocksDecoded * 1000) / m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
            }
            else 
            {
                nSilenceMS = 0;
            }

            if (nSilenceMS < 1000) 
            {
                // add the data to the visualization
                g_APEWinampPluginModule.SAAddPCMData((char *) spSampleBuffer.GetPtr(), m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS), m_nScaledBitsPerSample, m_nDecodePositionMS);
                g_APEWinampPluginModule.VSAAddPCMData((char *) spSampleBuffer.GetPtr(), m_spAPEDecompress->GetInfo(APE_INFO_CHANNELS), m_nScaledBitsPerSample, m_nDecodePositionMS);

                // write the data to the output stream
                g_APEWinampPluginModule.outMod->Write((char *) spSampleBuffer.GetPtr(), nBytesDecoded);
            }
            else
            {
                bSynched = FALSE;
            }

            // update the VBR display
            g_APEWinampPluginModule.SetInfo(m_spAPEDecompress->GetInfo(APE_DECOMPRESS_CURRENT_BITRATE), -1, -1, bSynched);

            // increment the decode position
            m_nDecodePositionMS += ((nBlocksDecoded * 1000) / m_spAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE));
        }

        // if it wasn't done, and there wasn't room in the output stream, just wait and try again
        else 
        {
            Sleep(20);
        }

        Sleep(0);
    }

    return 0;
}

/************************************************************************************
Returns the length of the current file in ms
************************************************************************************/
int CAPEWinampPlugin::GetFileLength() 
{ 
    int nRetVal = m_nLengthMS; 

    return nRetVal;
}

/************************************************************************************
Returns the output time in ms
************************************************************************************/
int CAPEWinampPlugin::GetOutputTime() 
{
    int nRetVal = m_nDecodePositionMS + (g_APEWinampPluginModule.outMod->GetOutputTime() - g_APEWinampPluginModule.outMod->GetWrittenTime()); 
    return nRetVal;
}

/************************************************************************************
Sets the output time in ms
************************************************************************************/
void CAPEWinampPlugin::SetOutputTime(int nNewPositionMS) 
{ 
    m_nSeekNeeded = nNewPositionMS; 
}

/************************************************************************************
Show the the file info dialog
************************************************************************************/
int CAPEWinampPlugin::ShowFileInformationDialog(char * pFilename, HWND hwnd) 
{
    CSmartPtr<str_utfn> spFilename(CAPECharacterHelper::GetUTF16FromANSI(pFilename), TRUE);
    
    CAPEInfoDialog APEInfoDialog;
    APEInfoDialog.ShowAPEInfoDialog(spFilename, g_APEWinampPluginModule.hDllInstance, (LPCTSTR) IDD_APE_INFO, hwnd);

    return 0;
}

/************************************************************************************
File info helpers
************************************************************************************/
void CAPEWinampPlugin::BuildDescriptionStringFromFilename(CString & strBuffer, const str_utfn * pFilename)
{
    const str_utfn * p = pFilename + _tcslen(pFilename);
    while (*p != '\\' && p >= pFilename)
        p--;
    
    strBuffer = ++p;
    
    if (strBuffer.GetLength() >= 4)
        strBuffer = strBuffer.Left(strBuffer.GetLength() - 4);
}

void CAPEWinampPlugin::BuildDescriptionString(CString & strBuffer, CAPETag * pAPETag, const str_utfn * pFilename)
{
    if (pAPETag == NULL)
    {
        BuildDescriptionStringFromFilename(strBuffer, pFilename);
        return;
    }

    if (pAPETag->GetHasID3Tag() == FALSE && pAPETag->GetHasAPETag() == FALSE)
    {
        BuildDescriptionStringFromFilename(strBuffer, pFilename);
        return;
    }

    TCHAR cBuffer[256];
    int nBufferBytes;

    #define REPLACE_TOKEN_WITH_TAG_FIELD(TAG_FIELD, TOKEN) \
        nBufferBytes = 256; \
        pAPETag->GetFieldString(TAG_FIELD, cBuffer, &nBufferBytes); \
        strBuffer.Replace(TOKEN, cBuffer);
    
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_ARTIST, _T("%1"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_TITLE, _T("%2"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_ALBUM, _T("%3"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_YEAR, _T("%4"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_COMMENT, _T("%5"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_GENRE, _T("%6"))
    REPLACE_TOKEN_WITH_TAG_FIELD(APE_TAG_FIELD_TRACK, _T("%7"))
    
    TCHAR * p = m_cCurrentFilename + _tcslen(m_cCurrentFilename);
    while (*p != '\\' && p >= m_cCurrentFilename)
        p--;
    strBuffer.Replace(_T("%8"), ++p);
    
    strBuffer.Replace(_T("%9"), m_cCurrentFilename);
}

/************************************************************************************
Get the file info
************************************************************************************/
void CAPEWinampPlugin::GetFileInformation(char * pFilename, char * pTitle, int * pLengthMS) 
{
    CSmartPtr<IAPEDecompress> spAPEDecompress;
    CString strFilename;
    if (!pFilename || !*pFilename)
    {
        // currently playing file
        spAPEDecompress.Assign(m_spAPEDecompress, FALSE, FALSE);
        strFilename = m_cCurrentFilename;
    }
    else
    {
        // different file
        CSmartPtr<wchar_t> spUTF16(CAPECharacterHelper::GetUTF16FromANSI(pFilename), TRUE);
        spAPEDecompress.Assign(CreateIAPEDecompress(spUTF16));
        strFilename = spUTF16;
    }

    if (spAPEDecompress != NULL)
    {
        if (pLengthMS)
        {
            *pLengthMS = spAPEDecompress->GetInfo(APE_DECOMPRESS_LENGTH_MS);
        }
        
        if (pTitle) 
        {
            CString strDisplay = GetSettings()->m_strFileDisplayMethod;
            BuildDescriptionString(strDisplay, GET_TAG(spAPEDecompress), strFilename);

            CSmartPtr<char> spDisplayANSI(CAPECharacterHelper::GetANSIFromUTF16(strDisplay), TRUE);
            strcpy(pTitle, spDisplayANSI);
        }                    
    }
}

/************************************************************************************
Displays the configuration dialog
************************************************************************************/
void CAPEWinampPlugin::ShowConfigurationDialog(HWND hwndParent) 
{
    GetSettings()->Show(hwndParent);
}

/************************************************************************************
Show the about dialog
************************************************************************************/
void CAPEWinampPlugin::ShowAboutDialog(HWND hwndParent) 
{
    MessageBox(hwndParent, PLUGIN_ABOUT, _T("Monkey's Audio Player"), MB_OK);
}

/************************************************************************************
Set the volume
************************************************************************************/
void CAPEWinampPlugin::SetVolume(int volume) 
{ 
    g_APEWinampPluginModule.outMod->SetVolume(volume); 
}

/************************************************************************************
Pause
************************************************************************************/
void CAPEWinampPlugin::Pause() 
{ 
    m_nPaused = 1;
    g_APEWinampPluginModule.outMod->Pause(1); 
}

/************************************************************************************
Unpause
************************************************************************************/
void CAPEWinampPlugin::Unpause() 
{ 
    m_nPaused = 0;
    g_APEWinampPluginModule.outMod->Pause(0); 
}

/************************************************************************************
Checks to see if it is currently m_nPaused
************************************************************************************/
int CAPEWinampPlugin::IsPaused() 
{ 
    return m_nPaused; 
}

/************************************************************************************
Set the pan
************************************************************************************/
void CAPEWinampPlugin::SetPan(int pan) 
{ 
    g_APEWinampPluginModule.outMod->SetPan(pan); 
}

/************************************************************************************
Initialize the plugin (called once on the close of Winamp)
************************************************************************************/
void CAPEWinampPlugin::InitializePlugin() 
{
}

/************************************************************************************
Uninitialize the plugin (called once on the close of Winamp)
************************************************************************************/
void CAPEWinampPlugin::UninitializePlugin() 
{ 
}

/************************************************************************************
Is our file (used for detecting URL streams)
************************************************************************************/
int CAPEWinampPlugin::IsOurFile(char * pFilename) { return 0; } 

/************************************************************************************
Get the settings
************************************************************************************/
CWinampSettingsDlg * CAPEWinampPlugin::GetSettings()
{
    return g_Application.GetWinampSettingsDlg();
}

/************************************************************************************
Exported functions
************************************************************************************/
extern "C" 
{
    __declspec( dllexport ) In_Module * winampGetInModule2()
    {
        return &g_APEWinampPluginModule;
    }

    __declspec( dllexport ) int winampGetExtendedFileInfo(extendedFileInfoStruct Info)
    {
        // on startup, type is queried for
        if (((Info.pFilename == NULL) || (Info.pFilename[0] == 0)) && 
            (strcmp(Info.pMetaData, "type") == 0))
        {
            strcpy(Info.pReturn, "ape");
            return 1;
        }

        // load the file 
        // use ugly statics since Winamp is built around global variables
        static CString strFilename;
        static CSmartPtr<IAPEDecompress> spAPEDecompress;

        CSmartPtr<wchar_t> spUTF16(CAPECharacterHelper::GetUTF16FromANSI(Info.pFilename), TRUE);
        if (spUTF16 != strFilename)
        {
            spAPEDecompress.Assign(CreateIAPEDecompress(spUTF16));
            strFilename = spUTF16;
        }

        // get the tag value
        CAPETag * pTag = spAPEDecompress ? (CAPETag *) spAPEDecompress->GetInfo(APE_INFO_TAG) : NULL;
        if (pTag != NULL)
        {
            if (strcmp(Info.pMetaData, "artist") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_ARTIST, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "album") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_ALBUM, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "title") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_TITLE, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "type") == 0)
            {
                strcpy(Info.pReturn, "ape");
            }
            else if (strcmp(Info.pMetaData, "comment") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_COMMENT, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "year") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_YEAR, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "genre") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_GENRE, Info.pReturn, &Info.nReturnBytes, FALSE);
            }
            else if (strcmp(Info.pMetaData, "length") == 0)
            {
                int nLength = spAPEDecompress->GetInfo(APE_DECOMPRESS_LENGTH_MS);
                sprintf(Info.pReturn, "%d", nLength);
            }
            else if (strcmp(Info.pMetaData, "track") == 0)
            {
                pTag->GetFieldString(APE_TAG_FIELD_TRACK, Info.pReturn, &Info.nReturnBytes, FALSE);

                // since track is queried last, unload 
                // this sucks, but Winamp doesn't provide a way around it
                strFilename.Empty();
                spAPEDecompress.Delete();
            }
            else
            {
                Info.pReturn[0] = 0;
            }

            return 1;
        }

        // no tag or file, so fail
        return 0;
    }

}