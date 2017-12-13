#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include "filters.h" 
#include "resource.h"
#include "all.h"
#include "apeinfo.h"
#include "apecompress.h"
#include "CharacterHelper.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//QueryCoolFilter: Setup the filter
///////////////////////////////////
__declspec(dllexport) short FAR PASCAL QueryCoolFilter(COOLQUERY far * cq)
{   
    strcpy(cq->szName,"Monkey's Audio");        
    strcpy(cq->szCopyright,"Monkey's Audio file");
    strcpy(cq->szExt,"APE"); 
    strcpy(cq->szExt2,"MAC");
    
    cq->lChunkSize=1; 
    cq->dwFlags=QF_READSPECIALLAST|QF_WRITESPECIALFIRST|QF_RATEADJUSTABLE|
        QF_CANSAVE|QF_CANLOAD|QF_HASOPTIONSBOX|QF_CANDO32BITFLOATS;
     cq->Stereo8=0xFF;
     cq->Stereo16=0xFF;
     cq->Stereo24=0xFF;
     cq->Stereo32=0x00;
     cq->Mono8=0xFF;
     cq->Mono16=0xFF;
     cq->Mono24=0xFF;
     cq->Mono32=0x00;

     return C_VALIDLIBRARY;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterUnderstandsFormat: Check if the file is a real .ape
///////////////////////////////////////////////////////////
__declspec(dllexport) BOOL FAR PASCAL FilterUnderstandsFormat(LPSTR filename)
{    
    BOOL bValid = FALSE;

    CATCH_ERRORS
    (
        CSmartPtr<wchar_t> spUTF16(CAPECharacterHelper::GetUTF16FromANSI(filename), TRUE);
        IAPEDecompress * pAPEDecompress = CreateIAPEDecompress(spUTF16, NULL);
        if (pAPEDecompress != NULL)
        {
            bValid = TRUE;
            delete pAPEDecompress;
        }
    )

    return bValid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//FilterGetOptions: Show Dialog to select Compression Level
///////////////////////////////////////////////////////////
__declspec(dllexport) DWORD FAR PASCAL FilterGetOptions(HWND hWnd, HINSTANCE hInst, long lSamprate, WORD wChannels, WORD wBitsPerSample, DWORD dwOptions)
{
    long nDialogReturn=0L;

    FARPROC lpfnDIALOGMsgProc;
    lpfnDIALOGMsgProc = GetProcAddress(hInst,(LPCSTR)MAKELONG(20,0));            
            
    if (dwOptions==0) nDialogReturn=1;
    else nDialogReturn=dwOptions;
        
    nDialogReturn = (long)DialogBoxParam((HINSTANCE)hInst,(LPCTSTR) MAKEINTRESOURCE(IDD_COMPRESSION), (HWND)hWnd, (DLGPROC)lpfnDIALOGMsgProc,nDialogReturn);
    
    return nDialogReturn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DIALOGMsgProc: All Dialog stuff here
//////////////////////////////////////
__declspec(dllexport) BOOL FAR PASCAL DIALOGMsgProc(HWND hWndDlg, UINT Message, intn wParam, intn lParam)
{
    switch(Message)
    {
        ///////////////////////////////////////////////////////////////////////////////
        //Initialize Dialog
        ///////////////////
        case WM_INITDIALOG:
        {     
            long nDialogReturn;

        nDialogReturn=(long)lParam;
            if (nDialogReturn==1) CheckDlgButton(hWndDlg,IDC_R1,TRUE);
            else if (nDialogReturn==2) CheckDlgButton(hWndDlg,IDC_R2,TRUE);
            else if (nDialogReturn==3) CheckDlgButton(hWndDlg,IDC_R3,TRUE);
            else if (nDialogReturn==4) CheckDlgButton(hWndDlg,IDC_R4,TRUE);
        }         
        break;

        case WM_CLOSE:
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;

        ///////////////////////////////////////////////////////////////////////////////
        //All WM_COMMAND here
        /////////////////////
        case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
                case IDOK:
                {
                    long nDialogReturn;

                    if (IsDlgButtonChecked(hWndDlg,IDC_R1)) nDialogReturn=1;
                    else if (IsDlgButtonChecked(hWndDlg,IDC_R2)) nDialogReturn=2;
                    else if (IsDlgButtonChecked(hWndDlg,IDC_R3)) nDialogReturn=3;
                    else if (IsDlgButtonChecked(hWndDlg,IDC_R4)) nDialogReturn=4;

                    EndDialog(hWndDlg,(short)nDialogReturn);
                }
                break;

                case IDCANCEL:
                    EndDialog(hWndDlg, FALSE);
                    break;
            }
            break;
        }

        default:
            return FALSE;
    }

    return TRUE;
}



