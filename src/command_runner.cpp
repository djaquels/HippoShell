#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <cctype>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include <fcntl.h>

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
    int out_pipe[2], err_pipe[2];
    pipe(out_pipe);
    pipe(err_pipe);

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        close(out_pipe[0]); // Close read end of stdout pipe
        close(err_pipe[0]); // Close read end of stderr pipe

        dup2(out_pipe[1], STDOUT_FILENO); // Redirect stdout
        dup2(err_pipe[1], STDERR_FILENO); // Redirect stderr

        close(out_pipe[1]);
        close(err_pipe[1]);

        // Execute the command using /bin/sh -c
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char*)nullptr);
        // If execl fails
        perror("execl");
        exit(1);
    } else {
        // Parent process
        close(out_pipe[1]); // Close write end
        close(err_pipe[1]);

        // Set both pipes to non-blocking
        fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);
        fcntl(err_pipe[0], F_SETFL, O_NONBLOCK);

        char buffer[256];
        bool out_open = true, err_open = true;
        bool std_error_flag = false;
        while (out_open || err_open) {
            ssize_t n = read(out_pipe[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0';
                std::cout << "[stdout] " << buffer << std::flush;
            } else if (n == 0) {
                out_open = false;
            }

            n = read(err_pipe[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                std_error_flag = true;
                buffer[n] = '\0';
                std::cerr << "[stderr] " << buffer << std::flush;
            } else if (n == 0) {
                err_open = false;
            }

            // Avoid busy-waiting
            usleep(10000); // Sleep 10ms
        }
        close(out_pipe[0]);
        close(err_pipe[0]);
        waitpid(pid, nullptr, 0); // Wait for child to exit
        return std_error_flag ? "something failed while running command" : "success!";
    }
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