#pragma once
#include "stdafx.h"
#include "Win10Desktops.h"

#define DllExport   __declspec( dllexport )

#define VDA_VirtualDesktopCreated 5
#define VDA_VirtualDesktopDestroyBegin 4
#define VDA_VirtualDesktopDestroyFailed 3
#define VDA_VirtualDesktopDestroyed 2
#define VDA_ViewVirtualDesktopChanged 1
#define VDA_CurrentVirtualDesktopChanged 0

#define POST_VDA_VirtualDesktopCreated          0x20
#define POST_VDA_VirtualDesktopDestroyBegin     0x10
#define POST_VDA_VirtualDesktopDestroyFailed    0x08
#define POST_VDA_VirtualDesktopDestroyed        0x04
#define POST_VDA_ViewVirtualDesktopChanged      0x02
#define POST_VDA_CurrentVirtualDesktopChanged   0x01


#define VDA_IS_NORMAL 1
#define VDA_IS_MINIMIZED 2
#define VDA_IS_MAXIMIZED 3

std::map<HWND, int> listeners;
IServiceProvider* pServiceProvider = nullptr;
IVirtualDesktopManagerInternal *pDesktopManagerInternal = nullptr;
IVirtualDesktopManager *pDesktopManager = nullptr;
IApplicationViewCollection *viewCollection = nullptr;
IVirtualDesktopPinnedApps *pinnedApps = nullptr;
IVirtualDesktopNotificationService* pDesktopNotificationService = nullptr;
BOOL registeredForNotifications = FALSE;

std::map<HWND, int> oldWindowCmds;
std::vector<HWND> hideWindowes;
std::vector<HWND> showWindowes;
DWORD idNotificationService = 0;
BOOL _changingDesktop = false;
BOOL _keepMinimized = false;
int  postitems = (POST_VDA_VirtualDesktopCreated
	| POST_VDA_VirtualDesktopDestroyBegin
	| POST_VDA_VirtualDesktopDestroyFailed
	| POST_VDA_VirtualDesktopDestroyed
//	| POST_VDA_ViewVirtualDesktopChanged
	| POST_VDA_CurrentVirtualDesktopChanged
	);

struct ChangeDesktopAction {
	GUID newDesktopGuid;
	GUID oldDesktopGuid;
};

void _PostMessageToListeners(int msgOffset, WPARAM wParam, LPARAM lParam) {
	for each (std::pair<HWND, int> listener in listeners) {
		PostMessage(listener.first, listener.second + msgOffset, wParam, lParam);
	}
}

int _RegisterService(BOOL force = FALSE) {
	// If force == TRUE then set all the pointers to NULL
	if (force) {
		pServiceProvider = nullptr;
		pDesktopManagerInternal = nullptr;
		pDesktopManager = nullptr;
		viewCollection = nullptr;
		pinnedApps = nullptr;
		pDesktopNotificationService = nullptr;
		registeredForNotifications = FALSE;
	}

	if (pServiceProvider != nullptr) {
		return 0;
	}

	// Initialize the COM library on the current thread and identify the concurrency model as single - thread apartment(STA).
	// New applications should call CoInitializeEx instead of CoInitialize
	//Return code	Description
	//	S_OK                  The COM library was initialized successfully on this thread.
	//	S_FALSE		          The COM library is already initialized on this thread.
	//	RPC_E_CHANGED_MODE    A previous call to CoInitializeEx specified the concurrency model for this thread as multithread apartment(MTA).This could also indicate that a change from neutral - threaded apartment to single - threaded apartment has occurred.
	::CoInitialize(NULL);

	// Add pServiceProvider
	::CoCreateInstance(	CLSID_ImmersiveShell, 
		                NULL, 
		                CLSCTX_LOCAL_SERVER,
		                __uuidof(IServiceProvider),  // riid a reference to the identifier of the interface
		                (PVOID*)&pServiceProvider);  // ppv Address of pointer variable that receives the interface pointer requested in riid

	if (pServiceProvider == nullptr) {
		//std::wcout << L"FATAL ERROR: pServiceProvider is null";
		return -6;
	}
	pServiceProvider->QueryService(__uuidof(IApplicationViewCollection), &viewCollection);

	pServiceProvider->QueryService(__uuidof(IVirtualDesktopManager), &pDesktopManager);

	pServiceProvider->QueryService(
		CLSID_VirtualDesktopPinnedApps,
		__uuidof(IVirtualDesktopPinnedApps), (PVOID*)&pinnedApps);

	pServiceProvider->QueryService(
		CLSID_VirtualDesktopManagerInternal,
		__uuidof(IVirtualDesktopManagerInternal), (PVOID*)&pDesktopManagerInternal);

	if (viewCollection == nullptr) {
		//std::wcout << L"FATAL ERROR: viewCollection is null";
		return -7;
	}

	if (pDesktopManagerInternal == nullptr) {
		//std::wcout << L"FATAL ERROR: pDesktopManagerInternal is null";
		return -8;
	}

	// Notification service
	HRESULT hrNotificationService = pServiceProvider->QueryService(
		CLSID_IVirtualNotificationService,
		__uuidof(IVirtualDesktopNotificationService),
		(PVOID*)&pDesktopNotificationService);
	if (hrNotificationService == S_OK)
		return 0;
	else
		return -10;
}


