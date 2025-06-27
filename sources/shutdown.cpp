#include "../includes/tasks.h"

bool shutdownSystem()
{
    std::cout << "Attempting to shutdown the system..." << std::endl;
    
    // Lấy token của process hiện tại với đặc quyền shutdown
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
    // Mở process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        std::cerr << "OpenProcessToken failed: " << GetLastError() << std::endl;
        return false;
    }
    
    // Tìm LUID cho shutdown privilege
    if (!LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid))
    {
        std::cerr << "LookupPrivilegeValue failed: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    tkp.PrivilegeCount = 1;  // Một privilege để set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    
    // Kích hoạt shutdown privilege
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
    
    // Thực hiện shutdown
    // EWX_SHUTDOWN: tắt máy
    // EWX_FORCE: buộc tắt các ứng dụng không phản hồi
    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
                       SHTDN_REASON_MAJOR_APPLICATION | 
                       SHTDN_REASON_MINOR_MAINTENANCE | 
                       SHTDN_REASON_FLAG_PLANNED))
    {
        std::cerr << "ExitWindowsEx failed: " << GetLastError() << std::endl;
        CloseHandle(hToken);
        return false;
    }
    
    CloseHandle(hToken);
    std::cout << "System shutdown initiated successfully." << std::endl;
    return true;
}
