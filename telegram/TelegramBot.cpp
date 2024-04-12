#include "TelegramBot.h"
#include "TelegramAPI.h"
#include "HoroscopeAPI.h"

#include <iostream>

#include <random>
#include <fstream>

void RuntimeLog(const std::string& username, const std::string& text) {
    std::cout << "@" + username + " - " + text + "\n";
}

bool In(const std::vector<std::string>& array, const std::string& val) {
    for (auto& element : array) {
        if (element == val) {
            return true;
        }
    }
    return false;
}

class HoroTelegramBot : public ITelegramBot {
public:
    HoroTelegramBot(const std::string& api_endpoint, const std::string& state_file_path,
                      std::optional<int64_t> timeout = std::nullopt)
        : tgapi_{api_endpoint},
          horoapi_{"https://horoscope-app-api.vercel.app/api/v1/get-horoscope"},
          state_file_path_{state_file_path},
          offset_{std::nullopt},
          timeout_{timeout} {
        std::fstream file{state_file_path_};
        if (!file.is_open()) {
            throw std::runtime_error{"State file cann't be opened: " + state_file_path_};
        }
        int64_t last_offset;
        if (file >> last_offset) {
            offset_ = last_offset;
        }
        file.close();
    }

    void Run() override {
        std::string description =
            "У меня мало возможностей, но выполняю их очень хорошо (Как говорил Брюс Ли: \"Я не "
            "боюсь "
            "того, кто изучает 10 000 различных ударов. Я боюсь того, кто изучает один удар 10 000 "
            "раз\").\n\n"
            "Я умею давать гороскоп следующими командами:\n1. /yesterday\n2. /today\n3. "
            "/tomorrow\n4. /month\n\n Также умею давать "
            "расклад таро:\n1. /taro";

        std::vector<std::string> timeframes = {"/today", "/yesterday", "/tomorrow", "/week",
                                               "/month"};

        std::vector<std::string> signs = {
            "♉ Телец ♉",    "♊ Близнецы ♊", "♋ Рак ♋",     "♌ Лев ♌",     "♍ Дева ♍", "♎ Весы ♎",
            "♏ Скорпион ♏", "♐ Стрелец ♐",  "♑ Козерог ♑", "♒ Водолей ♒", "♓ Рыбы ♓"};

        std::string horoscope = "TODAY";

        bool is_stoped = false;

        while (!is_stoped) {
            auto updates = tgapi_.GetUpdates(offset_, timeout_);
            for (const auto& update : updates) {
                offset_ = update.id + 1;
                WriteState();

                if (!update.message.text.has_value()) {
                    continue;
                }

                auto& message_text = update.message.text.value();
                auto& message_id = update.message.id;
                auto& chat_id = update.message.chat.id;
                auto& username = update.message.chat.username.value();

                RuntimeLog(username, message_text);

                std::string text;

                if (message_text == "/start") {
                    text = "Добрый день, @" + username + "! Лучший таролог на месте.\n\n" +
                           description;
                } else if (message_text == "/stop") {
                    is_stoped = true;
                    text = "Пока 😭";
                } else if (message_text == "/help") {
                    text = description;
                } else if (message_text == "/taro") {
                    tgapi_.SendPhoto(chat_id, "./../images/XVII.jpg");
                    tgapi_.SendPhoto(chat_id, "./../images/V.jpg");
                    tgapi_.SendPhoto(chat_id, "./../images/XIV.jpg");
                    text = "Карта № 1 — прошлое, карта № 2 — настоящее, карта № 3 — будущее;";
                } else if (In(timeframes, message_text)) {
                    if (message_text == "/yesterday") {
                        horoscope = "YESTERDAY";
                    } else if (message_text == "/today") {
                        horoscope = "TODAY";
                    } else if (message_text == "/tomorrow") {
                        horoscope = "TOMORROW";
                    } else if (message_text == "/week") {
                        horoscope = "WEEK";
                    } else if (message_text == "/month") {
                        horoscope = "MONTH";
                    }
                    text = "Выбери знак";
                    tgapi_.SendMessage(chat_id, text, message_id, signs);
                    continue;
                } else if (In(signs, message_text)) {
                    std::string sign;
                    if (message_text == "♈ Овен ♈'") {
                        sign = "aries";
                    } else if (message_text == "♉ Телец ♉") {
                        sign = "taurus";
                    } else if (message_text == "♊ Близнецы ♊") {
                        sign = "gemini";
                    } else if (message_text == "♋ Рак ♋") {
                        sign = "cancer";
                    } else if (message_text == "♌ Лев ♌") {
                        sign = "leo";
                    } else if (message_text == "♍ Дева ♍") {
                        sign = "virgo";
                    } else if (message_text == "♎ Весы ♎") {
                        sign = "libra";
                    } else if (message_text == "♏ Скорпион ♏") {
                        sign = "scorpio";
                    } else if (message_text == "♐ Стрелец ♐") {
                        sign = "sagittarius";
                    } else if (message_text == "♑ Козерог ♑") {
                        sign = "capricorn";
                    } else if (message_text == "♒ Водолей ♒") {
                        sign = "aquarius";
                    } else if (message_text == "♓ Рыбы ♓") {
                        sign = "pisces";
                    }

                    text = horoscope + "\n";

                    if (horoscope == "WEEK") {
                        text += horoapi_.GetWeekly(sign);
                    } else if (horoscope == "MONTH") {
                        text += horoapi_.GetMonthly(sign);
                    } else {
                        text += horoapi_.GetDaily(horoscope, sign);
                    }
                } else {
                    text = "Я ем команды только из /help...";
                }

                try {
                    tgapi_.SendMessage(chat_id, text, message_id);
                } catch (TelegramAPIError er) {
                    std::cerr << er.what() << "\n";
                }
            }
        }
    }

private:
    void WriteState() {
        if (offset_.has_value()) {
            std::fstream file{state_file_path_};
            if (!file.is_open()) {
                throw std::runtime_error{"State file cann't be opened: " + state_file_path_};
            }
            file << offset_.value();
        }
    }

private:
    TelegramAPI tgapi_;
    HoroscopeAPI horoapi_;

    std::string state_file_path_;

    std::optional<int64_t> offset_;
    std::optional<int64_t> timeout_;
};

std::unique_ptr<ITelegramBot> CreateTelegramBot(const std::string& api_endpoint,
                                                const std::string& state_file_path,
                                                int64_t timeout) {
    return std::make_unique<HoroTelegramBot>(api_endpoint, state_file_path, timeout);
}