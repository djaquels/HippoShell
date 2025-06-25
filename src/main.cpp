#include <iostream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <bits/stdc++.h>
#include <cctype>
#include <string>
#include <algorithm>
//--------- App related imports
#include "ollama_client.h"
#include "command_runner.h"

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

int main() {
    std::string input("start");
    std::cout << "Welcome to hippo-shell!\n";
    while(input.compare("exit")){
        std::cout << "Ask me something (e.g. 'Create docker image named api'); or type exit to close the agent:\n";
        std::getline(std::cin, input);
        transform(input.begin(), input.end(), input.begin(), ::tolower);
        input = trim(input);
        auto response = sendToOllama(input);
        std::string cmd = parseCommand(response);
        if (askUserToConfirm(cmd)) {
            auto output = runCommand(cmd);
            std::cout << "Output:\n" << output << "\n";
        } else {
        std::cout << "Aborted.\n";
        }
        }
    std::cout << "Good bye!";
}