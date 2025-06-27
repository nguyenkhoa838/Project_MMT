#pragma once
#include <string>

std::string listProcesses();
bool startProcess(const std::string& path);
bool stopProcess(const std::string& exeName);