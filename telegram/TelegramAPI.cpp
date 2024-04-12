#include "TelegramAPI.h"

#include <iostream>
#include <sstream>

#include <Poco/URI.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/FilePartSource.h>

std::unique_ptr<Poco::Net::HTTPClientSession> CreateTelegramClientSession(const Poco::URI& uri) {
    if (uri.getScheme() == "http") {
        return std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
    } else if (uri.getScheme() == "https") {
        return std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
    } else {
        throw TelegramAPIError{"Invalid Scheme"};
    }
}

template <class Container>
Container Parse(const Poco::Dynamic::Var& var);

template <>
User Parse(const Poco::Dynamic::Var& var) {
    auto object = var.extract<Poco::JSON::Object::Ptr>();

    User user;
    user.id = object->getValue<int64_t>("id");
    user.is_bot = object->getValue<bool>("is_bot");

    if (object->has("first_name")) {
        user.first_name = object->getValue<std::string>("first_name");
    }
    if (object->has("last_name")) {
        user.last_name = object->getValue<std::string>("last_name");
    }
    if (object->has("username")) {
        user.username = object->getValue<std::string>("username");
    }
    if (object->has("language_code")) {
        user.language_code = object->getValue<std::string>("language_code");
    }

    return user;
}

template <>
Chat Parse(const Poco::Dynamic::Var& var) {
    auto object = var.extract<Poco::JSON::Object::Ptr>();

    Chat chat;
    chat.id = object->getValue<int64_t>("id");
    chat.type = object->getValue<std::string>("type");

    if (object->has("first_name")) {
        chat.first_name = object->getValue<std::string>("first_name");
    }
    if (object->has("last_name")) {
        chat.last_name = object->getValue<std::string>("last_name");
    }
    if (object->has("username")) {
        chat.username = object->getValue<std::string>("username");
    }

    return chat;
}

template <>
Message Parse(const Poco::Dynamic::Var& var) {
    auto object = var.extract<Poco::JSON::Object::Ptr>();

    Message message;
    message.id = object->getValue<int64_t>("message_id");
    message.date = object->getValue<int64_t>("date");
    message.chat = Parse<Chat>(object->get("chat"));

    if (object->has("from")) {
        message.from = Parse<User>(object->get("from"));
    }
    if (object->has("text")) {
        message.text = object->getValue<std::string>("text");
    }

    return message;
}

template <>
std::vector<Update> Parse(const Poco::Dynamic::Var& var) {
    auto array = var.extract<Poco::JSON::Array::Ptr>();

    std::vector<Update> updates;
    for (auto it = array->begin(); it != array->end(); ++it) {
        auto update_object = it->extract<Poco::JSON::Object::Ptr>();

        Update update;
        update.id = update_object->getValue<int64_t>("update_id");
        update.message = Parse<Message>(update_object->get("message"));

        updates.emplace_back(std::move(update));
    }

    return updates;
}

template <class Container>
Container ParseResponse(std::unique_ptr<Poco::Net::HTTPClientSession>& session) {
    Poco::Net::HTTPResponse response;
    auto& rs = session->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
        throw TelegramAPIError{response.getStatus(), response.getReason()};
    }

    Poco::JSON::Parser parser;
    auto data = parser.parse(rs).extract<Poco::JSON::Object::Ptr>();

    if (!data->has("ok")) {
        throw TelegramAPIError{"Wrong response format"};
    }
    if (!data->getValue<bool>("ok")) {
        int error_code = data->has("error_code") ? data->getValue<int64_t>("error_code") : -1;
        std::string description = data->has("description")
                                      ? data->getValue<std::string>("description")
                                      : "Response status is not ok";
        throw TelegramAPIError{error_code, description};
    }

    Container result;
    try {
        result = Parse<Container>(data->get("result"));
    } catch (std::exception ex) {
        throw TelegramAPIError{"Cann't parse response"};
    }

    return result;
}

TelegramAPI::TelegramAPI(const std::string& api_endpoint) : api_endpoint_(api_endpoint) {
}

User TelegramAPI::GetMe() {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/getMe");

    Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery()};

    auto session = CreateTelegramClientSession(uri);
    session->sendRequest(request);

    return ParseResponse<User>(session);
}

std::vector<Update> TelegramAPI::GetUpdates(std::optional<int64_t> offset,
                                            std::optional<int64_t> timeout) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/getUpdates");

    if (offset.has_value()) {
        uri.addQueryParameter("offset", std::to_string(offset.value()));
    }
    if (timeout.has_value()) {
        uri.addQueryParameter("timeout", std::to_string(timeout.value()));
    }

    Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery()};

    auto session = CreateTelegramClientSession(uri);
    session->sendRequest(request);

    return ParseResponse<std::vector<Update>>(session);
}

Message TelegramAPI::SendMessage(int64_t chat_id, std::optional<std::string> text,
                                 std::optional<int64_t> reply_to_message_id,
                                 std::optional<std::vector<std::string>> reply_markup) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/sendMessage");

    Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery()};
    request.add("Content-Type", "application/json");

    Poco::JSON::Object message_object;
    message_object.set("chat_id", std::to_string(chat_id));
    if (text.has_value()) {
        message_object.set("text", text.value());
    }

    if (reply_to_message_id.has_value()) {
        message_object.set("reply_to_message_id", std::to_string(reply_to_message_id.value()));
    }

    if (reply_markup.has_value()) {
        Poco::JSON::Parser parser;
        std::string buttons = "[";
        for (auto& button : reply_markup.value()) {
            buttons += "[\"" + button + "\"],";
        }
        buttons = buttons.substr(0, buttons.length() - 1) + "]";
        std::string json = "{\"keyboard\":" + buttons + "}";
        message_object.set("reply_markup", parser.parse(json));
    }

    std::stringstream message_body;
    message_object.stringify(message_body);
    request.setContentLength(message_body.str().size());

    auto session = CreateTelegramClientSession(uri);
    session->sendRequest(request) << message_body.str();

    return ParseResponse<Message>(session);
}

Message TelegramAPI::SendPhoto(int64_t chat_id, std::string file) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/sendPhoto");

    uri.addQueryParameter("chat_id", std::to_string(chat_id));
    // uri.addQueryParameter("caption", caption);

    Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery()};
    request.add("Content-Type", "multipart/form-data");

    auto session = CreateTelegramClientSession(uri);

    Poco::Net::HTMLForm form(Poco::Net::HTMLForm::ENCODING_MULTIPART);
    form.addPart("photo", new Poco::Net::FilePartSource(file));

    form.prepareSubmit(request);
    form.write(session->sendRequest(request));

    return ParseResponse<Message>(session);
}