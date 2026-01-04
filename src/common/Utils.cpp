#include "common/Utils.h"
#include <ctime>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace HMS
{
    namespace Utils
    {

        // ==================== String Utilities ====================

        std::vector<std::string> split(const std::string &str, char delimiter)
        {
            std::vector<std::string> result;
            std::stringstream ss(str);
            std::string item;

            while (std::getline(ss, item, delimiter))
            {
                result.push_back(item);
            }

            // Handle trailing delimiter (e.g., "a|b|" should return ["a", "b", ""])
            if (!str.empty() && str.back() == delimiter)
            {
                result.push_back("");
            }

            return result;
        }

        std::string join(const std::vector<std::string> &parts, char delimiter)
        {
            if (parts.empty())
                return "";

            std::string result = parts[0];
            for (size_t i = 1; i < parts.size(); ++i)
            {
                result += delimiter;
                result += parts[i];
            }
            return result;
        }

        std::string trim(const std::string &str)
        {
            size_t start = str.find_first_not_of(" \t\n\r");
            if (start == std::string::npos)
                return "";

            size_t end = str.find_last_not_of(" \t\n\r");
            return str.substr(start, end - start + 1);
        }

        std::string toLower(const std::string &str)
        {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c)
                           { return std::tolower(c); });
            return result;
        }

        std::string toUpper(const std::string &str)
        {
            std::string result = str;
            std::transform(result.begin(), result.end(), result.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });
            return result;
        }

        bool containsIgnoreCase(const std::string &str, const std::string &substr)
        {
            return toLower(str).find(toLower(substr)) != std::string::npos;
        }

        // ==================== Date/Time Utilities ====================

        std::string getCurrentDate()
        {
            std::time_t now = std::time(nullptr);
            std::tm *tm = std::localtime(&now);

            char buffer[11];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
            return std::string(buffer);
        }

        std::string getCurrentTime()
        {
            std::time_t now = std::time(nullptr);
            std::tm *tm = std::localtime(&now);

            char buffer[6];
            std::strftime(buffer, sizeof(buffer), "%H:%M", tm);
            return std::string(buffer);
        }

        std::string getCurrentDateTime()
        {
            return getCurrentDate() + " " + getCurrentTime();
        }

        // Validates DD-MM-YYYY format (user input format)
        bool isValidDate(const std::string &date)
        {
            if (date.length() != 10)
                return false; // DD-MM-YYYY
            if (date[2] != '-' || date[5] != '-')
                return false;

            for (int i = 0; i < 10; ++i)
            {
                if (i == 2 || i == 5)
                    continue;
                if (!std::isdigit(date[i]))
                    return false;
            }

            int day = std::stoi(date.substr(0, 2));
            int month = std::stoi(date.substr(3, 2));
            int year = std::stoi(date.substr(6, 4));

            if (year < 1900 || year > 2100)
                return false;
            if (month < 1 || month > 12)
                return false;

            int maxDays = getDaysInMonth(month, year);
            if (day < 1 || day > maxDays)
                return false;

            return true;
        }

        // Validates YYYY-MM-DD format (internal storage format)
        bool isValidDateInternal(const std::string &date)
        {
            if (date.length() != 10)
                return false; // YYYY-MM-DD
            if (date[4] != '-' || date[7] != '-')
                return false;

            for (int i = 0; i < 10; ++i)
            {
                if (i == 4 || i == 7)
                    continue;
                if (!std::isdigit(date[i]))
                    return false;
            }

            int year = std::stoi(date.substr(0, 4));
            int month = std::stoi(date.substr(5, 2));
            int day = std::stoi(date.substr(8, 2));

            if (year < 1900 || year > 2100)
                return false;
            if (month < 1 || month > 12)
                return false;
            if (day < 1 || day > 31)
                return false;

            return true;
        }

        // Convert DD-MM-YYYY to YYYY-MM-DD
        std::string dateFromInput(const std::string &date)
        {
            if (!isValidDate(date))
                return "";

            std::string day = date.substr(0, 2);
            std::string month = date.substr(3, 2);
            std::string year = date.substr(6, 4);

            return year + "-" + month + "-" + day;
        }

        // Convert YYYY-MM-DD to DD-MM-YYYY
        std::string dateToDisplay(const std::string &date)
        {
            if (date.length() != 10 || date[4] != '-' || date[7] != '-')
                return date; // Return as-is if not internal format

            std::string year = date.substr(0, 4);
            std::string month = date.substr(5, 2);
            std::string day = date.substr(8, 2);

            return day + "-" + month + "-" + year;
        }

        bool isValidTime(const std::string &time)
        {
            if (time.length() != 5)
                return false; // HH:MM
            if (time[2] != ':')
                return false;

            for (int i = 0; i < 5; ++i)
            {
                if (i == 2)
                    continue;
                if (!std::isdigit(time[i]))
                    return false;
            }

            int hour = std::stoi(time.substr(0, 2));
            int minute = std::stoi(time.substr(3, 2));

            if (hour < 0 || hour > 23)
                return false;
            if (minute < 0 || minute > 59)
                return false;

            return true;
        }

        int compareDates(const std::string &date1, const std::string &date2)
        {
            if (date1 < date2)
                return -1;
            if (date1 > date2)
                return 1;
            return 0;
        }

        int daysBetweenDates(const std::string &futureDate, const std::string &currentDate)
        {
            if (!isValidDateInternal(futureDate) || !isValidDateInternal(currentDate))
            {
                return 0;
            }

            int year1 = std::stoi(futureDate.substr(0, 4));
            int month1 = std::stoi(futureDate.substr(5, 2));
            int day1 = std::stoi(futureDate.substr(8, 2));

            int year2 = std::stoi(currentDate.substr(0, 4));
            int month2 = std::stoi(currentDate.substr(5, 2));
            int day2 = std::stoi(currentDate.substr(8, 2));

            std::tm tm1 = {};
            tm1.tm_year = year1 - 1900;
            tm1.tm_mon = month1 - 1;
            tm1.tm_mday = day1;

            std::tm tm2 = {};
            tm2.tm_year = year2 - 1900;
            tm2.tm_mon = month2 - 1;
            tm2.tm_mday = day2;

            std::time_t time1 = std::mktime(&tm1);
            std::time_t time2 = std::mktime(&tm2);

            double seconds = std::difftime(time1, time2);
            return static_cast<int>(seconds / 86400);
        }

        bool isFutureDate(const std::string &date)
        {
            return compareDates(date, getCurrentDate()) > 0;
        }

        bool isTodayOrFuture(const std::string &date)
        {
            return compareDates(date, getCurrentDate()) >= 0;
        }

        int getDaysInMonth(int month, int year)
        {
            if (month < 1 || month > 12)
            {
                return 0;
            }

            if (month == 2)
            {
                bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                return isLeap ? 29 : 28;
            }

            if (month == 4 || month == 6 || month == 9 || month == 11)
            {
                return 30;
            }

            return 31;
        }

        bool getWeekRange(const std::string &date, std::string &startDate, std::string &endDate)
        {
            // This function expects internal format (YYYY-MM-DD)
            if (!isValidDateInternal(date))
            {
                return false;
            }

            int year = std::stoi(date.substr(0, 4));
            int month = std::stoi(date.substr(5, 2));
            int day = std::stoi(date.substr(8, 2));

            std::tm tm = {};
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            std::mktime(&tm);

            // Calculate days from Monday (Monday = 0, Sunday = 6)
            int daysFromMonday = (tm.tm_wday == 0) ? 6 : tm.tm_wday - 1;

            // Go to Monday
            tm.tm_mday -= daysFromMonday;
            std::mktime(&tm);

            char buffer[11];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
            startDate = buffer;

            // Go to Sunday (add 6 days)
            tm.tm_mday += 6;
            std::mktime(&tm);

            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
            endDate = buffer;

            return true;
        }

        // ==================== ID Generation ====================

        std::string generateID(const std::string &prefix)
        {
            static int counter = 0;
            std::time_t now = std::time(nullptr);

            std::stringstream ss;
            ss << prefix << now << std::setfill('0') << std::setw(4) << (++counter % 10000);
            return ss.str();
        }

        std::string generateAppointmentID()
        {
            return generateID("APT");
        }

        std::string generatePatientID()
        {
            return generateID("P");
        }

        std::string generateDoctorID()
        {
            return generateID("D");
        }

        // ==================== Password Utilities ====================

        std::string hashPassword(const std::string &password)
        {
            // Simple hash for educational purposes
            // In production, use bcrypt or similar
            return "HASH_" + password;
        }

        bool verifyPassword(const std::string &password, const std::string &hash)
        {
            return hashPassword(password) == hash;
        }

        // ==================== Validation Utilities ====================

        bool isValidPhone(const std::string &phone)
        {
            if (phone.length() != 10)
                return false;

            for (char c : phone)
            {
                if (!std::isdigit(c))
                    return false;
            }

            return phone[0] == '0';
        }

        bool isValidUsername(const std::string &username)
        {
            if (username.length() < 3 || username.length() > 50)
                return false;

            for (char c : username)
            {
                // Explicitly reject pipe character (field delimiter) to prevent data corruption
                if (c == '|')
                    return false;
                // Only allow alphanumeric, underscore, and dot
                if (!std::isalnum(c) && c != '_' && c != '.')
                    return false;
            }

            return true;
        }

        bool isValidPassword(const std::string &password)
        {
            if (password.length() < 6 || password.length() > 100)
            {
                return false;
            }

            // Reject pipe character (field delimiter) to prevent data corruption
            for (char c : password)
            {
                if (c == '|')
                {
                    return false;
                }
            }

            return true;
        }

        bool isNumeric(const std::string &str)
        {
            if (str.empty())
                return false;

            for (char c : str)
            {
                if (!std::isdigit(c))
                    return false;
            }

            return true;
        }

        bool isValidMoney(const std::string &str)
        {
            if (str.empty())
                return false;

            bool hasDot = false;
            for (size_t i = 0; i < str.length(); ++i)
            {
                if (str[i] == '.')
                {
                    if (hasDot)
                        return false;
                    hasDot = true;
                }
                else if (!std::isdigit(str[i]))
                {
                    return false;
                }
            }

            return true;
        }

        // ==================== Formatting Utilities ====================

        std::string formatMoney(double amount)
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << amount << " VND";
            return ss.str();
        }

        std::string formatDateDisplay(const std::string &date)
        {
            if (!isValidDate(date))
                return date;

            static const char *months[] = {
                "January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October", "November", "December"};

            int month = std::stoi(date.substr(5, 2));
            int day = std::stoi(date.substr(8, 2));
            std::string year = date.substr(0, 4);

            std::stringstream ss;
            ss << months[month - 1] << " " << day << ", " << year;
            return ss.str();
        }

        std::string padString(const std::string &str, size_t width,
                              char padChar, bool leftAlign)
        {
            if (str.length() >= width)
                return str;

            size_t padding = width - str.length();

            if (leftAlign)
            {
                return str + std::string(padding, padChar);
            }
            else
            {
                return std::string(padding, padChar) + str;
            }
        }

    } // namespace Utils
} // namespace HMS
