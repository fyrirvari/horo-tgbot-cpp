#include "HoroscopeAPI.h"

#include <Poco/URI.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/JSON/Parser.h>

std::unique_ptr<Poco::Net::HTTPClientSession> CreateHoroscopeClientSession(const Poco::URI& uri) {
    if (uri.getScheme() == "http") {
        return std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
    } else if (uri.getScheme() == "https") {
        return std::make_unique<Poco::Net::HTTPSClientSession>(uri.getHost(), uri.getPort());
    } else {
        throw HoroscopeAPIError{"Invalid Scheme"};
    }
}

std::string ParseResponse(std::unique_ptr<Poco::Net::HTTPClientSession>& session) {
    Poco::Net::HTTPResponse response;
    auto& rs = session->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_OK) {
        throw HoroscopeAPIError{response.getStatus(), response.getReason()};
    }

    Poco::JSON::Parser parser;
    auto data = parser.parse(rs).extract<Poco::JSON::Object::Ptr>();

    if (!data->has("success")) {
        throw HoroscopeAPIError{"Wrong response format"};
    }
    if (!data->getValue<bool>("success")) {
        int error_code = data->has("status") ? data->getValue<int64_t>("status") : -1;
        std::string description = data->has("message") ? data->getValue<std::string>("message")
                                                       : "Response status is not ok";
        throw HoroscopeAPIError{error_code, description};
    }

    std::string text;

    try {
        auto object = data->get("data").extract<Poco::JSON::Object::Ptr>();
        text = object->getValue<std::string>("horoscope_data");
    } catch (std::exception ex) {
        throw HoroscopeAPIError{"Cann't parse response"};
    }

    return text;
}

std::string GetHoroscope(Poco::URI& uri, const std::string& sign) {
    uri.addQueryParameter("sign", sign);

    Poco::Net::HTTPRequest request{Poco::Net::HTTPRequest::HTTP_GET, uri.getPathAndQuery()};
    request.add("accept", "application/json");

    auto session = CreateHoroscopeClientSession(uri);
    session->sendRequest(request);

    return ParseResponse(session);
}

HoroscopeAPI::HoroscopeAPI(const std::string& api_endpoint) : api_endpoint_(api_endpoint) {
}

std::string HoroscopeAPI::GetDaily(std::string day, std::string sign) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/daily");
    uri.addQueryParameter("day", day);

    return GetHoroscope(uri, sign);
}

std::string HoroscopeAPI::GetMonthly(std::string sign) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/monthly");

    return GetHoroscope(uri, sign);
}

std::string HoroscopeAPI::GetWeekly(std::string sign) {
    Poco::URI uri{api_endpoint_};
    uri.setPath(uri.getPath() + "/weekly");

    return GetHoroscope(uri, sign);
}