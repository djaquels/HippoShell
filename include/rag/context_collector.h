#pragma once
#include <string>

struct ContextBlock {
    std::string label;
    std::string command;
    std::string result;
};
std::string trim(std::string& str);
std::string detectOS();
std::string getInstalledPackagesCommand(const std::string& distro);
void runBlock(ContextBlock& block);
void collectContext(const std::string& outputPath = "/tmp/jeanne_context_rag.txt");