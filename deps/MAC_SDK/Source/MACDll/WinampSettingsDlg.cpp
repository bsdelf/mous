#include "stdafx.h"
#include "MACDll.h"
#include "WinampSettingsDlg.h"
#include ".\winampsettingsdlg.h"

IMPLEMENT_DYNAMIC(CWinampSettingsDlg, CDialog)

CWinampSettingsDlg::CWinampSettingsDlg(CWnd * pParent)
    : CDialog(CWinampSettingsDlg::IDD, pParent)
{
    m_hwndParent = NULL;

    GetModuleFileName(AfxGetInstanceHandle(), m_strSettingsFilename.GetBuffer(MAX_PATH), MAX_PATH);
    m_strSettingsFilename.ReleaseBuffer();
    m_strSettingsFilename = m_strSettingsFilename.Left(m_strSettingsFilename.GetLength() - 3) + _T("ini");

    LoadSettings();
}

CWinampSettingsDlg::~CWinampSettingsDlg()
{
}

void CWinampSettingsDlg::DoDataExchange(CDataExchange * pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IGNORE_BITSTREAM_ERRORS_CHECK, m_bIgnoreBitstreamErrors);
    DDX_Check(pDX, SUPPRESS_SILENCE_CHECK, m_bSuppressSilence);
    DDX_Check(pDX, SCALE_OUTPUT_CHECK, m_bScaleOutput);
    DDX_Text(pDX, FILE_DISPLAY_METHOD_EDIT, m_strFileDisplayMethod);
    DDX_Control(pDX, THREAD_PRIORITY_SLIDER, m_ctrlThreadPrioritySlider);
}

BEGIN_MESSAGE_MAP(CWinampSettingsDlg, CDialog)
END_MESSAGE_MAP()

BOOL CWinampSettingsDlg::LoadSettings()
{
    GetPrivateProfileString(_T("Plugin Settings"), _T("File Display Method"), _T("%1 - %2"), m_strFileDisplayMethod.GetBuffer(1024), 1023, m_strSettingsFilename);
    m_strFileDisplayMethod.ReleaseBuffer();
    m_nThreadPriority = GetPrivateProfileInt(_T("Plugin Settings"), _T("Thread Priority)"), THREAD_PRIORITY_HIGHEST, m_strSettingsFilename);
    m_bScaleOutput = GetPrivateProfileInt(_T("Plugin Settings"), _T("Scale Output"), FALSE, m_strSettingsFilename);
    m_bIgnoreBitstreamErrors = GetPrivateProfileInt(_T("Plugin Settings"), _T("Ignore Bitstream Errors"), FALSE, m_strSettingsFilename);
    m_bSuppressSilence = GetPrivateProfileInt(_T("Plugin Settings"), _T("Suppress Silence"), FALSE, m_strSettingsFilename);

    return TRUE;
}

BOOL CWinampSettingsDlg::SaveSettings()
{
    CString strTemp;
    
    WritePrivateProfileString(_T("Plugin Settings"), _T("File Display Method"), m_strFileDisplayMethod, m_strSettingsFilename);

    strTemp.Format(_T("%d"), m_nThreadPriority);
    WritePrivateProfileString(_T("Plugin Settings"), _T("Thread Priority"), strTemp, m_strSettingsFilename);
    
    strTemp.Format(_T("%d"), m_bScaleOutput);
    WritePrivateProfileString(_T("Plugin Settings"), _T("Scale Output"), strTemp, m_strSettingsFilename);

    strTemp.Format(_T("%d"), m_bIgnoreBitstreamErrors);
    WritePrivateProfileString(_T("Plugin Settings"), _T("Ignore Bitstream Errors"), strTemp, m_strSettingsFilename);
    
    strTemp.Format(_T("%d"), m_bSuppressSilence);
    WritePrivateProfileString(_T("Plugin Settings"), _T("Suppress Silence"), strTemp, m_strSettingsFilename);

    return TRUE;
}

BOOL CWinampSettingsDlg::Show(HWND hwndParent)
{
    m_hwndParent = hwndParent;
    DoModal();
    return TRUE;
}

BOOL CWinampSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_ctrlThreadPrioritySlider.SetRange(0, 4);
    m_ctrlThreadPrioritySlider.SetPos(GetSliderFromThreadPriority());

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CWinampSettingsDlg::OnOK()
{
    UpdateData(TRUE);
    m_nThreadPriority = GetThreadPriorityFromSlider(); 
    
    SaveSettings();

    CDialog::OnOK();
}

int CWinampSettingsDlg::GetSliderFromThreadPriority()
{
    int nSlider = 4;
    switch (m_nThreadPriority) 
    {
        case THREAD_PRIORITY_LOWEST: nSlider = 0; break;
        case THREAD_PRIORITY_BELOW_NORMAL: nSlider = 1; break;
        case THREAD_PRIORITY_NORMAL: nSlider = 2; break;
        case THREAD_PRIORITY_ABOVE_NORMAL: nSlider = 3; break;
        case THREAD_PRIORITY_HIGHEST: nSlider = 4; break;
    }
    return nSlider;
}

int CWinampSettingsDlg::GetThreadPriorityFromSlider()
{
    int nThreadPriority = THREAD_PRIORITY_HIGHEST;
    switch (m_ctrlThreadPrioritySlider.GetPos()) 
    {
        case 0: nThreadPriority = THREAD_PRIORITY_LOWEST; break;
        case 1: nThreadPriority = THREAD_PRIORITY_BELOW_NORMAL; break;
        case 2: nThreadPriority = THREAD_PRIORITY_NORMAL; break;
        case 3: nThreadPriority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        case 4: nThreadPriority = THREAD_PRIORITY_HIGHEST; break;
    }
    return nThreadPriority;
}
