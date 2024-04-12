#include <string>
#include <stdexcept>

struct HoroscopeAPIError : std::runtime_error {
    explicit HoroscopeAPIError(std::string details)
        : std::runtime_error(details), http_code(-1), details(std::move(details)) {
    }
    HoroscopeAPIError(int http_code, std::string details)
        : std::runtime_error{"api error: code=" + std::to_string(http_code) +
                             " details=" + details},
          http_code{http_code},
          details{std::move(details)} {
    }

    int http_code;
    std::string details;
};

class HoroscopeAPI {
public:
    explicit HoroscopeAPI(const std::string& api_endpoint);
    std::string GetDaily(std::string day, std::string sign);
    std::string GetMonthly(std::string sign);
    std::string GetWeekly(std::string sign);

private:
    std::string api_endpoint_;
};