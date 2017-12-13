#pragma once

namespace APE
{

/*************************************************************************************
Definitions
*************************************************************************************/
class CIO;

/*************************************************************************************
Read / Write from an IO source and return failure if the number of bytes specified
isn't read or written
*************************************************************************************/
int ReadSafe(CIO * pIO, void * pBuffer, int nBytes);
intn WriteSafe(CIO * pIO, void * pBuffer, intn nBytes);

/*************************************************************************************
Checks for the existence of a file
*************************************************************************************/
bool FileExists(wchar_t * pFilename);

/*************************************************************************************
Allocate aligned memory
*************************************************************************************/
void * AllocateAligned(intn nBytes, intn nAlignment);
void FreeAligned(void * pMemory);

/*************************************************************************************
Test for CPU features
*************************************************************************************/
bool GetMMXAvailable();
bool GetSSEAvailable();

/*************************************************************************************
String helpers
*************************************************************************************/
bool StringIsEqual(const str_utfn * pString1, const str_utfn * pString2, bool bCaseSensitive, int nCharacters = -1);

}
