#include "../includes/tasks.h"

std::string access_token;

bool refreshAccessToken() 
{
    int result = system("python ../scripts/refresh_token.py");
    if (result != 0)
    {
        std::cerr << "Failed to refresh access token." << std::endl;
        return false;
    }

    std::ifstream in("../scripts/access_token.txt");
    if (!in.is_open()) 
    {
        std::cerr << "Failed to open access token file." << std::endl;
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

bool sendEmail(const std::string& subject, const std::string& body)
{
    std::string command = "python ../scripts/send_email.py \"" + subject + "\" \"" + body + "\"";
    int result = system(command.c_str());
    if (result != 0)
    {
        std::cerr << "Failed to send email." << std::endl;
        return false;
    }
    std::cout << "Email sent successfully." << std::endl;
    return true;
}

std::string readLastEmailCommand()
{
    int result = system("python ../scripts/read_mail.py");
    if (result != 0)
    {
        std::cerr << "Failed to read last email command." << std::endl;
        return "";
    }

    std::ifstream in("../scripts/last_command.txt");
    if (!in.is_open()) 
    {
        std::cerr << "Failed to open last command file." << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();
    std::string lastCommand = buffer.str();
    if (lastCommand.empty()) 
    {
        std::cerr << "Last command is empty." << std::endl;
        return "";
    }
    std::cout << "Last command read successfully." << std::endl;
    return lastCommand;
}


void startGmailControlLoop()
{
    while (true)
    {
        refreshAccessToken();
        std::string cmd = readLastEmailCommand();
        
    }
}