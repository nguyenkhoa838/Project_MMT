#include "../includes/tasks.h"

bool restartSystem()
{
    std::cout << "Attempting to restart the system..." << std::endl;
    
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        std::cerr << "OpenProcessToken failed: " << GetLastError() << std::endl;
        return false;
    }
    
    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
        std::cerr << "LookupPrivilegeValue failed: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        std::cerr << "AdjustTokenPrivileges failed: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        std::cerr << "The token does not have the specified privilege." << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 
                       SHTDN_REASON_MAJOR_APPLICATION | 
                       SHTDN_REASON_MINOR_MAINTENANCE | 
                       SHTDN_REASON_FLAG_PLANNED))
    {
        std::cerr << "ExitWindowsEx failed: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    CloseHandle(hToken);
    std::cout << "System restart initiated successfully." << std::endl;
    return true;
}