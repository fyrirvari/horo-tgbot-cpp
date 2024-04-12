#include "TelegramBot.h"
#include "TelegramAPI.h"

#include <iostream>

int main(int argc, char* argv[]) {
    std::string host = "https://api.telegram.org";
    std::string path = "/bot";

    std::string token = argv[1];
    std::string state_file = "../state";

    auto bot = CreateTelegramBot(host + path + token, state_file);
    std::cout << "Run...\n";
    bot->Run();
    
    return 0;
}
