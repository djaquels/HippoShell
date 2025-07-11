#pragma once
#include <string>

std::string askUserToConfirm(const std::string& cmd);
std::string parseCommand(const std::string& llmOutput);
std::string runCommand(const std::string& command);
std::string cleanCommand(const std::string& raw);
