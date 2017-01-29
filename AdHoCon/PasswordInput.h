#pragma once
#include "afxwin.h"


// CPasswordInput dialog

class CPasswordInput : public CDialogEx
{
	DECLARE_DYNAMIC(CPasswordInput)

public:
	CPasswordInput(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPasswordInput();

// Dialog Data
	enum { IDD = IDD_DIALOG_PASSWORD_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	CEdit m_wndPassword;
	CString m_password;
	DECLARE_MESSAGE_MAP()
public:
	CStringA getPassword();
	virtual void OnOK();
};
