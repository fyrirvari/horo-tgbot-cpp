#pragma once

#include <string>
#include <vector>

#include <memory>
#include <optional>
#include <stdexcept>

struct User {
    int64_t id;
    bool is_bot;

    std::optional<std::string> first_name;
    std::optional<std::string> last_name;
    std::optional<std::string> username;
    std::optional<std::string> language_code;
};

struct Chat {
    int64_t id;
    std::string type;

    std::optional<std::string> first_name;
    std::optional<std::string> last_name;
    std::optional<std::string> username;
};

struct Message {
    int64_t id;
    int64_t date;

    Chat chat;

    std::optional<User> from;
    std::optional<std::string> text;
};

struct Update {
    int64_t id;
    Message message;
};

struct TelegramAPIError : std::runtime_error {
    explicit TelegramAPIError(std::string details)
        : std::runtime_error(details), http_code(-1), details(std::move(details)) {
    }
    TelegramAPIError(int http_code, std::string details)
        : std::runtime_error{"api error: code=" + std::to_string(http_code) +
                             " details=" + details},
          http_code{http_code},
          details{std::move(details)} {
    }

    int http_code;
    std::string details;
};

class TelegramAPI {
public:
    explicit TelegramAPI(const std::string& api_endpoint);

    User GetMe();
    std::vector<Update> GetUpdates(std::optional<int64_t> offset = std::nullopt,
                                   std::optional<int64_t> timeout = std::nullopt);
    Message SendMessage(int64_t chat_id, std::optional<std::string> text = std::nullopt,
                        std::optional<int64_t> reply_to_message_id = std::nullopt,
                        std::optional<std::vector<std::string>> reply_markup = std::nullopt);
    Message SendPhoto(int64_t chat_id, std::string file);

private:
    std::string api_endpoint_;
};
