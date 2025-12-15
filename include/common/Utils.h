#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <random>

namespace HMS {
namespace Utils {

// ==================== String Utilities ====================

/**
 * @brief Split a string by delimiter
 * @param str The input string
 * @param delimiter The character to split by
 * @return Vector of substrings
 */
std::vector<std::string> split(const std::string& str, char delimiter);

/**
 * @brief Join vector of strings with delimiter
 * @param parts The strings to join
 * @param delimiter The joining character
 * @return Joined string
 */
std::string join(const std::vector<std::string>& parts, char delimiter);

/**
 * @brief Trim whitespace from both ends of string
 * @param str The input string
 * @return Trimmed string
 */
std::string trim(const std::string& str);

/**
 * @brief Convert string to lowercase
 * @param str The input string
 * @return Lowercase string
 */
std::string toLower(const std::string& str);

/**
 * @brief Convert string to uppercase
 * @param str The input string
 * @return Uppercase string
 */
std::string toUpper(const std::string& str);

/**
 * @brief Check if string contains substring (case insensitive)
 * @param str The string to search in
 * @param substr The substring to find
 * @return True if found
 */
bool containsIgnoreCase(const std::string& str, const std::string& substr);

// ==================== Date/Time Utilities ====================

/**
 * @brief Get current date as string
 * @return Date string in YYYY-MM-DD format
 */
std::string getCurrentDate();

/**
 * @brief Get current time as string
 * @return Time string in HH:MM format
 */
std::string getCurrentTime();

/**
 * @brief Get current datetime as string
 * @return Datetime string in YYYY-MM-DD HH:MM format
 */
std::string getCurrentDateTime();

/**
 * @brief Validate date format (YYYY-MM-DD)
 * @param date The date string to validate
 * @return True if valid
 */
bool isValidDate(const std::string& date);

/**
 * @brief Validate time format (HH:MM)
 * @param time The time string to validate
 * @return True if valid
 */
bool isValidTime(const std::string& time);

/**
 * @brief Compare two dates
 * @param date1 First date
 * @param date2 Second date
 * @return -1 if date1 < date2, 0 if equal, 1 if date1 > date2
 */
int compareDates(const std::string& date1, const std::string& date2);

/**
 * @brief Check if date is in the future
 * @param date The date to check
 * @return True if date is in the future
 */
bool isFutureDate(const std::string& date);

// ==================== ID Generation ====================

/**
 * @brief Generate unique ID with prefix
 * @param prefix The prefix for the ID (e.g., "P", "D", "APT")
 * @return Unique ID string
 */
std::string generateID(const std::string& prefix);

/**
 * @brief Generate unique appointment ID
 * @return Appointment ID string
 */
std::string generateAppointmentID();

/**
 * @brief Generate unique patient ID
 * @return Patient ID string
 */
std::string generatePatientID();

/**
 * @brief Generate unique doctor ID
 * @return Doctor ID string
 */
std::string generateDoctorID();

// ==================== Password Utilities ====================

/**
 * @brief Hash password using simple hash (for educational purposes)
 * @param password Plain text password
 * @return Hashed password
 * @note In production, use bcrypt or similar
 */
std::string hashPassword(const std::string& password);

/**
 * @brief Verify password against hash
 * @param password Plain text password
 * @param hash Stored hash
 * @return True if password matches
 */
bool verifyPassword(const std::string& password, const std::string& hash);

// ==================== Validation Utilities ====================

/**
 * @brief Check if string is a valid phone number
 * @param phone The phone string
 * @return True if valid
 */
bool isValidPhone(const std::string& phone);

/**
 * @brief Check if string is a valid username
 * @param username The username to validate
 * @return True if valid
 */
bool isValidUsername(const std::string& username);

/**
 * @brief Check if string is a valid password
 * @param password The password to validate
 * @return True if valid
 */
bool isValidPassword(const std::string& password);

/**
 * @brief Check if string is numeric
 * @param str The string to check
 * @return True if numeric
 */
bool isNumeric(const std::string& str);

/**
 * @brief Check if string is a valid money amount
 * @param str The string to check
 * @return True if valid money format
 */
bool isValidMoney(const std::string& str);

// ==================== Formatting Utilities ====================

/**
 * @brief Format money value
 * @param amount The amount
 * @return Formatted string (e.g., "1,000,000 VND")
 */
std::string formatMoney(double amount);

/**
 * @brief Format date for display
 * @param date Date in YYYY-MM-DD format
 * @return Formatted date (e.g., "March 15, 2024")
 */
std::string formatDateDisplay(const std::string& date);

/**
 * @brief Pad string to fixed width
 * @param str The input string
 * @param width The target width
 * @param padChar The character to pad with
 * @param leftAlign Whether to left-align
 * @return Padded string
 */
std::string padString(const std::string& str, size_t width,
                      char padChar = ' ', bool leftAlign = true);

} // namespace Utils
} // namespace HMS