IApplicationView* _GetApplicationViewForHwnd(HWND hwnd) {
	if (hwnd == 0)
		return nullptr;
	IApplicationView* app = nullptr;
	viewCollection->GetViewForHwnd(hwnd, &app);
	return app;
}

__inline BOOL CALLBACK
_EnumWindowProc_ChangeDesktop(HWND hwnd, LPARAM lParam)
{
	ChangeDesktopAction* act = (ChangeDesktopAction*)lParam;
	if (act == nullptr) {
		return TRUE;
	}
	IApplicationView* view = _GetApplicationViewForHwnd(hwnd);
	if (view != nullptr) {
		GUID winDesktopGuid;
		if (SUCCEEDED(view->GetVirtualDesktopId(&winDesktopGuid))) {
			if (winDesktopGuid == act->oldDesktopGuid) {
				//std::wcout << "Old desktop's window " << hwnd << "\r\n";

				int style;
				if ((style = GetWindowLong(hwnd, GWL_STYLE)) & WS_CHILD) // Ignore all windows with child flag set
					return TRUE;

				if (style & WS_MINIMIZE) {
					oldWindowCmds[hwnd] = VDA_IS_MINIMIZED;
				}
				else if (style & WS_MAXIMIZE) {
					oldWindowCmds[hwnd] = VDA_IS_MAXIMIZED;
				}
				else {
					oldWindowCmds[hwnd] = VDA_IS_NORMAL;
				}
				hideWindowes.push_back(hwnd);
			}
			else if (winDesktopGuid == act->newDesktopGuid) {
				showWindowes.insert(showWindowes.begin(), hwnd);
			}
		}
		view->Release();
	}
	return TRUE;
}

void _ChangeDesktop_HideOld(BOOL async = false) {
	for each (HWND win in hideWindowes)
	{
		if (async) {
			ShowWindowAsync(win, SW_SHOWMINNOACTIVE);
		}
		else {
			ShowWindow(win, SW_SHOWMINNOACTIVE);
		}
	}

}

void _ChangeDesktop_ShowNew(BOOL async = false) {
	HWND active = GetForegroundWindow();
	for each (HWND win in showWindowes)
	{
		if ((oldWindowCmds[win] == VDA_IS_MAXIMIZED || oldWindowCmds[win] == VDA_IS_NORMAL) && IsIconic(win)) {

			if (async) {
				ShowWindowAsync(win, SW_SHOWNOACTIVATE);
			}
			else {
				ShowWindow(win, SW_SHOWNOACTIVATE);
			}
			
		}
	}
}

void _ChangeDesktop_ListWins(ChangeDesktopAction act) {
	hideWindowes.clear();
	showWindowes.clear();
	EnumWindows(_EnumWindowProc_ChangeDesktop, (LPARAM)&act);

}

void DllExport EnableKeepMinimized() {
	_keepMinimized = true;
}

void DllExport RestoreMinimized() {
	std::map<HWND, int>::iterator it;
	for (it = oldWindowCmds.begin(); it != oldWindowCmds.end(); it++)
	{
		if ((it->second == VDA_IS_MAXIMIZED || it->second == VDA_IS_NORMAL) && IsIconic(it->first)) {
			ShowWindowAsync(it->first, SW_SHOWNOACTIVATE);
		}
	}
}

