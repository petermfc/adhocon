
// AdHoConDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CAdHoConDlg dialog
class CAdHoConDlg : public CDialogEx
{
// Construction
public:
	CAdHoConDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_ADHOCON_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
private:
	void listInit();
	void insertItem(CString, CString, CString);

	void init();
	void enumInterfaces();
	void scan();
	void exit();
	void findDuplicates();
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_wndInterface;
	CButton m_wndScan;
	CButton m_wndConnect;
	CStatic m_wndState;
	afx_msg void OnBnClickedButtonScan();
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnBnClickedButtonDisconnect();
	LRESULT OnWlanMessage(UINT wParam, LONG lParam);
	CListCtrl m_wndListctrlNetworks;
};
