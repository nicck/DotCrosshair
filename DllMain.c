#include <Windows.h>
#include <wchar.h>

INT g_iSize = 0;
COLORREF g_dotColor = RGB(255, 255, 255); // Default to White
HWND g_hWnd = NULL;
BOOL g_bAnyMouseButtonDown = FALSE; // Track any mouse button state
BOOL g_bAlwaysVisible = FALSE; // Whether dot should always be visible

// Function to check if any mouse button is pressed
BOOL IsAnyMouseButtonDown()
{
    return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||    // Left mouse button
           (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0 ||    // Right mouse button
           (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0 ||    // Middle mouse button
           (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0 ||   // Mouse4 (X1)
           (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;     // Mouse5 (X2)
}

typedef HWND (*CreateWindowInBand)(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam,
    DWORD dwBand);

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTaskbarRestart = 0;
    static NOTIFYICONDATAW Data = {.cbSize = sizeof(NOTIFYICONDATAW),
                                   .uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
                                   .uCallbackMessage = WM_USER,
                                   .szTip = L"DotCrosshair"};
    switch (uMsg)
    {

    case WM_CREATE:
        s_uTaskbarRestart = RegisterWindowMessageW(L"TaskbarCreated");
        Data.hWnd = hWnd;
        Data.hIcon = LoadIconW(NULL, IDI_APPLICATION);
        Shell_NotifyIconW(NIM_ADD, &Data);
        break;

    case WM_USER:
        if (lParam == WM_RBUTTONDOWN)
        {
            POINT Point = {};
            SetForegroundWindow(hWnd);
            GetCursorPos(&Point);
            HANDLE hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, IDOK, L"Exit");
            TrackPopupMenu(hMenu, TPM_LEFTBUTTON, Point.x, Point.y, 0, hWnd, NULL);
        }
        break;

    case WM_CLOSE:
    case WM_COMMAND:
        KillTimer(hWnd, 1); // Kill our timer
        Shell_NotifyIconW(NIM_DELETE, &Data);
        TerminateProcess(GetCurrentProcess(), 0);
        break;

    case WM_STYLECHANGING:
        ((LPSTYLESTRUCT)lParam)->styleNew = wParam == GWL_STYLE
                                                ? WS_VISIBLE | WS_POPUP | WS_BORDER
                                                : WS_EX_NOACTIVATE |
                                                      WS_EX_TRANSPARENT |
                                                      WS_EX_LAYERED |
                                                      WS_EX_TOOLWINDOW;
        break;

    case WM_SETTEXT:
        lParam = (LPARAM)L"DotCrosshair";
        return TRUE;
        break;

    case WM_DISPLAYCHANGE:
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0, 0);
        break;
        
    case WM_TIMER:
        // Check right mouse button state and update window visibility
        if (wParam == 1) // Our timer ID
        {
            if (!g_bAlwaysVisible) // Only check mouse state if not always visible
            {
                BOOL bCurrentState = IsAnyMouseButtonDown();
                if (bCurrentState != g_bAnyMouseButtonDown)
                {
                    g_bAnyMouseButtonDown = bCurrentState;
                    if (g_hWnd)
                    {
                        if (g_bAnyMouseButtonDown)
                        {
                            ShowWindow(g_hWnd, SW_SHOW);
                            UpdateWindow(g_hWnd);
                        }
                        else
                        {
                            ShowWindow(g_hWnd, SW_HIDE);
                            UpdateWindow(g_hWnd);
                            // Force hide with SetWindowPos as well
                            SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW);
                        }
                    }
                }
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT Paint = {};
            HDC hDC = BeginPaint(hWnd, &Paint);
            RECT rc = {};
            HBRUSH hbr = CreateSolidBrush(g_dotColor);
            GetClientRect(hWnd, &rc);
            FillRect(hDC, &rc, hbr);
            DeleteObject(hbr);
            EndPaint(hWnd, &Paint);
        }
        break;

    case WM_WINDOWPOSCHANGED:
        SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY);
        break;

    case WM_WINDOWPOSCHANGING:
        {
            HWND hForegroundWnd = GetForegroundWindow();
            RECT rc = {};
            GetClientRect(hForegroundWnd, &rc);
            POINT Point = {((rc.right - rc.left) / 2) + rc.left, ((rc.bottom - rc.top) / 2) + rc.top};
            MapWindowPoints(hForegroundWnd, HWND_DESKTOP, &Point, 1);

            *((PWINDOWPOS)lParam) = (WINDOWPOS){.hwnd = hWnd,
                                                .hwndInsertAfter = HWND_TOPMOST,
                                                .x = Point.x - g_iSize,
                                                .y = Point.y - g_iSize,
                                                .cx = g_iSize * 2,
                                                .cy = g_iSize * 2,
                                                .flags = (g_bAlwaysVisible || g_bAnyMouseButtonDown ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) |
                                                         SWP_FRAMECHANGED |
                                                         SWP_ASYNCWINDOWPOS |
                                                         SWP_NOACTIVATE};
        }
        break;
    default:
        if (s_uTaskbarRestart)
            Shell_NotifyIconW(NIM_ADD, &Data);
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

VOID WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hWnd, LONG lIdObject, LONG lIdChild, DWORD dwIdEventThread, DWORD dwMsEventTime)
{
    // Only update window position if it should be visible (always visible or any mouse button down)
    if ((g_bAlwaysVisible || g_bAnyMouseButtonDown) && g_hWnd)
    {
        SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0, 0);
    }
}

