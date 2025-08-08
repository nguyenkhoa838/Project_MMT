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

static std::atomic<bool> checkWebcam(false);
static cv::VideoWriter videoWriter;

void startWebcamRecording(const std::string& filename)
{
    if (checkWebcam)
    {
        std::cerr << "Webcam is already recording." << std::endl;
        return;
    }

    videoWriter.open(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 30, cv::Size(640, 480));
    if (!videoWriter.isOpened())
    {
        std::cerr << "Error: Could not open video writer." << std::endl;
        return;
    }

    checkWebcam = true;
    std::cout << "Webcam recording started: " << filename << std::endl;

    // Start a separate thread to capture frames
    std::thread([&]() {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            std::cerr << "Error: Could not open webcam." << std::endl;
            checkWebcam.store(false);
            return;
        }

        cv::Mat frame;
        while (checkWebcam)
        {
            cap >> frame;
            if (frame.empty())
            {
                std::cerr << "Error: Could not capture frame from webcam." << std::endl;
                break;
            }
            videoWriter.write(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        cap.release();
        videoWriter.release();
        checkWebcam = false;
        std::cout << "Webcam recording stopped." << std::endl;
    }).detach();
}

void stopWebcamRecording()
{
    checkWebcam = false;
    videoWriter.release();
    std::cout << "Webcam recording stopped." << std::endl;
}

bool isWebcamRecording()
{
    // Check if webcam is currently recording
    return checkWebcam;
}