#pragma once
#include <string>

std::string sendToOllama(const std::string& prompt);
std::string extractResponseField(const std::string& jsonString);