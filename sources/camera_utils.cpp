#include "../includes/tasks.h"

bool captureWebcamPhoto(const std::string& filename)
{
    cv::VideoCapture cap(0); // Open default camera
    if (!cap.isOpened())
    {
        std::cerr << "Error: Could not open webcam." << std::endl;
        return false;
    }
    cv::Mat frame;
    cap >> frame; // Capture a single frame

    if (frame.empty())
    {
        std::cerr << "Error: Could not capture frame from webcam." << std::endl;
        return false;
    }

    // Ensure filename has .jpg extension
    std::string outputFilename = filename;
    if (outputFilename.find(".jpg") == std::string::npos && 
        outputFilename.find(".jpeg") == std::string::npos)
    {
        outputFilename += ".jpg";
    }
    if (!cv::imwrite(outputFilename, frame))
    {
        std::cerr << "Error: Could not save image to " << outputFilename << std::endl;
        return false;
    }
    std::cout << "Webcam photo captured successfully: " << outputFilename << std::endl;
    return true;
}