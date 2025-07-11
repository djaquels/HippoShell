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
//--------- Enriched line input experience with GNU Readline
#include <readline/readline.h>
#include <readline/history.h>
//--------- RAG libraries
#include "rag/context_collector.h"


const std::string CONTEXT_FILE = "/tmp/jeanne_context.txt";
const std::string RAG_CONTEXT_FILE = "/tmp/jeanne_context_rag.txt";

std::string  getInput(const char* prompt){
 char* line = readline(prompt);
 if (line && *line) {
	add_history(line);
	std::string input(line);
	free(line);
	return input;
    }
    free(line);
    return "";
}

inline std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

int main() {
    std::string input("start");
    std::cout << "Welcome to hippo-shell!\n";
    if(!contextExists(CONTEXT_FILE)){
        std::cout << "Fine tunning agent, please wait a moment. This can take some minutes\n";
        generateContextFile("/home");
        readContextFile();
        std::cout << "context generated for /home if you want to fetch full system context please run [update context] and select root [/]\n";
    }
    bool ai_running = initAgent();
    while(input.compare("exit")){
        std::cout << "Ask me something (e.g. 'Create docker image named api'); or type exit to close the agent:\n";
        input = getInput("hippo-shell> ");
	transform(input.begin(), input.end(), input.begin(), ::tolower);
        input = trim(input);
        if(!input.compare("exit")){
            break;
        }
        if(!input.compare("clean")){
          
        }
        if(!input.compare("update context")){
            std::string contextFolder("/");
            std::cout << "Please select the folder to use for context default: [/] (this operation will override current context)\n";
            contextFolder = getInput("hippo-shell> ");
	        collectContext("/tmp/jeanne_context_rag.txt",contextFolder);
            std::cout << "new context loaded!\n";
            continue;
        }
        auto response = sendToOllama(input);
        std::string cmd = parseCommand(response);
	std::string finalCommand = askUserToConfirm(cmd);
        if (!finalCommand.empty()) {
            auto result = runCommand(finalCommand);
            std::cout << "Result:\n" << result << "\n";
        } else {
        std::cout << "Aborted.\n";
        }
        }
    std::cout << "Good bye!";
}
