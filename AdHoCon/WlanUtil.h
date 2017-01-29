#pragma once
class CWlanUtil
{
public:
	CWlanUtil();
	~CWlanUtil();

	static CStringA net2authA(WLAN_AVAILABLE_NETWORK *n);
	static CStringA net2encA(WLAN_AVAILABLE_NETWORK *n);
	static BOOL setProfile(HANDLE hClient, const GUID *interfaceGuid, WLAN_AVAILABLE_NETWORK *net, char *password);
};

