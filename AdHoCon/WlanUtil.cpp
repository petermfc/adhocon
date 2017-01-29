#include "stdafx.h"
#include "WlanUtil.h"

static char profileXml[] = "<?xml version=\"1.0\"?>" \
"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">" \
"	<name>%s</name>" \
"	<SSIDConfig> " \
"		<SSID> " \
"			<name>%s</name>" \
"		</SSID>" \
"	</SSIDConfig>" \
"	<connectionType>ESS</connectionType>" \
"	<MSM>" \
"		<security>" \
"			<authEncryption>" \
"				<authentication>%s</authentication>" \
"				<encryption>%s</encryption>" \
"				<useOneX>false</useOneX>" \
"			</authEncryption>" \
"			<sharedKey>" \
"				<keyType>passPhrase</keyType>" \
"				<protected>false</protected>" \
"				<keyMaterial>%s</keyMaterial>" \
"			</sharedKey>" \
"		</security>" \
"	</MSM>" \
" </WLANProfile>";

CWlanUtil::CWlanUtil()
{
}


CWlanUtil::~CWlanUtil()
{
}

CStringA CWlanUtil::net2authA(WLAN_AVAILABLE_NETWORK *n)
{
	CStringA result;
	switch (n->dot11DefaultAuthAlgorithm) {
	case DOT11_AUTH_ALGO_80211_OPEN:
		result = "802.11 Open";
		break;
	case DOT11_AUTH_ALGO_80211_SHARED_KEY:
		result = "802.11 Shared";
		break;
	case DOT11_AUTH_ALGO_WPA:
		result = "WPA";
		break;
	case DOT11_AUTH_ALGO_WPA_PSK:
		result = "WPAPSK";
		break;
	case DOT11_AUTH_ALGO_WPA_NONE:
		result = "WPA-None";
		break;
	case DOT11_AUTH_ALGO_RSNA:
		result = "RSNA";
		break;
	case DOT11_AUTH_ALGO_RSNA_PSK:
		result = "WPA2PSK";
		break;
	default:
		result = "Other";
		break;
	}
	return result;
}

CStringA CWlanUtil::net2encA(WLAN_AVAILABLE_NETWORK *n)
{
	CStringA result;
	switch (n->dot11DefaultCipherAlgorithm) {
	case DOT11_CIPHER_ALGO_NONE:
		break;
	case DOT11_CIPHER_ALGO_WEP40:
		result = "WEP-40";
		break;
	case DOT11_CIPHER_ALGO_TKIP:
		result = "TKIP";
		break;
	case DOT11_CIPHER_ALGO_CCMP:
		result = "AES";
		break;
	case DOT11_CIPHER_ALGO_WEP104:
		result = "WEP-104";
		break;
	case DOT11_CIPHER_ALGO_WEP:
		result = "WEP";
		break;
	default:
		result = "None";
		break;
	}
	return result;
}

BOOL CWlanUtil::setProfile(HANDLE hClient, const GUID *interfaceGuid, WLAN_AVAILABLE_NETWORK *net, char *password)
{
	char buff[1024] = { 0 };
	DWORD dwResult;
	WCHAR wbuff[1024] = { 0 };
	WLAN_REASON_CODE code;
	CStringA auth, enc;
	auth = CWlanUtil::net2authA(net);
	enc = CWlanUtil::net2encA(net);
	sprintf_s(buff, 1024, profileXml, net->dot11Ssid.ucSSID, net->dot11Ssid.ucSSID, auth, enc, password);

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buff, strlen(buff), wbuff, 1024);
	dwResult = WlanSetProfile(hClient, interfaceGuid, 0, wbuff, NULL, TRUE, NULL, &code);
	if (dwResult != ERROR_SUCCESS) {
		CString strMsg;
		WCHAR buf[4096] = {};
		WlanReasonCodeToString(code, 4095, buf, NULL);
		strMsg.Format(L"WlanSetProfile failed with error: %d\n",
			dwResult);
		MessageBox(NULL, strMsg, L"Error", MB_ICONSTOP);
		MessageBox(NULL, buf, L"Reason", MB_ICONINFORMATION);
		return FALSE;
	}
	return TRUE;
}