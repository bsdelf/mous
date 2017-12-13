#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include "filters.h" 
#include "resource.h"
#include "MACLib.h"
#include "CharacterHelper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//OpenFilterInput: Open the file for reading
////////////////////////////////////////////
__declspec(dllexport) int FAR PASCAL FilterGetFileSize(HANDLE hInput)
{    
    IAPEDecompress* pAPEDecompress = (IAPEDecompress*) hInput;
    if (hInput == NULL) return 0;
    
    int BytesPerSample = pAPEDecompress->GetInfo(APE_INFO_BYTES_PER_SAMPLE);
    if (BytesPerSample == 3) BytesPerSample = 4;
    return pAPEDecompress->GetInfo(APE_INFO_TOTAL_BLOCKS) * pAPEDecompress->GetInfo(APE_INFO_CHANNELS) * BytesPerSample;
}

__declspec(dllexport) HANDLE FAR PASCAL OpenFilterInput( LPSTR lpstrFilename, int far *lSamprate, 
                                            WORD far *wBitsPerSample, WORD far *wChannels, HWND hWnd, int far *lChunkSize)
{
    ///////////////////////////////////////////////////////////////////////////////
    // open the APE file
    ///////////////////////////////////////////////////////////////////////////////    
    CSmartPtr<wchar_t> spUTF16(CAPECharacterHelper::GetUTF16FromANSI(lpstrFilename), TRUE);
    IAPEDecompress * pAPEDecompress = CreateIAPEDecompress(spUTF16);
    if (pAPEDecompress == NULL)
        return NULL;
    
    *lSamprate= pAPEDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
    *wChannels=(WORD) pAPEDecompress->GetInfo(APE_INFO_CHANNELS);
    *wBitsPerSample=(WORD) pAPEDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);
    
    if (*wBitsPerSample == 24) *wBitsPerSample = 32;
    
    // this doesn't matter (but must be careful to avoid alignment problems)
    *lChunkSize = 16384 * (*wBitsPerSample / 8) * *wChannels;

    return (HANDLE) pAPEDecompress;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ReadFilterInput: Effective Reading
////////////////////////////////////
__declspec(dllexport) DWORD FAR PASCAL ReadFilterInput(HANDLE hInput, unsigned char far *buf, int lBytes)
{
    IAPEDecompress* pAPEDecompress = (IAPEDecompress*) hInput;
    if (hInput == NULL) return 0;

    intn nBlocksDecoded = 0;
    intn nBlocksToDecode = lBytes / pAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN);

    if (pAPEDecompress->GetInfo(APE_INFO_BYTES_PER_SAMPLE) == 3)
    {
        ///////////////////////////////////////////////////////////////////////////////
        // 24 bit decode (convert to 32 bit)
        //////////////////////
        
        nBlocksToDecode = (nBlocksToDecode * 3) / 4;
        if (pAPEDecompress->GetData((char*) buf, nBlocksToDecode, &nBlocksDecoded) != ERROR_SUCCESS)
            return 0;

        // expand to 32 bit
        unsigned char* p24Bit = (unsigned char*) &buf[(nBlocksDecoded * pAPEDecompress->GetInfo(APE_INFO_BLOCK_ALIGN)) - 3];
        float* p32Bit = (float*) &buf[(nBlocksDecoded * pAPEDecompress->GetInfo(APE_INFO_CHANNELS) * 4) - 4];

        while (p32Bit >= (float*) &buf[0])
        {
            float fValue = (float) (*p24Bit + *(p24Bit + 1) * 256 + *(p24Bit + 2) * 65536) / 256;
            if (fValue > 32768) fValue -= 65536;
            *p32Bit = fValue;

            p24Bit -= 3;
            p32Bit--;
        }
    }
    else
    {
        ///////////////////////////////////////////////////////////////////////////////
        // 8 and 16 bits decode
        //////////////////////
        if (pAPEDecompress->GetData((char*) buf, nBlocksToDecode, &nBlocksDecoded) != ERROR_SUCCESS)
            return 0;
    }

    int BytesPerSample = pAPEDecompress->GetInfo(APE_INFO_BYTES_PER_SAMPLE);
    if (BytesPerSample == 3) BytesPerSample = 4;
    return nBlocksDecoded * pAPEDecompress->GetInfo(APE_INFO_CHANNELS) * BytesPerSample;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CloseFileInput: Closes the file after reading
///////////////////////////////////////////////
__declspec(dllexport) void FAR PASCAL CloseFilterInput(HANDLE hInput)
{
    IAPEDecompress* pAPEDecompress = (IAPEDecompress*) hInput;
    if (pAPEDecompress != NULL) 
    {
        delete pAPEDecompress;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterOptions: Returns the compression level for this wave
////////////////////////////////////////////////////////////
__declspec(dllexport) DWORD FAR PASCAL FilterOptions(HANDLE hInput)
{ 
    int nCompressionLevel = 2;

    IAPEDecompress* pAPEDecompress = (IAPEDecompress*) hInput;
    if (pAPEDecompress != NULL) 
    {
        switch (pAPEDecompress->GetInfo(APE_INFO_COMPRESSION_LEVEL))
        {
        case COMPRESSION_LEVEL_FAST: nCompressionLevel = 1; break;
        case COMPRESSION_LEVEL_NORMAL: nCompressionLevel = 2; break;
        case COMPRESSION_LEVEL_HIGH: nCompressionLevel = 3; break;
        case COMPRESSION_LEVEL_EXTRA_HIGH: nCompressionLevel = 4; break;
        }
    }
    
    return nCompressionLevel;
}        

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterOptionsString: Fills compression level in "File Info"
/////////////////////////////////////////////////////////////
__declspec(dllexport) DWORD FAR PASCAL FilterOptionsString(HANDLE hInput, LPSTR szString)
{ 
    // default
    strcpy(szString, "Compression Level: Normal");

    // fill in from decoder
    IAPEDecompress* pAPEDecompress = (IAPEDecompress*) hInput;
    if (pAPEDecompress != NULL) 
    {
        char Title[256]; strcpy(Title, "Compression Level: ");
        
        switch (pAPEDecompress->GetInfo(APE_INFO_COMPRESSION_LEVEL))
        {
        case COMPRESSION_LEVEL_FAST: strcat(Title, "Fast"); break;
        case COMPRESSION_LEVEL_NORMAL: strcat(Title, "Normal"); break;
        case COMPRESSION_LEVEL_HIGH: strcat(Title, "High"); break;
        case COMPRESSION_LEVEL_EXTRA_HIGH: strcat(Title, "Extra High"); break;
        }

        Title[30] = 0x00;
        strcpy(szString, Title);
    }

    // return compression level
    return FilterOptions(hInput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterGetFirstSpecialData:
////////////////////////////
__declspec(dllexport) DWORD FAR PASCAL FilterGetFirstSpecialData(HANDLE hInput, SPECIALDATA * psp)
{    
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterGetNextSpecialData:
///////////////////////////
__declspec(dllexport) DWORD FAR PASCAL FilterGetNextSpecialData(HANDLE hInput, SPECIALDATA * psp)
{    
    return 0;
}

