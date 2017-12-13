#pragma once

class CWinampSettingsDlg;

class CMACDllApp : public CWinApp
{
public:

    CMACDllApp();
    ~CMACDllApp();

    CWinampSettingsDlg * GetWinampSettingsDlg();

    virtual BOOL InitInstance();

protected:

    DECLARE_MESSAGE_MAP();

    CSmartPtr<CWinampSettingsDlg> m_spWinampSettingsDlg;
};

extern CMACDllApp g_Application;