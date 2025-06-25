// ollama_client.cpp
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
#include <sstream>
#include <iostream>



std::string extractResponseField(const std::string& jsonString) {
    try {
        auto json = nlohmann::json::parse(jsonString);
        if (json.contains("response") && json["response"].is_string()) {
            return json["response"];
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing failed: " << e.what() << std::endl;
    }
    return "";
}

std::string sendToOllama(const std::string& prompt) {
    CURL* curl = curl_easy_init();
    std::string response;
    std::string enrichedPrompt = 
        "Please respond only with the executable command string and all operators that can be run on shell from the following: " 
        + prompt + "( and please remove \\n\\r new line and ```bash headers)";
    if (curl) {
        std::string postData = R"({"model": "llama3.2", "stream": false, "prompt":")" + enrichedPrompt + R"("})";
        struct curl_slist* headers = nullptr;
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());        
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
            +[](char* ptr, size_t size, size_t nmemb, std::string* data) -> size_t {
                data->append(ptr, size * nmemb);
                return size * nmemb;
            });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    return extractResponseField(response);
}