int DllExport GetDesktopCount()
{
	_RegisterService();

	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);
		pObjectArray->Release();
		return count;
	}

	return -1;
}

// Return the Desktop Number the HWND is on. Index starts at 0.
int DllExport GetDesktopNumberById(GUID desktopId) {
	_RegisterService();

	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	UINT    idx;
	int     found = -1;

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);

		if (SUCCEEDED(hr))
		{
			for (idx = 0; idx < count; idx++)
			{
				IVirtualDesktop *pDesktop = nullptr;

				if (FAILED(pObjectArray->GetAt(idx, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
					continue;

				GUID id = { 0 };
				if (SUCCEEDED(pDesktop->GetID(&id)) && id == desktopId)
				{
					found = idx;
					pDesktop->Release();
					break;
				}

				pDesktop->Release();
			}
		}

		pObjectArray->Release();
	}

	return found;
}

IVirtualDesktop* _GetDesktopByNumber(int number) {
	_RegisterService();

	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	IVirtualDesktop* found = nullptr;

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);
		pObjectArray->GetAt(number, __uuidof(IVirtualDesktop), (void**)&found);
		pObjectArray->Release();
	}

	return found;
}

GUID DllExport GetWindowDesktopId(HWND window) {
	_RegisterService();

	GUID pDesktopId = {};
	pDesktopManager->GetWindowDesktopId(window, &pDesktopId);

	return pDesktopId;
}

// Return the Desktop Number the HWND is on. Index starts at 0.
int DllExport GetWindowDesktopNumber(HWND window) {
	_RegisterService();

	GUID* pDesktopId = new GUID({ 0 });

	// Get a GUID pointer (pDesktopId) for the top level HWND "window"
	// Function only available in windows 10
	if (SUCCEEDED(pDesktopManager->GetWindowDesktopId(window, pDesktopId))) 
	{
		return (GetDesktopNumberById(*pDesktopId));
	}

	return -1;
}

int DllExport IsWindowOnCurrentVirtualDesktop(HWND window) {
	_RegisterService();

	BOOL b;
	if (SUCCEEDED(pDesktopManager->IsWindowOnCurrentVirtualDesktop(window, &b))) {
		return b;
	}

	return -1;
}

GUID DllExport GetDesktopIdByNumber(int number) {
	GUID id;
	IVirtualDesktop* pDesktop = _GetDesktopByNumber(number);
	if (pDesktop != nullptr) {
		pDesktop->GetID(&id);
	}
	return id;
}


int DllExport IsWindowOnDesktopNumber(HWND window, int number) {
	_RegisterService();
	IApplicationView* app = nullptr;
	if (window == 0) {
		return -1;
	}
	viewCollection->GetViewForHwnd(window, &app);
	GUID desktopId = { 0 };
	app->GetVirtualDesktopId(&desktopId);
	GUID desktopCheckId = GetDesktopIdByNumber(number);
	app->Release();
	if (desktopCheckId == GUID_NULL || desktopId == GUID_NULL) {
		return -1;
	}

	if (GetDesktopIdByNumber(number) == desktopId) {
		return 1;
	}
	else {
		return 0;
	}
	
	return -1;
}

// Move HWND windows to desktop number.  Number is 0 based. 
int DllExport MoveWindowToDesktopNumber(HWND window, int number) {
	_RegisterService();
	IVirtualDesktop* pDesktop = _GetDesktopByNumber(number);
	if (pDesktopManager == nullptr) {
		std::wcout << L"ARRGH?";
		return (-1);
	}
	if (window == 0) {
		return (-1);
	}
	if (pDesktop != nullptr) {
		GUID id = { 0 };
		if (SUCCEEDED(pDesktop->GetID(&id))) {
			IApplicationView* app = nullptr;
			viewCollection->GetViewForHwnd(window, &app);
			if (app != nullptr) {
				pDesktopManagerInternal->MoveViewToDesktop(app, pDesktop);
				return 0;
			}
		}
	}
	return -1;
}

