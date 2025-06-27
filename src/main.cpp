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
#include "context.h"

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

int main() {
    std::string input("start");
    std::cout << "Welcome to hippo-shell!\n";
    if(!contextExists()){
        std::cout << "Fine tunning agent, please wait a moment. This can take some minutes\n";
        generateContextFile("/home");
        std::cout << readContextFile() << "\n";
        std::cout << "context generated for /home if you want to fetch full system context please run [update context] and select root [/]\n";
    }
    bool ai_running = initAgent();
    while(input.compare("exit")){
        std::cout << "Ask me something (e.g. 'Create docker image named api'); or type exit to close the agent:\n";
        std::getline(std::cin, input);
        transform(input.begin(), input.end(), input.begin(), ::tolower);
        input = trim(input);
        if(!input.compare("exit")){
            break;
        }
        if(!input.compare("update context")){
            std::string contextFolder("/");
            std::cout << "Please select the folder to use for context default: [/] (this operation will override current context)\n";
            std::getline(std::cin, contextFolder);
            generateContextFile(contextFolder);
            std::cout << readContextFile() << "\n";
            continue;
        }
        auto response = sendToOllama(input);
        std::string cmd = parseCommand(response);
        if (askUserToConfirm(cmd)) {
            auto result = runCommand(cmd);
            std::cout << "Result:\n" << result << "\n";
        } else {
        std::cout << "Aborted.\n";
        }
        }
    std::cout << "Good bye!";
}