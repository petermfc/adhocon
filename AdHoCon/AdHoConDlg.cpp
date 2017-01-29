
// AdHoConDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdHoCon.h"
#include "AdHoConDlg.h"
#include "afxdialogex.h"
#include "PasswordInput.h"
#include <vector>
#include "WlanUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_WLAN_MESSAGE WM_USER+654

static int itemCount = 0;

CString Message;
CString xml;

HANDLE hClient = NULL;
HWND mainWindow = NULL;
DWORD dwMaxClient = 2;      //    
DWORD dwCurVersion = 0;
DWORD dwResult = 0;
DWORD dwRetVal = 0;
DWORD dwPrevNotSrc = 0;
int iRet = 0;

WCHAR GuidString[39] = { 0 };

unsigned int i, j, k;

std::vector<CString> details;
std::vector<WLAN_AVAILABLE_NETWORK>networks;

WLAN_INTERFACE_INFO currentInterface;

/* variables used for WlanEnumInterfaces  */

PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
PWLAN_INTERFACE_INFO pIfInfo = NULL;

PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;

int iRSSI = 0;



CEvent ev(0, 0);
VOID WINAPI WlanNotification(PWLAN_NOTIFICATION_DATA wlanNotifData, PVOID *p);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAdHoConDlg dialog



CAdHoConDlg::CAdHoConDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAdHoConDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAdHoConDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_wndInterface);
	DDX_Control(pDX, IDC_BUTTON_SCAN, m_wndScan);
	DDX_Control(pDX, IDC_BUTTON_CONNECT, m_wndConnect);
	DDX_Control(pDX, IDC_STATICSTATE, m_wndState);
	DDX_Control(pDX, IDC_LISTCTRL_NETWORKS, m_wndListctrlNetworks);
}

BEGIN_MESSAGE_MAP(CAdHoConDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SCAN, &CAdHoConDlg::OnBnClickedButtonScan)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CAdHoConDlg::OnBnClickedButtonConnect)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CAdHoConDlg::OnBnClickedButtonDisconnect)
	ON_MESSAGE(WM_WLAN_MESSAGE, &CAdHoConDlg::OnWlanMessage)
END_MESSAGE_MAP()


// CAdHoConDlg message handlers

BOOL CAdHoConDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	init();
	enumInterfaces();
	listInit();
	mainWindow = AfxGetMainWnd()->m_hWnd;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAdHoConDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAdHoConDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAdHoConDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAdHoConDlg::listInit()
{
	m_wndListctrlNetworks.InsertColumn(0, L"SSID");
	m_wndListctrlNetworks.InsertColumn(1, L"Signal");
	m_wndListctrlNetworks.InsertColumn(2, L"Authorization (Encryption)");
	m_wndListctrlNetworks.SetColumnWidth(0, 140);
	m_wndListctrlNetworks.SetColumnWidth(1, 120);
	m_wndListctrlNetworks.SetColumnWidth(2, 160);
}

void CAdHoConDlg::insertItem(CString item1, CString item2, CString item3)
{
	LVITEM lvi;
	CString strItem;
	{
		// Insert the first item
		lvi.mask = LVIF_TEXT;
		lvi.iItem = itemCount;
		++itemCount;
		lvi.iSubItem = 0;
		lvi.pszText = (LPTSTR)(LPCTSTR)(item1);
		m_wndListctrlNetworks.InsertItem(&lvi);
		// Set subitem 1
		strItem.Format(_T("%d"), 10 * i);
		lvi.iSubItem = 1;
		lvi.pszText = (LPTSTR)(LPCTSTR)(item2);
		m_wndListctrlNetworks.SetItem(&lvi);
		// Set subitem 2
		lvi.iSubItem = 2;
		lvi.pszText = (LPTSTR)(LPCTSTR)(item3);
		m_wndListctrlNetworks.SetItem(&lvi);
	}
}

