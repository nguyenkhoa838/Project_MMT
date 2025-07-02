#include "../includes/tasks.h"

bool copyFile(const std::string& sourcePath, const std::string& destPath)
{
    try
    {
        std::filesystem::copy(sourcePath, destPath, std::filesystem::copy_options::overwrite_existing);
        std::cout << "File copied successfully from " << sourcePath << " to " << destPath << std::endl;
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Failed to copy file: " << e.what() << std::endl;
        return false;
    }
}