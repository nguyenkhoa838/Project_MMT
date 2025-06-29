#include "../includes/tasks.h"

bool isAppProcess(DWORD pid)
{
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd)
    {
        DWORD windowPID;
        GetWindowThreadProcessId(hwnd, &windowPID);
        if (windowPID == pid && IsWindowVisible(hwnd))
        {
            return true; // This process has a visible window
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return false; // No visible window found for this process
}

std::string wcharToString(const wchar_t* wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0)
    {
        return "";
    }

    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], size_needed, nullptr, nullptr);
    return str;
}

std::string listUserApps()
{
    std::string result;

    std::ofstream fout("list_apps.txt");
    if (!fout.is_open())
    {
        return "Failed to open file for writing.";
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return "Failed to create process snapshot.";
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    std::set<std::string> seen;

    if (Process32FirstW(hSnapshot, &pe))
    {
        do
        {
            if (isAppProcess(pe.th32ProcessID))
            {
                std::string appName = wcharToString(pe.szExeFile);
                if (seen.find(appName) == seen.end())
                {
                    seen.insert(appName);
                    fout << "Process ID: " << pe.th32ProcessID << ", Name: " << appName << "\n";
                    result += "Process ID: " + std::to_string(pe.th32ProcessID) + ", Name: " + appName + "\n";
                }
            }
        } while (Process32NextW(hSnapshot, &pe));
    }
    else
    {
        CloseHandle(hSnapshot);
        return "Failed to retrieve process information.";
    }

    CloseHandle(hSnapshot);
    fout.close();

    return result;
}