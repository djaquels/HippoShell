#include <string>
#include <vector>
#include <iostream>
#include <array>
#include <filesystem>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <array>
#include <cstdio>
#include <thread>

std::atomic<bool> loadingDone(false); 

const std::string CONTEXT_FILE = "/tmp/jeanne_context.txt";


void showLoadingBar() {
    const std::string spinner = "|/-\\";
    int pos = 0;

    std::cout << "\n⏳ Creating model, please wait ";
    std::cout.flush();

    while (!loadingDone.load()) {
        std::cout << "\b" << spinner[pos++ % spinner.size()];
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    std::cout << "\b✔️ Done!" << std::endl;
}

std::string runShellCommandWithSpinner(const std::string& cmd) {
    loadingDone.store(false);
    std::thread spinnerThread(showLoadingBar);

    std::array<char, 256> buffer;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
	loadingDone.store(true);
	spinnerThread.join();
	return "Error running command";
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
	result += buffer.data();
    }
    int status = pclose(pipe);
    loadingDone.store(true);
    spinnerThread.join();
    return (status == 0) ? result : "Error running command";
}

std::string runShellCommand(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "Error running command";
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    pclose(pipe);
    return result;
}

std::string getSystemContext(const std::string root) {
    std::string context;
    context += "Config file: " + CONTEXT_FILE + "\n";
    context += "Kernel: ";
    context += runShellCommand("uname -a");
    context += "\nOS Version: ";
    context += runShellCommand("lsb_release -ds 2> /dev/null || cat /etc/*release 2> /dev/null | head -n1");
    /*context += "\n\n### Directory structure ("+root+")"+":\n";
    context += runShellCommand("tree "+ root + " -L 10");

    context += "\n\n### Available Binaries:\n";
    context += runShellCommand("which docker && docker --version");
    context += runShellCommand("which git && git --version");

    context += "\n### Installed Packages (dpkg):\n";
    context += runShellCommand("dpkg -l | grep '^ii' | head -n 10");

    context += "\n### Snap Packages:\n";
    context += runShellCommand("snap list 2>/dev/null | head -n 10");

    context += "\n### Open Ports:\n";
    context += runShellCommand("ss -tuln | head -n 10");

    context += "\n### Disk Usage:\n";
    context += runShellCommand("df -h");

    context += "\n### Mount Points:\n";
    context += runShellCommand("mount | grep '^/dev' | head -n 10");

    context += "\n### GPU Info:\n";
    context += runShellCommand("lspci | grep VGA");

    context += "\n### Battery Info:\n";
    context += runShellCommand("upower -i $(upower -e | grep BAT) 2>/dev/null | grep -E 'state|time|percentage'");
    */
    return context;
}

bool contextExists() {
    return std::filesystem::exists(CONTEXT_FILE);
}

void generateContextFile(const std::string root) {
    std::ofstream out(CONTEXT_FILE);
    if (out.is_open()) {
        out << "FROM llama3.2\n";
        out << "PARAMETER temperature 1\n";
        out << "SYSTEM \"\"\" ";
        out << "You are a shell assistant. Based on the current system info below, please respond only with a safe, complete shell command to accomplish the user task.\n";
        out << getSystemContext(root);
        out << "\"\"\"";
        out.close();
    }
}

std::string readContextFile() {
    if(contextExists()){
        std::string result = runShellCommandWithSpinner("ollama create hippo-shell -f " + CONTEXT_FILE);
        if(!result.compare("Error running command")){
            return "Error generating agent";
        }
    }else{
        return "no model file ready, please try again.";
    }
    return "agent created successfully";
}

bool initAgent(){
    std::string agent_running = runShellCommand("ollama ps | grep hippo-shell");
    if(!agent_running.empty()){
         std::cout << "🟢 Agent hippo-shell is already running.\n";
    }else{
        std::cout << "🔄 Agent not running, starting now...\n";
        std::string startResult = runShellCommand("ollama run hippo-shell &");
        if (!startResult.compare("Error running command")) return false;
        std::this_thread::sleep_for(std::chrono::seconds(2)); // give it a moment to boot up
    }
    return true;
}
