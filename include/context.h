#pragma once
#include <string>


std::string getSystemContext(const std::string root);
std::string runShellCommand(const std::string& cmd);
bool contextExists();
void generateContextFile(const std::string root);
std::string readContextFile();
bool initAgent();