int DllExport GetDesktopNumber(IVirtualDesktop *pDesktop) {
	_RegisterService();

	if (pDesktop == nullptr) {
		return -1;
	}

	GUID guid;

	if (SUCCEEDED(pDesktop->GetID(&guid))) {
		return GetDesktopNumberById(guid);
	}

	return -1;
}
IVirtualDesktop* GetCurrentDesktop() {
	_RegisterService();

	if (pDesktopManagerInternal == nullptr) {
		return nullptr;
	}
	IVirtualDesktop* found = nullptr;
	pDesktopManagerInternal->GetCurrentDesktop(&found);
	return found;
}

// Return the current desktup number. Indexed from 0
int DllExport GetCurrentDesktopNumber() {
	IVirtualDesktop* virtualDesktop = GetCurrentDesktop();
	int number = GetDesktopNumber(virtualDesktop);
	virtualDesktop->Release();
	return number;
}

void DllExport GoToDesktopNumber(int number) {
	_RegisterService();

	if (pDesktopManagerInternal == nullptr) {
		return;
	}

	IVirtualDesktop* oldDesktop = GetCurrentDesktop();
	GUID oldId = { 0 };
	oldDesktop->GetID(&oldId);
	oldDesktop->Release();

	IObjectArray *pObjectArray = nullptr;
	HRESULT hr = pDesktopManagerInternal->GetDesktops(&pObjectArray);
	int found = -1;
	ChangeDesktopAction act;

	if (SUCCEEDED(hr))
	{
		UINT count;
		hr = pObjectArray->GetCount(&count);

		if (SUCCEEDED(hr))
		{
			for (UINT i = 0; i < count; i++)
			{
				IVirtualDesktop *pDesktop = nullptr;

				if (FAILED(pObjectArray->GetAt(i, __uuidof(IVirtualDesktop), (void**)&pDesktop)))
					continue;

				GUID id = { 0 };
				pDesktop->GetID(&id);
				if (i == number) {
					if (_keepMinimized) {
						_changingDesktop = true;
						act.oldDesktopGuid = oldId;
						act.newDesktopGuid = id;
						_ChangeDesktop_ListWins(act);
						_ChangeDesktop_ShowNew();
					}
					pDesktopManagerInternal->SwitchDesktop(pDesktop);
				}

				pDesktop->Release();
			}
		}
		pObjectArray->Release();
	}
}

struct ShowWindowOnDesktopAction {
	int desktopNumber;
	int cmdShow;
};
//
//__inline BOOL CALLBACK
//_EnumWindowProc_ShowWindowOnDesktopAsync(HWND hwnd, LPARAM lParam)
//{
//	ShowWindowOnDesktopAction *act = (ShowWindowOnDesktopAction *) lParam;
//	if (act == nullptr) {
//		return TRUE;
//	}
//	IApplicationView* view = GetApplicationViewForHwnd(hwnd);
//	if (view != nullptr) {
//		GUID desktopId;
//		if (SUCCEEDED(view->GetVirtualDesktopId(&desktopId))) {
//			int deskNum = GetDesktopNumberById(desktopId);
//			std::wcout << "Window " << hwnd << " desk" << deskNum << " hide " << act->desktopNumber << "\r\n";
//			if (deskNum == act->desktopNumber) {
//				ShowWindowAsync(hwnd, act->cmdShow);
//			}
//		}
//	}
//	return TRUE;
//}



//void ShowWindowOnDesktopAsync(int desktopNumber, int cmdShow) {
//	ShowWindowOnDesktopAction act;
//	act.cmdShow = cmdShow;
//	act.desktopNumber = desktopNumber;
//	EnumWindows(_EnumWindowProc_ShowWindowOnDesktopAsync, (LPARAM) &act);
//}
//
//void _temp_enumAll() {
//	ShowWindowOnDesktopAsync(6, SW_MINIMIZE);
//}
//
//void _temp_GetViews() {
//	_RegisterService();
//	IObjectArray *array;
//	UINT count;
//	if (SUCCEEDED(viewCollection->GetViewsByZOrder(&array))) {
//		if (SUCCEEDED(array->GetCount(&count))) {
//			for (int i = 0; i < count; i++)
//			{
//				IApplicationView *view;
//				if (SUCCEEDED(array->GetAt(i, IID_IApplicationView, (void**)&view))) {
//					GUID desktopId;
//					// BOOL isTray;
//					// view->IsTray(&isTray);
//					if (SUCCEEDED(view->GetVirtualDesktopId(&desktopId))) {
//						int deskNum = GetDesktopNumberById(desktopId);
//						std::wcout << "On Desktop: " << deskNum << "\r\n";
//						if (deskNum == 6) {
//							UINT state;
//							int vis;
//							view->GetViewState(&state);
//							view->GetVisibility(&vis);
//
//							std::wcout << "State: " << state << " " << vis << "\r\n";
//						}
//					}
//				}
//			}
//		}
//	}
//
//}