void CAdHoConDlg::init()
{
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
	if (dwResult != ERROR_SUCCESS) {
		MessageBox(L"WlanOpenHandle failed with error: %u\n", L"Error", MB_ICONSTOP);
		// You can use FormatMessage here to find out why the function failed
	}
	dwResult = WlanRegisterNotification(hClient, WLAN_NOTIFICATION_SOURCE_ALL, FALSE,
		(WLAN_NOTIFICATION_CALLBACK)WlanNotification, NULL, NULL, &dwPrevNotSrc);
	if (dwResult != ERROR_SUCCESS) {
		MessageBox(L"WlanRegisterNotification failed with error: %u\n", L"Error", MB_ICONSTOP);
		// You can use FormatMessage here to find out why the function failed
	}
}

void CAdHoConDlg::enumInterfaces()
{
	m_wndInterface.Clear();
	m_wndState.SetWindowText(L"");
	dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		MessageBox(L"WlanEnumInterfaces failed with error: %u\n", L"Error", dwResult);
		// You can use FormatMessage here to find out why the function failed
	}
	else
	{
		wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
		wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
		for (int i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
			pIfInfo = (WLAN_INTERFACE_INFO *)&pIfList->InterfaceInfo[i];

			m_wndInterface.AddString(pIfInfo->strInterfaceDescription);

			CString strState;

			switch (pIfInfo->isState) {
			case wlan_interface_state_not_ready:
				strState = L"Not ready";
				break;
			case wlan_interface_state_connected:
				strState = L"Connected";
				break;
			case wlan_interface_state_ad_hoc_network_formed:
				strState = L"First node in a ad hoc network";
				break;
			case wlan_interface_state_disconnecting:
				strState = L"Disconnecting";
				break;
			case wlan_interface_state_disconnected:
				strState = L"Not connected";
				break;
			case wlan_interface_state_associating:
				strState = L"Attempting to associate with a network";
				break;
			case wlan_interface_state_discovering:
				strState = L"Auto configuration is discovering settings for the network";
				break;
			case wlan_interface_state_authenticating:
				strState = L"In process of authenticating";
				break;
			default:
				strState.Format(L"Unknown state %ld\n", pIfInfo->isState);
				break;
			}
			m_wndState.SetWindowText(strState);
		}
		m_wndInterface.SetCurSel(0);
		currentInterface = *pIfInfo;
	}
}

