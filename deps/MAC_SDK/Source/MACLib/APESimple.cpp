#include "All.h"
#include "APEInfo.h"
#include "APECompress.h"
#include "APEDecompress.h"
#include "WAVInputSource.h"
#include IO_HEADER_FILE
#include "MACProgressHelper.h"
#include "GlobalFunctions.h"
#include "MD5.h"
#include "CharacterHelper.h"
using namespace APE;

#define UNMAC_DECODER_OUTPUT_NONE       0
#define UNMAC_DECODER_OUTPUT_WAV        1
#define UNMAC_DECODER_OUTPUT_APE        2

#define BLOCKS_PER_DECODE               9216

int DecompressCore(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nOutputMode, int nCompressionLevel, IAPEProgressCallback * pProgressCallback);

/*****************************************************************************************
Simple progress callback (for legacy support)
*****************************************************************************************/
class CAPEProgressCallbackLegacy : public IAPEProgressCallback
{
public:
    CAPEProgressCallbackLegacy(int * pProgress, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
    {
        m_pProgress = pProgress;
        m_ProgressCallback = ProgressCallback;
        m_pKillFlag = pKillFlag;
    }

    virtual void Progress(int nPercentageDone)
    {
        if (m_pProgress != NULL)
            *m_pProgress = nPercentageDone;

        if (m_ProgressCallback != NULL)
            m_ProgressCallback(nPercentageDone);
    }

    virtual int GetKillFlag()
    {
        return (m_pKillFlag == NULL) ? KILL_FLAG_CONTINUE : *m_pKillFlag;
    }

private:
    int * m_pProgress;
    APE_PROGRESS_CALLBACK m_ProgressCallback;
    int * m_pKillFlag;
};

/*****************************************************************************************
ANSI wrappers
*****************************************************************************************/
int __stdcall CompressFile(const APE::str_ansi * pInputFilename, const APE::str_ansi * pOutputFilename, int nCompressionLevel, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CSmartPtr<str_utfn> spInputFile(CAPECharacterHelper::GetUTF16FromANSI(pInputFilename), true);
    CSmartPtr<str_utfn> spOutputFile(CAPECharacterHelper::GetUTF16FromANSI(pOutputFilename), true);
    return CompressFileW(spInputFile, spOutputFile, nCompressionLevel, pPercentageDone, ProgressCallback, pKillFlag);
}

int __stdcall DecompressFile(const APE::str_ansi * pInputFilename, const APE::str_ansi * pOutputFilename, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CSmartPtr<str_utfn> spInputFile(CAPECharacterHelper::GetUTF16FromANSI(pInputFilename), true);
    CSmartPtr<str_utfn> spOutputFile(CAPECharacterHelper::GetUTF16FromANSI(pOutputFilename), true);
    return DecompressFileW(spInputFile, pOutputFilename ? spOutputFile : NULL, pPercentageDone, ProgressCallback, pKillFlag);
}

int __stdcall ConvertFile(const APE::str_ansi * pInputFilename, const APE::str_ansi * pOutputFilename, int nCompressionLevel, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CSmartPtr<str_utfn> spInputFile(CAPECharacterHelper::GetUTF16FromANSI(pInputFilename), true);
    CSmartPtr<str_utfn> spOutputFile(CAPECharacterHelper::GetUTF16FromANSI(pOutputFilename), true);
    return ConvertFileW(spInputFile, spOutputFile, nCompressionLevel, pPercentageDone, ProgressCallback, pKillFlag);
}

int __stdcall VerifyFile(const APE::str_ansi * pInputFilename, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag, bool bQuickVerifyIfPossible)
{
    CSmartPtr<str_utfn> spInputFile(CAPECharacterHelper::GetUTF16FromANSI(pInputFilename), true);
    return VerifyFileW(spInputFile, pPercentageDone, ProgressCallback, pKillFlag, false);
}

/*****************************************************************************************
Legacy callback wrappers
*****************************************************************************************/
int __stdcall CompressFileW(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nCompressionLevel, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CAPEProgressCallbackLegacy ProgressCallbackLegacy(pPercentageDone, ProgressCallback, pKillFlag);
    return CompressFileW2(pInputFilename, pOutputFilename, nCompressionLevel, &ProgressCallbackLegacy);
}

int __stdcall VerifyFileW(const APE::str_utfn * pInputFilename, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag, bool bQuickVerifyIfPossible)
{
    CAPEProgressCallbackLegacy ProgressCallbackLegacy(pPercentageDone, ProgressCallback, pKillFlag);
    return VerifyFileW2(pInputFilename, &ProgressCallbackLegacy, bQuickVerifyIfPossible);
}

int __stdcall DecompressFileW(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CAPEProgressCallbackLegacy ProgressCallbackLegacy(pPercentageDone, ProgressCallback, pKillFlag);
    return DecompressFileW2(pInputFilename, pOutputFilename, &ProgressCallbackLegacy);
}

int __stdcall ConvertFileW(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nCompressionLevel, int * pPercentageDone, APE_PROGRESS_CALLBACK ProgressCallback, int * pKillFlag)
{
    CAPEProgressCallbackLegacy ProgressCallbackLegacy(pPercentageDone, ProgressCallback, pKillFlag);
    return ConvertFileW2(pInputFilename, pOutputFilename, nCompressionLevel, &ProgressCallbackLegacy);
}

/*****************************************************************************************
Compress file
*****************************************************************************************/
int __stdcall CompressFileW2(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nCompressionLevel, IAPEProgressCallback * pProgressCallback)
{
    // declare the variables
    int nFunctionRetVal = ERROR_SUCCESS;
    APE::WAVEFORMATEX WaveFormatEx;
    CSmartPtr<CMACProgressHelper> spMACProgressHelper;
    CSmartPtr<unsigned char> spBuffer;
    CSmartPtr<IAPECompress> spAPECompress;
    
    try
    {
        // create the input source
        int nResult = ERROR_UNDEFINED;
        int nAudioBlocks = 0; int nHeaderBytes = 0; int nTerminatingBytes = 0;
        CSmartPtr<CInputSource> spInputSource(CreateInputSource(pInputFilename, &WaveFormatEx, &nAudioBlocks,
            &nHeaderBytes, &nTerminatingBytes, &nResult));

        if ((spInputSource == NULL) || (nResult != ERROR_SUCCESS))
            throw nResult;

        // create the compressor
        spAPECompress.Assign(CreateIAPECompress());
        if (spAPECompress == NULL) throw ERROR_UNDEFINED;
        
        // figure the audio bytes
        unsigned int nAudioBytes = nAudioBlocks * WaveFormatEx.nBlockAlign;

        // start the encoder
        if (nHeaderBytes > 0) spBuffer.Assign(new unsigned char [nHeaderBytes], true);
        THROW_ON_ERROR(spInputSource->GetHeaderData(spBuffer.GetPtr()))
        THROW_ON_ERROR(spAPECompress->Start(pOutputFilename, &WaveFormatEx, nAudioBytes,
            nCompressionLevel, spBuffer.GetPtr(), nHeaderBytes));
    
        spBuffer.Delete();

        // set-up the progress
        spMACProgressHelper.Assign(new CMACProgressHelper(nAudioBytes, pProgressCallback));

        // master loop
        unsigned int nBytesLeft = nAudioBytes;

        while (nBytesLeft > 0)
        {
            int nBytesAdded = 0;
            THROW_ON_ERROR(spAPECompress->AddDataFromInputSource(spInputSource.GetPtr(), nBytesLeft, &nBytesAdded))

            nBytesLeft -= nBytesAdded;

            // update the progress
            spMACProgressHelper->UpdateProgress(nAudioBytes - nBytesLeft);

            // process the kill flag
            if (spMACProgressHelper->ProcessKillFlag(true) != ERROR_SUCCESS)
                throw(ERROR_USER_STOPPED_PROCESSING);
        }

        // finalize the file
        if (nTerminatingBytes > 0) spBuffer.Assign(new unsigned char [nTerminatingBytes], true);
        THROW_ON_ERROR(spInputSource->GetTerminatingData(spBuffer.GetPtr()));
        THROW_ON_ERROR(spAPECompress->Finish(spBuffer.GetPtr(), nTerminatingBytes, nTerminatingBytes))

        // update the progress to 100%
        spMACProgressHelper->UpdateProgressComplete();
    }
    catch(int nErrorCode)
    {
        nFunctionRetVal = (nErrorCode == 0) ? ERROR_UNDEFINED : nErrorCode;
    }
    catch(...)
    {
        nFunctionRetVal = ERROR_UNDEFINED;
    }
    
    // kill the compressor if we failed
    if ((nFunctionRetVal != 0) && (spAPECompress != NULL))
        spAPECompress->Kill();
    
    // return
    return nFunctionRetVal;
}

/*****************************************************************************************
Verify file
*****************************************************************************************/
int __stdcall VerifyFileW2(const APE::str_utfn * pInputFilename, IAPEProgressCallback * pProgressCallback, bool bQuickVerifyIfPossible)
{
    // error check the function parameters
    if (pInputFilename == NULL)
    {
        return ERROR_INVALID_FUNCTION_PARAMETER;
    }

    // return value
    int nResult = ERROR_UNDEFINED;
    
    // see if we can quick verify
    if (bQuickVerifyIfPossible)
    {
        CSmartPtr<IAPEDecompress> spAPEDecompress;
        try
        {
            int nFunctionRetVal = ERROR_SUCCESS;
            
            spAPEDecompress.Assign(CreateIAPEDecompress(pInputFilename, &nFunctionRetVal));
            if (spAPEDecompress == NULL || nFunctionRetVal != ERROR_SUCCESS) throw(nFunctionRetVal);

            APE_FILE_INFO * pInfo = (APE_FILE_INFO *) spAPEDecompress->GetInfo(APE_INTERNAL_INFO);
            
            // check version
            if ((pInfo->nVersion < 3980) || (pInfo->spAPEDescriptor == NULL))
                throw(ERROR_UPSUPPORTED_FILE_VERSION);

            // make sure the MD5 is valid
            if (pInfo->nMD5Invalid)
                throw(ERROR_UPSUPPORTED_FILE_VERSION);
        }
        catch(...)
        {
            bQuickVerifyIfPossible = false;
        }
    }

    // if we can and should quick verify, then do it
    if (bQuickVerifyIfPossible)
    {
        // variable declares
        int nFunctionRetVal = ERROR_SUCCESS;
        unsigned int nBytesRead = 0;
        CSmartPtr<IAPEDecompress> spAPEDecompress;

        // run the quick verify
        try
        {
            spAPEDecompress.Assign(CreateIAPEDecompress(pInputFilename, &nFunctionRetVal));
            if (spAPEDecompress == NULL || nFunctionRetVal != ERROR_SUCCESS) throw(nFunctionRetVal);

            CMD5Helper MD5Helper;
            
            CIO * pIO = GET_IO(spAPEDecompress);
            APE_FILE_INFO * pInfo = (APE_FILE_INFO *) spAPEDecompress->GetInfo(APE_INTERNAL_INFO);

            if ((pInfo->nVersion < 3980) || (pInfo->spAPEDescriptor == NULL))
                throw(ERROR_UPSUPPORTED_FILE_VERSION);

            int nHead = pInfo->nJunkHeaderBytes + pInfo->spAPEDescriptor->nDescriptorBytes;
            int nStart = nHead + pInfo->spAPEDescriptor->nHeaderBytes + pInfo->spAPEDescriptor->nSeekTableBytes;

            pIO->Seek(nHead, FILE_BEGIN);
            int nHeadBytes = nStart - nHead;
            CSmartPtr<unsigned char> spHeadBuffer(new unsigned char [nHeadBytes], true);
            if ((pIO->Read(spHeadBuffer, nHeadBytes, &nBytesRead) != ERROR_SUCCESS) || (nHeadBytes != int(nBytesRead)))
                throw(ERROR_IO_READ);
            
            int nBytesLeft = pInfo->spAPEDescriptor->nHeaderDataBytes + pInfo->spAPEDescriptor->nAPEFrameDataBytes + pInfo->spAPEDescriptor->nTerminatingDataBytes;
            CSmartPtr<unsigned char> spBuffer(new unsigned char [16384], true);
            nBytesRead = 1;
            while ((nBytesLeft > 0) && (nBytesRead > 0))
            {
                int nBytesToRead = ape_min(16384, nBytesLeft);
                if (pIO->Read(spBuffer, nBytesToRead, &nBytesRead) != ERROR_SUCCESS)
                    throw(ERROR_IO_READ);

                MD5Helper.AddData(spBuffer, nBytesRead);
                nBytesLeft -= nBytesRead;
            }

            if (nBytesLeft != 0)
                throw(ERROR_IO_READ);

            MD5Helper.AddData(spHeadBuffer, nHeadBytes);

            unsigned char cResult[16];
            MD5Helper.GetResult(cResult);

            if (memcmp(cResult, pInfo->spAPEDescriptor->cFileMD5, 16) != 0)
                nFunctionRetVal = ERROR_INVALID_CHECKSUM;

        }
        catch(int nErrorCode)
        {
            nFunctionRetVal = (nErrorCode == 0) ? ERROR_UNDEFINED : nErrorCode;
        }
        catch(...)
        {
            nFunctionRetVal = ERROR_UNDEFINED;
        }
        
        // return value
        nResult = nFunctionRetVal;
    }
    else
    {
        nResult = DecompressCore(pInputFilename, NULL, UNMAC_DECODER_OUTPUT_NONE, -1, pProgressCallback);
    }

    return nResult;
}

/*****************************************************************************************
Decompress file
*****************************************************************************************/
int __stdcall DecompressFileW2(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, IAPEProgressCallback * pProgressCallback)
{
    if (pOutputFilename == NULL)
        return VerifyFileW2(pInputFilename, pProgressCallback);
    else
        return DecompressCore(pInputFilename, pOutputFilename, UNMAC_DECODER_OUTPUT_WAV, -1, pProgressCallback);
}

/*****************************************************************************************
Convert file
*****************************************************************************************/
int __stdcall ConvertFileW2(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nCompressionLevel, IAPEProgressCallback * pProgressCallback) 
{
    return DecompressCore(pInputFilename, pOutputFilename, UNMAC_DECODER_OUTPUT_APE, nCompressionLevel, pProgressCallback);
}

/*****************************************************************************************
Decompress a file using the specified output method
*****************************************************************************************/
int DecompressCore(const APE::str_utfn * pInputFilename, const APE::str_utfn * pOutputFilename, int nOutputMode, int nCompressionLevel, IAPEProgressCallback * pProgressCallback) 
{
    // error check the function parameters
    if (pInputFilename == NULL) 
    {
        return ERROR_INVALID_FUNCTION_PARAMETER;
    }

    // variable declares
    int nFunctionRetVal = ERROR_SUCCESS;
    CSmartPtr<IO_CLASS_NAME> spioOutput;
    CSmartPtr<IAPECompress> spAPECompress;
    CSmartPtr<IAPEDecompress> spAPEDecompress;
    CSmartPtr<unsigned char> spTempBuffer;
    CSmartPtr<CMACProgressHelper> spMACProgressHelper;
    APE::WAVEFORMATEX wfeInput;

    try
    {
        // create the decoder
        spAPEDecompress.Assign(CreateIAPEDecompress(pInputFilename, &nFunctionRetVal));
        if (spAPEDecompress == NULL || nFunctionRetVal != ERROR_SUCCESS) throw(nFunctionRetVal);

        // get the input format
        THROW_ON_ERROR(spAPEDecompress->GetInfo(APE_INFO_WAVEFORMATEX, (intn) &wfeInput))

        // allocate space for the header
        spTempBuffer.Assign(new unsigned char [spAPEDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES)], true);
        if (spTempBuffer == NULL) throw(ERROR_INSUFFICIENT_MEMORY);

        // get the header
        THROW_ON_ERROR(spAPEDecompress->GetInfo(APE_INFO_WAV_HEADER_DATA, (intn) spTempBuffer.GetPtr(), spAPEDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES)));

