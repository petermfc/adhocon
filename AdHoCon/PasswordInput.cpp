// PasswordInput.cpp : implementation file
//

#include "stdafx.h"
#include "AdHoCon.h"
#include "PasswordInput.h"
#include "afxdialogex.h"


// CPasswordInput dialog

IMPLEMENT_DYNAMIC(CPasswordInput, CDialogEx)

CPasswordInput::CPasswordInput(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPasswordInput::IDD, pParent)
{

}

CPasswordInput::~CPasswordInput()
{
}

void CPasswordInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_wndPassword);
}


BEGIN_MESSAGE_MAP(CPasswordInput, CDialogEx)
END_MESSAGE_MAP()


// CPasswordInput message handlers
CStringA CPasswordInput::getPassword()
{
	char buff[1024] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, m_password, -1, buff, 1024, 0, 0);

	CStringA result(buff);
	return result;

}

void CPasswordInput::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class
	m_wndPassword.GetWindowText(m_password);
	CDialogEx::OnOK();
}