void CAdHoConDlg::scan()
{
	details.clear();
	networks.clear();
	itemCount = 0;
	m_wndListctrlNetworks.DeleteAllItems();

	dwResult = WlanScan(hClient, &pIfInfo->InterfaceGuid, NULL, NULL, NULL);
	
	
	if (dwResult != ERROR_SUCCESS) {
		CString strMsg;
		strMsg.Format(L"WlanScan failed with error: %u\n",
			dwResult);
		MessageBox(strMsg, L"Error", MB_ICONSTOP);

	}
	
	ev.Lock(10000);
	dwResult = WlanGetAvailableNetworkList(hClient,
		&pIfInfo->InterfaceGuid,
		WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES,
		NULL,
		&pBssList);
	ev.Unlock();
	ev.ResetEvent();
	if (dwResult != ERROR_SUCCESS) {
		CString strMsg;
		strMsg.Format(L"WlanGetAvailableNetworkList failed with error: %u\n",
			dwResult);
		MessageBox(strMsg, L"Error", MB_ICONSTOP);

	}
	else {
		
		for (j = 0; j < pBssList->dwNumberOfItems; j++) {
			CString strName;
			pBssEntry =
				(WLAN_AVAILABLE_NETWORK *)& pBssList->Network[j];
			if ((pBssEntry->dwFlags != WLAN_AVAILABLE_NETWORK_HAS_PROFILE)
				/*&& (pBssEntry->dot11BssType == dot11_BSS_type_independent)*/)
			{
				networks.push_back(*pBssEntry);
			}
		}
		findDuplicates();
		for each(auto n in networks)
		{
			CString tmp, strName;
			if (n.wlanSignalQuality == 0)
				iRSSI = -100;
			else if (n.wlanSignalQuality == 100)
				iRSSI = -50;
			else
				iRSSI = -100 + (n.wlanSignalQuality / 2);
			tmp.Format(L"%u%% (RSSI: %i dBm)", n.wlanSignalQuality, iRSSI);
			if (n.dot11Ssid.uSSIDLength == 0)
				strName = L"";
			else {
				for (unsigned int k = 0; k < n.dot11Ssid.uSSIDLength; k++) {
					strName.AppendFormat(L"%c", (int)n.dot11Ssid.ucSSID[k]);
				}
			}
			CString auth;
			switch (n.dot11DefaultAuthAlgorithm) {
			case DOT11_AUTH_ALGO_80211_OPEN:
				auth = L"802.11 Open";
				break;
			case DOT11_AUTH_ALGO_80211_SHARED_KEY:
				auth = L"802.11 Shared";
				break;
			case DOT11_AUTH_ALGO_WPA:
				auth = L"WPA";
				break;
			case DOT11_AUTH_ALGO_WPA_PSK:
				auth = L"WPAPSK";
				break;
			case DOT11_AUTH_ALGO_WPA_NONE:
				auth = L"WPA-None";
				break;
			case DOT11_AUTH_ALGO_RSNA:
				auth = L"RSNA";
				break;
			case DOT11_AUTH_ALGO_RSNA_PSK:
				auth = L"WPA2PSK\r\n";
				break;
			default:
				auth = L"Other\r\n";
				break;
			}

			switch (n.dot11DefaultCipherAlgorithm) {
			case DOT11_CIPHER_ALGO_NONE:
				break;
			case DOT11_CIPHER_ALGO_WEP40:
				auth += L" (WEP-40)";
				break;
			case DOT11_CIPHER_ALGO_TKIP:
				auth += L" (TKIP)";
				break;
			case DOT11_CIPHER_ALGO_CCMP:
				auth += L" (AES)";
				break;
			case DOT11_CIPHER_ALGO_WEP104:
				auth += L" (WEP-104)";
				break;
			case DOT11_CIPHER_ALGO_WEP:
				auth += L" (WEP)";
				break;
			default:
				break;
			}
			insertItem(strName, tmp, auth);
		}
	}
}

void CAdHoConDlg::OnBnClickedButtonScan()
{
	scan();
}

void CAdHoConDlg::OnBnClickedButtonConnect()
{
	int index = -1;

	POSITION p = m_wndListctrlNetworks.GetFirstSelectedItemPosition()-1;
	index = (int)p;
	if (index == -1)
		MessageBox(L"Select network from the list.", L"Warning", MB_ICONWARNING);
	else
	{
		WLAN_CONNECTION_PARAMETERS conn;
		WLAN_AVAILABLE_NETWORK pNet = networks.at(index);
		conn.wlanConnectionMode = pNet.bSecurityEnabled ? wlan_connection_mode_discovery_secure : wlan_connection_mode_discovery_unsecure;
		if (conn.wlanConnectionMode == wlan_connection_mode_discovery_unsecure)
		{

			conn.strProfile = NULL;
			conn.pDot11Ssid = &pNet.dot11Ssid;
			conn.dot11BssType = pNet.dot11BssType;
			conn.pDesiredBssidList = NULL;
			conn.dwFlags = WLAN_CONNECTION_ADHOC_JOIN_ONLY;
		}
		else
		{
			WCHAR wbuff[1024] = { 0 };
			char *ssid = (char *)pNet.dot11Ssid.ucSSID;
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ssid, strlen(ssid), wbuff, 1024);
			CPasswordInput pi;
			if (pi.DoModal() == IDOK)
			{

				CWlanUtil::setProfile(hClient, &currentInterface.InterfaceGuid, &networks.at(index), pi.getPassword().GetBuffer());

				conn.strProfile = wbuff;
				conn.pDot11Ssid = &pNet.dot11Ssid;
				conn.dot11BssType = pNet.dot11BssType;
				conn.pDesiredBssidList = NULL;
				conn.dwFlags = 0;
			}
			else
				return;
		}
		GUID *pGuid = &currentInterface.InterfaceGuid;
		dwResult = WlanConnect(hClient, pGuid, &conn, NULL);

		if (dwResult != ERROR_SUCCESS) {
			CString strMsg;
			WCHAR buf[4096] ={};
			WlanReasonCodeToString(dwResult, 4095, buf, NULL);
			strMsg.Format(L"WlanConnect failed with error: %d\n",
				dwResult);
			MessageBox(strMsg, L"Error", MB_ICONSTOP);


			// You can use FormatMessage to find out why the function failed
		}
		else
		{
			this->enumInterfaces();
		}
	}
}