WCHAR * _GetApplicationIdForHwnd(HWND hwnd) {
	if (hwnd == 0)
		return nullptr;
	IApplicationView* app = _GetApplicationViewForHwnd(hwnd);
	if (app != nullptr) {
		WCHAR * appId = new WCHAR[1024];
		app->GetAppUserModelId(&appId);
		app->Release();
		return appId;
	}
	return nullptr;
}

int DllExport IsPinnedWindow(HWND hwnd) {
	if (hwnd == 0)
		return -1;
	_RegisterService();
	IApplicationView* pView = _GetApplicationViewForHwnd(hwnd);
	BOOL isPinned = false;
	if (pView != nullptr) {
		pinnedApps->IsViewPinned(pView, &isPinned);
		pView->Release();
		if (isPinned) {
			return 1;
		}
		else {
			return 0;
		}
	}

	return -1;
}

void DllExport PinWindow(HWND hwnd) {
	if (hwnd == 0)
		return;
	_RegisterService();
	IApplicationView* pView = _GetApplicationViewForHwnd(hwnd);
	if (pView != nullptr) {
		pinnedApps->PinView(pView);
		pView->Release();
	}
}

void DllExport UnPinWindow(HWND hwnd) {
	if (hwnd == 0)
		return;
	_RegisterService();
	IApplicationView* pView = _GetApplicationViewForHwnd(hwnd);
	if (pView != nullptr) {
		pinnedApps->UnpinView(pView);
		pView->Release();
	}
}

int DllExport IsPinnedApp(HWND hwnd) {
	if (hwnd == 0)
		return -1;
	_RegisterService();
	WCHAR * appId = _GetApplicationIdForHwnd(hwnd);
	if (appId != nullptr) {
		BOOL isPinned = false;
		pinnedApps->IsAppIdPinned(appId, &isPinned);
		if (isPinned) {
			return 1;
		}
		else {
			return 0;
		}
	}
	return -1;
}

void DllExport PinApp(HWND hwnd) {
	if (hwnd == 0)
		return;
	_RegisterService();
	WCHAR * appId = _GetApplicationIdForHwnd(hwnd);
	if (appId != nullptr) {
		pinnedApps->PinAppID(appId);
	}
}

void DllExport UnPinApp(HWND hwnd) {
	if (hwnd == 0)
		return;
	_RegisterService();
	WCHAR * appId = _GetApplicationIdForHwnd(hwnd);
	if (appId != nullptr) {
		pinnedApps->UnpinAppID(appId);
	}
}