DWORD ThreadProc(LPVOID lpParameter)
{
    HMODULE hLibModule = LoadLibraryExW(L"User32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    CreateWindowInBand fnCreateWindowInBand = (CreateWindowInBand)GetProcAddress(hLibModule, "CreateWindowInBand");
    FreeLibrary(hLibModule);

    if (RegisterClassExW(&((WNDCLASSEXW){
            .cbSize = sizeof(WNDCLASSEXW),
            .hInstance = (HINSTANCE)lpParameter,
            .lpfnWndProc = WndProc,
            .hbrBackground = GetStockObject(BLACK_BRUSH),
            .lpszClassName = L"DotCrosshair"})) &&
        (g_hWnd = fnCreateWindowInBand(
             WS_EX_NOACTIVATE |
                 WS_EX_TRANSPARENT |
                 WS_EX_LAYERED |
                 WS_EX_TOOLWINDOW,
             L"DotCrosshair",
             L"DotCrosshair",
             WS_POPUP | WS_BORDER, // Removed WS_VISIBLE to start hidden
             0,
             0,
             0,
             0,
             NULL,
             NULL,
             (HINSTANCE)lpParameter,
             NULL,
             2)))
    {

        MSG Msg = {0};
        
        // Set initial window visibility based on configuration
        if (g_bAlwaysVisible)
        {
            ShowWindow(g_hWnd, SW_SHOW);
            g_bAnyMouseButtonDown = TRUE; // Pretend any mouse button is down for always visible mode
        }
        else
        {
            ShowWindow(g_hWnd, SW_HIDE);
        }
        UpdateWindow(g_hWnd);
        
        // Set up a timer to check mouse button state
        SetTimer(g_hWnd, 1, 8, NULL); // Check every ~8ms (120fps) for better responsiveness
        
        SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        // Don't call WinEventProc initially since window should be hidden
        while (GetMessageW(&Msg, NULL, 0, 0))
        {
            // Ensure window visibility matches any mouse button state
            if (g_hWnd && IsWindowVisible(g_hWnd) != g_bAnyMouseButtonDown)
            {
                ShowWindow(g_hWnd, g_bAnyMouseButtonDown ? SW_SHOW : SW_HIDE);
            }
            DispatchMessageW(&Msg);
        }
    }

    TerminateProcess(GetCurrentProcess(), 0);
    return EXIT_SUCCESS;
}

BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        LPWSTR lpFileName = NULL;
        DWORD nSize = 0;
        
        do
        {
            nSize += 1;
            lpFileName = realloc(lpFileName, sizeof(WCHAR) * nSize);
            GetModuleFileNameW(hinstDLL, lpFileName, nSize);
        }
        while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        for (DWORD dwIndex = nSize; dwIndex < -1; dwIndex -= 1)
            if (lpFileName[dwIndex] == '\\')
            {
                lpFileName[dwIndex + 1] = '\0';
                nSize = dwIndex + wcslen(L"DotCrosshair.ini");
                lpFileName = realloc(lpFileName, sizeof(WCHAR) * nSize);
                wcscat(lpFileName, L"DotCrosshair.ini");
                g_iSize = GetPrivateProfileIntW(L"Settings", L"Size", 3, lpFileName);
                g_iSize = g_iSize > 2 ? g_iSize : 2;
                
                // Read activation mode configuration
                WCHAR szActivate[32];
                if (GetPrivateProfileStringW(L"Settings", L"Activate", L"always", szActivate, sizeof(szActivate)/sizeof(WCHAR), lpFileName) > 0)
                {
                    if (_wcsicmp(szActivate, L"dynamic") == 0)
                    {
                        g_bAlwaysVisible = FALSE;
                    }
                    else
                    {
                        g_bAlwaysVisible = TRUE; // Default to always visible
                    }
                }
                
                // Read color configuration (RGB format: "R,G,B")
                WCHAR szColor[32];
                if (GetPrivateProfileStringW(L"Settings", L"Color", L"0,255,255", szColor, sizeof(szColor)/sizeof(WCHAR), lpFileName) > 0)
                {
                    int r = 0, g = 0, b = 0;
                    if (swscanf(szColor, L"%d,%d,%d", &r, &g, &b) == 3)
                    {
                        g_dotColor = RGB(r, g, b);
                    }
                }
                break;
            }
        free(lpFileName);

        CloseHandle(CreateThread(NULL, 0, ThreadProc, hinstDLL, 0, NULL));
    }
    return TRUE;
}