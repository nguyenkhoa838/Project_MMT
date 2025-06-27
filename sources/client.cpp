#include "../includes/tasks.h"

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
    std::cout << "Type 'help' to see available commands." << std::endl;

    while (true)
    {
        std::string cmd;
        std::cout << "Enter command (or 'exit' to quit): ";
        std::getline(std::cin, cmd);

        if (cmd == "exit")
        {
            std::cout << "Exiting client." << std::endl;
            break;
        }

        // Gửi lệnh đến server
        if (send(sock, cmd.c_str(), cmd.size(), 0) == SOCKET_ERROR)
        {
            std::cerr << "Failed to send command to server." << std::endl;
            break;
        }
        std::cout << "Command sent: " << cmd << std::endl;

        // Nhận response từ server
        char buffer[1024];
        int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) 
        {
            std::cout << "Server disconnected." << std::endl;
            break;
        }
        buffer[len] = '\0';
        std::cout << "Server response: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();

    std::cout << "Connection closed." << std::endl;

    return 0;
}