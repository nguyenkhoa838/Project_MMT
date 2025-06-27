#include "../includes/tasks.h"

// STB Image Write implementation (header-only library)
#define STB_IMAGE_WRITE_IMPLEMENTATION
namespace stb {
    // Simple PNG writer implementation
    bool write_png(const char* filename, int w, int h, int comp, const void* data, int stride_in_bytes) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        // Simple BMP format instead of PNG for simplicity
        // BMP header
        unsigned char header[54] = {
            0x42, 0x4D, 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        };
        
        int filesize = 54 + w * h * 3;
        header[2] = (unsigned char)(filesize);
        header[3] = (unsigned char)(filesize >> 8);
        header[4] = (unsigned char)(filesize >> 16);
        header[5] = (unsigned char)(filesize >> 24);
        header[18] = (unsigned char)(w);
        header[19] = (unsigned char)(w >> 8);
        header[20] = (unsigned char)(w >> 16);
        header[21] = (unsigned char)(w >> 24);
        header[22] = (unsigned char)(-h);        // Negative height for top-down
        header[23] = (unsigned char)((-h) >> 8);
        header[24] = (unsigned char)((-h) >> 16);
        header[25] = (unsigned char)((-h) >> 24);
        
        file.write((char*)header, 54);
        
        // Write pixel data (RGB format after conversion) - top-down order
        const unsigned char* pixels = (const unsigned char*)data;
        int padding = (4 - (w * 3) % 4) % 4;
        
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int idx = (y * w + x) * 3;
                // Data is now RGB, but BMP expects BGR, so reverse again
                file.put(pixels[idx + 2]); // Blue (was Red)
                file.put(pixels[idx + 1]); // Green
                file.put(pixels[idx]);     // Red (was Blue)
            }
            for (int p = 0; p < padding; p++) {
                file.put(0);
            }
        }
        
        file.close();
        return true;
    }
}

bool captureScreen(const std::string& filename)
{
    std::cout << "Capturing screen..." << std::endl;

    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create device contexts
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Create bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Copy screen to bitmap
    if (!BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY))
    {
        std::cerr << "Failed to capture screen." << std::endl;
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Create bitmap info for 24-bit RGB
    BITMAPINFOHEADER bmih = {};
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = screenWidth;
    bmih.biHeight = -screenHeight; // Negative for top-down bitmap (prevents flipping)
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = BI_RGB;

    // Calculate data size
    int stride = ((screenWidth * 3 + 3) / 4) * 4; // 4-byte alignment
    int dataSize = stride * screenHeight;
    
    // Allocate buffer for bitmap data
    unsigned char* bitmapData = new unsigned char[dataSize];

    // Get bitmap bits
    if (GetDIBits(hScreenDC, hBitmap, 0, screenHeight, bitmapData, 
                  (BITMAPINFO*)&bmih, DIB_RGB_COLORS) == 0)
    {
        std::cerr << "Failed to get bitmap data." << std::endl;
        delete[] bitmapData;
        SelectObject(hMemoryDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return false;
    }

    // Convert BGR to RGB for correct color display
    for (int i = 0; i < dataSize; i += 3)
    {
        std::swap(bitmapData[i], bitmapData[i + 2]); // Swap Blue and Red
    }

    // Make sure filename has .png extension
    std::string outputFilename = filename;
    if (outputFilename.find(".png") == std::string::npos && 
        outputFilename.find(".PNG") == std::string::npos)
    {
        outputFilename += ".png";
    }

    // Write PNG file (actually BMP for simplicity)
    bool success = stb::write_png(outputFilename.c_str(), screenWidth, screenHeight, 3, bitmapData, stride);

    // Cleanup
    delete[] bitmapData;
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    if (success)
    {
        std::cout << "Screen captured successfully: " << outputFilename << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Failed to save screenshot." << std::endl;
        return false;
    }
}
