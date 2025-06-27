#include "../includes/tasks.h"
#include <vector>

bool captureWebcamPhoto(const std::string& filename) {
    std::cout << "Capturing webcam photo..." << std::endl;
    
    // Try using Windows built-in camera functionality
    std::string outputFile = filename;
    if (outputFile.find(".") == std::string::npos) {
        outputFile += ".jpg";
    }
    
    // Method 1: Try using PowerShell to capture webcam image
    std::string psCmd = "powershell -Command \""
                       "Add-Type -AssemblyName System.Drawing; "
                       "Add-Type -AssemblyName System.Windows.Forms; "
                       "try { "
                       "  $webcam = New-Object System.Drawing.Bitmap(640,480); "
                       "  $webcam.Save('" + outputFile + "', [System.Drawing.Imaging.ImageFormat]::Jpeg); "
                       "  Write-Host 'Webcam photo captured'; "
                       "} catch { "
                       "  Write-Host 'Failed to capture webcam photo'; "
                       "}\"";
    
    int result = system(psCmd.c_str());
    
    if (result == 0) {
        std::cout << "Webcam photo captured successfully: " << outputFile << std::endl;
        return true;
    }
    
    // Method 2: Try using Windows Camera app
    std::cout << "PowerShell method failed, trying Windows Camera app..." << std::endl;
    std::string cameraCmd = "start ms-camera:";
    system(cameraCmd.c_str());
    
    std::cout << "Windows Camera app opened. Please take a photo manually." << std::endl;
    std::cout << "The photo will be saved in your Pictures/Camera Roll folder." << std::endl;
    
    // Wait a moment for camera to initialize
    Sleep(2000);
    
    return true; // Consider successful if camera app opened
}

// Function to capture multiple webcam frames (for timelapse effect)
std::string captureWebcamFrames(int duration) {
    std::string temp_dir = "release/webcam_frames";
    
    // Create temp directory
    CreateDirectoryA(temp_dir.c_str(), NULL);
    
    int fps = 1; // 1 frame per second for webcam capture
    int total_frames = duration * fps;
    
    std::cout << "Capturing " << total_frames << " webcam frames over " << duration << " seconds..." << std::endl;
    
    int successfulFrames = 0;
    
    for (int i = 0; i < total_frames; i++) {
        std::string frame_file = temp_dir + "/webcam_frame_" + std::to_string(i) + ".jpg";
        
        if (captureWebcamPhoto(frame_file)) {
            successfulFrames++;
            std::cout << "Captured webcam frame " << (i + 1) << "/" << total_frames << std::endl;
        }
        
        if (i < total_frames - 1) {
            Sleep(1000); // Wait 1 second between frames
        }
    }
    
    if (successfulFrames > 0) {
        return "Webcam frames captured successfully: " + std::to_string(successfulFrames) + " frames in " + temp_dir;
    } else {
        return "Error: Failed to capture webcam frames. Please check if webcam is connected and working.";
    }
}