void CAdHoConDlg::exit()
{
	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}

	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}
}

/**
findDuplicates()
Finds and erases duplicates of current connected-to network.
*/
void CAdHoConDlg::findDuplicates()
{
	char ssid[256] = { 0 };
	std::vector<unsigned int>toRemove;
	for each(WLAN_AVAILABLE_NETWORK n in networks)
	{
		if (n.dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
		{
			strcpy_s(ssid, 255, (char *)(n.dot11Ssid.ucSSID));
			for (unsigned int i = 0; i < networks.size(); ++i)
			{
				if ((!memcmp(networks[i].dot11Ssid.ucSSID, n.dot11Ssid.ucSSID, 32))
					&& (networks[i].dwFlags != n.dwFlags))
				{
					toRemove.push_back(i);
				}
			}
		}
	}
	for each(auto i in toRemove)
	{
		networks.erase(networks.begin() + i);
	}
}

void CAdHoConDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	this->exit();
	CDialogEx::OnClose();
}
void CAdHoConDlg::OnOK()
{
	// TODO: Add your specialized code here and/or call the base class

	//CDialogEx::OnOK();
}

void CAdHoConDlg::OnBnClickedButtonDisconnect()
{
	WlanDisconnect(hClient, &currentInterface.InterfaceGuid, NULL);
	this->enumInterfaces();
}

LRESULT CAdHoConDlg::OnWlanMessage(UINT wParam, LONG lParam)
{
	m_wndState.SetWindowText(*(CString *)wParam);
	return 0;
}

VOID WINAPI WlanNotification(PWLAN_NOTIFICATION_DATA wlanNotifData, PVOID *p)
{
	switch(wlanNotifData->NotificationCode)
	{
	case wlan_notification_acm_scan_complete:
		ev.SetEvent();
		break;
	case wlan_notification_acm_connection_start:
		Message = "Connecting...";
		::SendMessage(mainWindow, WM_WLAN_MESSAGE, (WPARAM)&Message, (WPARAM)0);
		break;
	case wlan_notification_acm_connection_complete:
		Message = "Connected";
		::SendMessage(mainWindow, WM_WLAN_MESSAGE, (WPARAM)&Message, (WPARAM)0);
		break;
	case wlan_notification_acm_connection_attempt_fail:
	{
		WLAN_CONNECTION_NOTIFICATION_DATA *conn = (WLAN_CONNECTION_NOTIFICATION_DATA *)wlanNotifData->pData;
		WCHAR buf[4096] = { 0 };
		WlanReasonCodeToString(conn->wlanReasonCode, 4095, buf, NULL);
		Message.Format(L"Connection failed with error: %s\n",
			buf);
		::SendMessage(mainWindow, WM_WLAN_MESSAGE, (WPARAM)&Message, (WPARAM)0);
		break;
	}
	case wlan_notification_acm_disconnecting:
		Message = "Disconnecting...";
		::SendMessage(mainWindow, WM_WLAN_MESSAGE, (WPARAM)&Message, (WPARAM)0);
		break;
	case wlan_notification_acm_disconnected:
		Message = "Not connected";
		::SendMessage(mainWindow, WM_WLAN_MESSAGE, (WPARAM)&Message, (WPARAM)0);
		break;

	}
}