        // initialize the output
        if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
        {
            // create the file
            spioOutput.Assign(new IO_CLASS_NAME); THROW_ON_ERROR(spioOutput->Create(pOutputFilename))
        
            // output the header
            THROW_ON_ERROR(WriteSafe(spioOutput, spTempBuffer, spAPEDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES)));
        }
        else if (nOutputMode == UNMAC_DECODER_OUTPUT_APE)
        {
            // quit if there is nothing to do
            if (spAPEDecompress->GetInfo(APE_INFO_FILE_VERSION) == MAC_FILE_VERSION_NUMBER && spAPEDecompress->GetInfo(APE_INFO_COMPRESSION_LEVEL) == nCompressionLevel)
                throw(ERROR_SKIPPED);

            // create and start the compressor
            spAPECompress.Assign(CreateIAPECompress());
            THROW_ON_ERROR(spAPECompress->Start(pOutputFilename, &wfeInput, (unsigned int) (spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS) * spAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN)),
                nCompressionLevel, spTempBuffer, spAPEDecompress->GetInfo(APE_INFO_WAV_HEADER_BYTES)))
        }

        // allocate space for decompression
        spTempBuffer.Assign(new unsigned char [spAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN) * BLOCKS_PER_DECODE], true);
        if (spTempBuffer == NULL) throw(ERROR_INSUFFICIENT_MEMORY);

        intn nBlocksLeft = spAPEDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS);
        
        // create the progress helper
        spMACProgressHelper.Assign(new CMACProgressHelper((unsigned int) (nBlocksLeft / BLOCKS_PER_DECODE), pProgressCallback));

        // main decoding loop
        while (nBlocksLeft > 0)
        {
            // decode data
            intn nBlocksDecoded = -1;
            int nResult = spAPEDecompress->GetData((char *) spTempBuffer.GetPtr(), BLOCKS_PER_DECODE, &nBlocksDecoded);
            if (nResult != ERROR_SUCCESS) 
                throw(ERROR_INVALID_CHECKSUM);

            // handle the output
            if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
            {
                unsigned int nBytesToWrite = (unsigned int) (nBlocksDecoded * spAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN));
                unsigned int nBytesWritten = 0;
                int nResult = spioOutput->Write(spTempBuffer, nBytesToWrite, &nBytesWritten);
                if ((nResult != 0) || (nBytesToWrite != nBytesWritten)) 
                    throw(ERROR_IO_WRITE);
            }
            else if (nOutputMode == UNMAC_DECODER_OUTPUT_APE)
            {
                THROW_ON_ERROR(spAPECompress->AddData(spTempBuffer, (nBlocksDecoded * spAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN))))
            }

            // update amount remaining
            nBlocksLeft -= nBlocksDecoded;
        
            // update progress and kill flag
            spMACProgressHelper->UpdateProgress();
            if (spMACProgressHelper->ProcessKillFlag(true) != 0)
                throw(ERROR_USER_STOPPED_PROCESSING);
        }

        // terminate the output
        if (nOutputMode == UNMAC_DECODER_OUTPUT_WAV)
        {
            // write any terminating WAV data
            if (spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES) > 0) 
            {
                spTempBuffer.Assign(new unsigned char[spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES)], true);
                if (spTempBuffer == NULL) throw(ERROR_INSUFFICIENT_MEMORY);
                THROW_ON_ERROR(spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (intn) spTempBuffer.GetPtr(), spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES)))
        
                unsigned int nBytesToWrite = (unsigned int) spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES);
                unsigned int nBytesWritten = 0;
                int nResult = spioOutput->Write(spTempBuffer, nBytesToWrite, &nBytesWritten);
                if ((nResult != 0) || (nBytesToWrite != nBytesWritten)) 
                    throw(ERROR_IO_WRITE);
            }
        }
        else if (nOutputMode == UNMAC_DECODER_OUTPUT_APE)
        {
            // write the WAV data and any tag
            int nTagBytes = GET_TAG(spAPEDecompress)->GetTagBytes();
            bool bHasTag = (nTagBytes > 0);
            int nTerminatingBytes = nTagBytes;
            nTerminatingBytes += (int) spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES);

            if (nTerminatingBytes > 0) 
            {
                spTempBuffer.Assign(new unsigned char[nTerminatingBytes], true);
                if (spTempBuffer == NULL) throw(ERROR_INSUFFICIENT_MEMORY);
                
                THROW_ON_ERROR(spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_DATA, (intn) spTempBuffer.GetPtr(), nTerminatingBytes))

                if (bHasTag)
                {
                    unsigned int nBytesRead = 0;
                    THROW_ON_ERROR(GET_IO(spAPEDecompress)->Seek(-(nTagBytes), FILE_END))
                    THROW_ON_ERROR(GET_IO(spAPEDecompress)->Read(&spTempBuffer[spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES)], nTagBytes, &nBytesRead))
                }

                THROW_ON_ERROR(spAPECompress->Finish(spTempBuffer, nTerminatingBytes, (int) spAPEDecompress->GetInfo(APE_INFO_WAV_TERMINATING_BYTES)));
            }
            else 
            {
                THROW_ON_ERROR(spAPECompress->Finish(NULL, 0, 0));
            }
        }

        // fire the "complete" progress notification
        spMACProgressHelper->UpdateProgressComplete();
    }
    catch(int nErrorCode)
    {
        nFunctionRetVal = (nErrorCode == 0) ? ERROR_UNDEFINED : nErrorCode;
    }
    catch(...)
    {
        nFunctionRetVal = ERROR_UNDEFINED;
    }
    
    // return
    return nFunctionRetVal;
}
