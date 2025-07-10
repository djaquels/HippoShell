#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include "context.h" // Your runShellCommand()

std::mutex writeMutex; // For thread-safe writing

struct ContextBlock {
    std::string label;
    std::string command;
    std::string result;
};

std::string trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ') + 1);  // Suffixing spaces
    str.erase(0, str.find_first_not_of(' '));  // Prefixing spaces
    return str;
}

std::string detectOS() {
    std::string osInfo = runShellCommand("uname -s");
    if (osInfo.find("Linux") != std::string::npos) {
        std::string distro = runShellCommand("cat /etc/os-release | grep '^ID=' | cut -d'=' -f2");
        // Trim newline characters from the string
        std::string distro_name = trim(distro);
        return distro_name;
    } else if (osInfo.find("FreeBSD") != std::string::npos) {
        return "freebsd";
    } else if (osInfo.find("Darwin") != std::string::npos) {
        return "macos";
    } else {
        return "unknown";
    }
}

std::string getInstalledPackagesCommand(const std::string& distro) {
    if (distro == "ubuntu" || distro == "debian") {
        return "dpkg -l | grep '^ii' | head -n 50";
    } else if (distro == "fedora" || distro == "redhat") {
        return "dnf list installed | head -n 50";
    } else if (distro == "opensuse" || distro == "sles") {
        return "zypper se --installed-only | head -n 50";
    } else if (distro == "arch") {
        return "pacman -Q | head -n 50";
    } else if (distro == "freebsd") {
        return "pkg info | head -n 50";
    } else {
        return "echo '❌ Unknown package manager'";
    }
}

void runBlock(ContextBlock& block) {
    std::string output = runShellCommand(block.command);
    block.result = output;
}

void collectContext(const std::string& outputPath = "/tmp/jeanne_context_rag.txt") {
    std::string distro = detectOS();

    std::vector<ContextBlock> blocks = {
        {"[OS Info]", "uname -a && lsb_release -a || cat /etc/os-release"},
        {"[CPU Info]", "lscpu"},
        {"[RAM Info]", "free -h"},
        {"[Disk Usage]", "df -h"},
        {"[Installed Packages]", getInstalledPackagesCommand(distro)},
        {"[Installed Tools]", "which docker git terraform make gcc g++"},
        {"[Tool Versions]", "docker --version && git --version && terraform version"},
        {"[Running Services]", "systemctl list-units --type=service --state=running | head -n 20"},
        {"[Processes]", "ps aux --sort=-%cpu | head -n 10"},
        {"[Network Info]", "hostname -I && ip a | grep inet && ss -tuln"},
        {"[Directory Tree]", "tree -L 2 ~ 2>/dev/null"}
    };

    std::vector<std::thread> threads;

    // Spawn threads for each block
    for (auto& block : blocks) {
        threads.emplace_back(runBlock, std::ref(block));
    }

    // Join threads
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // Write results to file
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "❌ Failed to open " << outputPath << " for writing." << std::endl;
        return;
    }

    for (const auto& block : blocks) {
        out << block.label << "\n";
        out << block.result << "\n\n";
    }

    out.close();
    std::cout << "✅ Context collected (multi-threaded) at: " << outputPath << std::endl;
}
