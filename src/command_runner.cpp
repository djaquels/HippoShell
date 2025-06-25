#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <cctype>
#include <bits/stdc++.h>

std::string cleanCommand(const std::string& raw) {
    std::string cleaned = raw;

    // Replace \u0026 with &
    cleaned = std::regex_replace(cleaned, std::regex(R"(\\u0026)"), "&");

    // Replace \n or \\n with actual newline or space
    cleaned = std::regex_replace(cleaned, std::regex(R"(\\n|\n)"), " ");

    // Replace escaped quotes
    cleaned = std::regex_replace(cleaned, std::regex(R"(\\")"), "\"");

    // Remove leading/trailing whitespace
    cleaned = std::regex_replace(cleaned, std::regex(R"(^\s+|\s+$)"), "");

    // Remove outer quotes (optional)
    if (cleaned.front() == '"' && cleaned.back() == '"') {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }

    // Collapse multiple spaces
    cleaned = std::regex_replace(cleaned, std::regex(R"(\s{2,})"), " ");

    return cleaned;
}

bool askUserToConfirm(const std::string& cmd) {
    std::string choice;
    std::string cmd_custom;
    std::cout << "Suggested command: " << cmd << "\nRun it? (y/n/edit): ";
    std::getline(std::cin, choice);
    if (choice == "y") return true;
    else if (choice == "edit") {
        std::cout << "Edit command: ";
        std::getline(std::cin, cmd_custom);
        return true;
    }
    return false;
}

std::string runCommand(const std::string& cmd) {
    std::string result;
    char buffer[128];
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error: Failed to run command.";
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

std::string parseCommand(const std::string& llmOutput) {
    std::stringstream result;
    std::istringstream stream(llmOutput);
    std::string line;
    bool insideCodeBlock = false;

    while (std::getline(stream, line)) {
        // Trim leading/trailing whitespace
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

        if (line.rfind("```", 0) == 0) {
            insideCodeBlock = !insideCodeBlock;
            continue;
        }

        if (insideCodeBlock || std::regex_search(line, std::regex("^[a-zA-Z0-9_/]+\\s"))) {
            if (!result.str().empty()) {
                result << " \\\n";
            }
            result << line;
        }
    }

    // Fallback: if no code blocks, try to extract single line command with heuristic
    if (result.str().empty()) {
        std::smatch match;
        std::regex commandRegex(R"(([a-zA-Z]+\s[^\n`]+))");
        if (std::regex_search(llmOutput, match, commandRegex)) {
            return match.str(1);
        }
    }

    return result.str();
    //std::string cleanedCmd = cleanCommand(result.str());
    //return cleanedCmd;

}