class _Notifications : public IVirtualDesktopNotification {
private:
	ULONG _referenceCount;
public:
	// Inherited via IVirtualDesktopNotification
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject) override
	{
		// Always set out parameter to NULL, validating it first.
		if (!ppvObject)
			return E_INVALIDARG;
		*ppvObject = NULL;

		if (riid == IID_IUnknown || riid == IID_IVirtualDesktopNotification)
		{
			// Increment the reference count and return the pointer.
			*ppvObject = (LPVOID)this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() override
	{
		return InterlockedIncrement(&_referenceCount);
	}

	virtual ULONG STDMETHODCALLTYPE Release() override
	{
		ULONG result = InterlockedDecrement(&_referenceCount);
		if (result == 0)
		{
			delete this;
		}
		return 0;
	}
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(IVirtualDesktop * pDesktop) override
	{
		if (postitems & POST_VDA_VirtualDesktopCreated)
		    _PostMessageToListeners(VDA_VirtualDesktopCreated, GetDesktopNumber(pDesktop), 0);
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override
	{
		if (postitems & POST_VDA_VirtualDesktopDestroyBegin)
		    _PostMessageToListeners(VDA_VirtualDesktopDestroyBegin, GetDesktopNumber(pDesktopDestroyed), GetDesktopNumber(pDesktopFallback));
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override
	{
		if (postitems & POST_VDA_VirtualDesktopDestroyFailed)
		_PostMessageToListeners(VDA_VirtualDesktopDestroyFailed, GetDesktopNumber(pDesktopDestroyed), GetDesktopNumber(pDesktopFallback));
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) override
	{
		if (postitems & POST_VDA_VirtualDesktopDestroyed)
		    _PostMessageToListeners(VDA_VirtualDesktopDestroyed, GetDesktopNumber(pDesktopDestroyed), GetDesktopNumber(pDesktopFallback));
		return S_OK;
	}
	virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(IApplicationView * pView) override
	{
		if (postitems & POST_VDA_ViewVirtualDesktopChanged)
		    _PostMessageToListeners(VDA_ViewVirtualDesktopChanged, 0, 0);
		return S_OK;
	}
	// This routine will be called whenever the desktop is changed by either keyboard (ctrl-win-left/right) or 
	// by the virtual desktop screen icon (next to cortana)
	// Part of IVirtualDesktopNotification
	virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(
		IVirtualDesktop *pDesktopOld,
		IVirtualDesktop *pDesktopNew) override
	{
		if (postitems & POST_VDA_CurrentVirtualDesktopChanged)
		{ 
			viewCollection->RefreshCollection();
			ChangeDesktopAction act;
			if (pDesktopOld != nullptr) {
				pDesktopOld->GetID(&act.oldDesktopGuid);
			}
			if (pDesktopNew != nullptr) {
				pDesktopNew->GetID(&act.newDesktopGuid);
			}

			// This happens at times
			if (act.oldDesktopGuid != act.newDesktopGuid && _keepMinimized) {
				if (!_changingDesktop) {
					_ChangeDesktop_ListWins(act);
					_ChangeDesktop_HideOld();
					_ChangeDesktop_ShowNew();
				}
				else {
					_ChangeDesktop_HideOld(true);
					_changingDesktop = false;
				}
			}
			// Send a message back to ourselfs indicating the desktop changed. 
			_PostMessageToListeners(VDA_CurrentVirtualDesktopChanged, GetDesktopNumberById(act.oldDesktopGuid), GetDesktopNumberById(act.newDesktopGuid));
		}
		return S_OK;
	}
};

int _RegisterDesktopNotifications() 
{
	int rc = 0;
	rc = _RegisterService();
	if (pDesktopNotificationService == nullptr) {
		return rc ;
	}
	if (registeredForNotifications) {
		return rc;
	}
	_Notifications *nf = new _Notifications();
	if (nf == NULL)
		return -8;
	HRESULT res = pDesktopNotificationService->Register(nf, &idNotificationService);
	if (SUCCEEDED(res)) {
		registeredForNotifications = TRUE;
	}
	else
		return -9;
	return rc;
}

void DllExport RestartVirtualDesktopAccessor() {
	_RegisterService(TRUE);
	_RegisterDesktopNotifications();
}

/* Call this routine early in your program to connect to the services (notifications) */

int DllExport RegisterPostMessageHook(HWND listener, int messageOffset) 
{
	int rc;
	rc = _RegisterService(FALSE);
	if (rc == 0)
	{ 
		listeners.insert(std::pair<HWND, int>(listener, messageOffset));
		if (listeners.size() != 1) 
		{
			return -2;
		}
		rc = _RegisterDesktopNotifications();
	}

	return rc;
}

/* Call this routine during cleanup in your program to unconnect the services (notifications) */

void DllExport UnregisterPostMessageHook(HWND hwnd) {
	_RegisterService(FALSE);

	listeners.erase(hwnd);
	if (listeners.size() != 0) {
		return;
	}

	if (pDesktopNotificationService == nullptr) {
		return;
	}

	if (idNotificationService > 0) {
		registeredForNotifications = TRUE;
		pDesktopNotificationService->Unregister(idNotificationService);
	}
}

VOID _OpenDllWindow(HINSTANCE injModule) {
	//_wndProdHModule = injModule;
	//CreateThread(0, NULL, _DllThreadProc, NULL, NULL, NULL);
}