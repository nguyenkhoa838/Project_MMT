#include "process_utils.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <cwchar>

std::string listProcesses()
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateToolhelp32Snapshot failed with error: " << GetLastError() << std::endl;
        return "";
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe))
    {
        std::cerr << "Process32First failed with error: " << GetLastError() << std::endl;
        CloseHandle(hSnapshot);
        return "";
    }

    std::ofstream outFile("process_list.txt");
    if (!outFile.is_open())
    {
        std::cerr << "Failed to open process_list.txt for writing." << std::endl;
        CloseHandle(hSnapshot);
        return "";
    }
    
    if (Process32First(hSnapshot, &pe))
    {
        do
        {
            outFile << "Process ID: " << pe.th32ProcessID << ", Name: " << pe.szExeFile << std::endl;
        } while (Process32Next(hSnapshot, &pe));
    }
    else
    {
        std::cerr << "Process32Next failed with error: " << GetLastError() << std::endl;
    }

    CloseHandle(hSnapshot);
    outFile.close();
    std::cout << "Process list saved to process_list.txt." << std::endl;
    return "process_list.txt";
}

bool startProcess(const std::string& path)
{
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(path.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "Process started successfully: " << path << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Failed to start process: " << path << ", error: " << GetLastError() << std::endl;
        return false;
    }
}

bool stopProcess(const std::string& exeName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        std::cerr << "CreateToolhelp32Snapshot failed with error: " << GetLastError() << std::endl;
        return false;
    }

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe))
    {
        std::cerr << "Process32First failed with error: " << GetLastError() << std::endl;
        CloseHandle(hSnapshot);
        return false;
    }

    // Convert exeName to wide string
    std::wstring targetName(exeName.begin(), exeName.end());

    do
    {
        if (_wcsicmp(pe.szExeFile, targetName.c_str()) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (hProcess == NULL)
            {
                std::cerr << "OpenProcess failed for " << pe.szExeFile << ", error: " << GetLastError() << std::endl;
                CloseHandle(hSnapshot);
                return false;
            }

            if (TerminateProcess(hProcess, 0))
            {
                std::cout << "Process terminated successfully: " << pe.szExeFile << std::endl;
                CloseHandle(hProcess);
                CloseHandle(hSnapshot);
                return true;
            }
            else
            {
                std::cerr << "Failed to terminate process: " << pe.szExeFile << ", error: " << GetLastError() << std::endl;
                CloseHandle(hProcess);
                CloseHandle(hSnapshot);
                return false;
            }
        }
    } while (Process32NextW(hSnapshot, &pe));
    
    std::cerr << "Process not found: " << exeName << std::endl;
    CloseHandle(hSnapshot);

    return false;
}