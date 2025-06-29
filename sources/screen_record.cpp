#include "../includes/tasks.h"

static std::thread recordThread;
static std::atomic<bool> recording(false);

cv::Mat captureScreenMat()
{
    HWND hDesktop = GetDesktopWindow();
    HDC hdcScreen = GetDC(hDesktop);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    RECT desktopRect;
    GetClientRect(hDesktop, &desktopRect);
    int width = desktopRect.right;
    int height = desktopRect.bottom;

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // Negative to flip the image
    bi.biPlanes = 1;
    bi.biBitCount = 24; // 24 bits per pixel
    bi.biCompression = BI_RGB;

    cv::Mat mat(height, width, CV_8UC3);
    GetDIBits(hdcMem, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hDesktop, hdcScreen);

    cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB); // Convert from BGR to RGB
    return mat;
}

void startScreenRecording(const std::string& filename)
{
    if (recording)
    {
        std::cerr << "Screen recording is already in progress." << std::endl;
        return;
    }

    recording = true;
    recordThread = std::thread([filename]()
    {
        int fps = 10; // Frames per second
        cv::Mat frame = captureScreenMat();
        int width = frame.cols;
        int height = frame.rows;

        cv::VideoWriter writer(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, cv::Size(width, height));
        if (!writer.isOpened())
        {
            std::cerr << "Failed to open video writer." << std::endl;
            recording = false;
            return;
        }

        while (recording)
        {
            frame = captureScreenMat();
            writer.write(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps)); // Sleep to maintain FPS
        }

        writer.release();
    });
}

void stopScreenRecording()
{
    recording = false;
    if (recordThread.joinable())
    {
        recordThread.join();
    }
}

bool isRecording()
{
    return recording;
}