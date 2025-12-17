#include "common/Utils.h"
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace HMS {
namespace Utils {

// ==================== String Utilities ====================

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    // Handle trailing delimiter (e.g., "a|b|" should return ["a", "b", ""])
    if (!str.empty() && str.back() == delimiter) {
        result.push_back("");
    }

    return result;
}

std::string join(const std::vector<std::string>& parts, char delimiter) {
    if (parts.empty()) return "";

    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += delimiter;
        result += parts[i];
    }
    return result;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

bool containsIgnoreCase(const std::string& str, const std::string& substr) {
    return toLower(str).find(toLower(substr)) != std::string::npos;
}

// ==================== Date/Time Utilities ====================

std::string getCurrentDate() {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    char buffer[11];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
    return std::string(buffer);
}

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    char buffer[6];
    std::strftime(buffer, sizeof(buffer), "%H:%M", tm);
    return std::string(buffer);
}

std::string getCurrentDateTime() {
    return getCurrentDate() + " " + getCurrentTime();
}

bool isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(date[i])) return false;
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;

    return true;
}

bool isValidTime(const std::string& time) {
    if (time.length() != 5) return false;
    if (time[2] != ':') return false;

    for (int i = 0; i < 5; ++i) {
        if (i == 2) continue;
        if (!std::isdigit(time[i])) return false;
    }

    int hour = std::stoi(time.substr(0, 2));
    int minute = std::stoi(time.substr(3, 2));

    if (hour < 0 || hour > 23) return false;
    if (minute < 0 || minute > 59) return false;

    return true;
}

int compareDates(const std::string& date1, const std::string& date2) {
    if (date1 < date2) return -1;
    if (date1 > date2) return 1;
    return 0;
}

bool isFutureDate(const std::string& date) {
    return compareDates(date, getCurrentDate()) > 0;
}

// ==================== ID Generation ====================

std::string generateID(const std::string& prefix) {
    static int counter = 0;
    std::time_t now = std::time(nullptr);

    std::stringstream ss;
    ss << prefix << now << std::setfill('0') << std::setw(4) << (++counter % 10000);
    return ss.str();
}

std::string generateAppointmentID() {
    return generateID("APT");
}

std::string generatePatientID() {
    return generateID("P");
}

std::string generateDoctorID() {
    return generateID("D");
}

// ==================== Password Utilities ====================

std::string hashPassword(const std::string& password) {
    // Simple hash for educational purposes
    // In production, use bcrypt or similar
    return "HASH_" + password;
}

bool verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

// ==================== Validation Utilities ====================

bool isValidPhone(const std::string& phone) {
    if (phone.length() != 10) return false;

    for (char c : phone) {
        if (!std::isdigit(c)) return false;
    }

    return phone[0] == '0';
}

bool isValidUsername(const std::string& username) {
    if (username.length() < 3 || username.length() > 50) return false;

    for (char c : username) {
        if (!std::isalnum(c) && c != '_' && c != '.') return false;
    }

    return true;
}

bool isValidPassword(const std::string& password) {
    if (password.length() < 6 || password.length() > 100) {
        return false;
    }

    // Reject pipe character (field delimiter) to prevent data corruption
    for (char c : password) {
        if (c == '|') {
            return false;
        }
    }

    return true;
}

bool isNumeric(const std::string& str) {
    if (str.empty()) return false;

    for (char c : str) {
        if (!std::isdigit(c)) return false;
    }

    return true;
}

bool isValidMoney(const std::string& str) {
    if (str.empty()) return false;

    bool hasDot = false;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

// ==================== Formatting Utilities ====================

std::string formatMoney(double amount) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << amount << " VND";
    return ss.str();
}

std::string formatDateDisplay(const std::string& date) {
    if (!isValidDate(date)) return date;

    static const char* months[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));
    std::string year = date.substr(0, 4);

    std::stringstream ss;
    ss << months[month - 1] << " " << day << ", " << year;
    return ss.str();
}

std::string padString(const std::string& str, size_t width,
                      char padChar, bool leftAlign) {
    if (str.length() >= width) return str;

    size_t padding = width - str.length();

    if (leftAlign) {
        return str + std::string(padding, padChar);
    } else {
        return std::string(padding, padChar) + str;
    }
}

} // namespace Utils
} // namespace HMS
