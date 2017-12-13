#include "stdafx.h"
#include "MACDllApp.h"
#include "WinampSettingsDlg.h"

CMACDllApp g_Application;

BEGIN_MESSAGE_MAP(CMACDllApp, CWinApp)
END_MESSAGE_MAP()

CMACDllApp::CMACDllApp()
{
}

CMACDllApp::~CMACDllApp()
{
    m_spWinampSettingsDlg.Delete();
}

BOOL CMACDllApp::InitInstance()
{
    CWinApp::InitInstance();
    return TRUE;
}

CWinampSettingsDlg * CMACDllApp::GetWinampSettingsDlg()
{
    if (m_spWinampSettingsDlg == NULL)
        m_spWinampSettingsDlg.Assign(new CWinampSettingsDlg);

    return m_spWinampSettingsDlg;
}
