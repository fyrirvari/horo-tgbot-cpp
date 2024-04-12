#pragma once

#include <memory>
#include <string>

class ITelegramBot {
public:
    virtual ~ITelegramBot() = default;
    virtual void Run() = 0;
};

std::unique_ptr<ITelegramBot> CreateTelegramBot(const std::string& api_endpoint,
                                                const std::string& state_path, int64_t timeout = 5);