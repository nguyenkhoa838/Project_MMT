#include "../includes/tasks.h"

std::atomic<bool> webcamRecording(false);
std::thread webcamThread;

void startWebcamRecording(const std::string& filename)
{
    if (webcamRecording) return;
    webcamRecording = true;

    webcamThread = std::thread([filename]() {
        cv::VideoCapture cap(0);
        if (!cap.isOpened()) return;

        int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
        int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
        cv::VideoWriter writer(filename, cv::VideoWriter::fourcc('M','J','P','G'), 10, cv::Size(width, height));

        cv::Mat frame;
        while (webcamRecording)
        {
            cap >> frame;
            if (frame.empty()) break;
            writer.write(frame);
        }

        cap.release();
        writer.release();
    });
}

void stopWebcamRecording()
{
    if (!webcamRecording) return;
    webcamRecording = false;
    if (webcamThread.joinable()) webcamThread.join();
}

bool isWebcamRecording()
{
    return webcamRecording;
}
