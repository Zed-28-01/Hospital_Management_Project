#pragma once

#include <string>
#include <vector>
#include <regex>

namespace HMS {
namespace UI {

/**
 * @class InputValidator
 * @brief Utility class for validating user input
 *
 * Provides static methods for validating various types of user input
 * including usernames, passwords, dates, phone numbers, etc.
 */
class InputValidator {
public:
    // ==================== Username Validation ====================

    /**
     * @brief Validate username format
     * @param username The username to validate
     * @return True if valid (3-50 chars, alphanumeric + underscore)
     */
    static bool validateUsername(const std::string& username);

    /**
     * @brief Get username validation error message
     * @param username The invalid username
     * @return Error message describing the issue
     */
    static std::string getUsernameError(const std::string& username);

    // ==================== Password Validation ====================

    /**
     * @brief Validate password strength
     * @param password The password to validate
     * @return True if valid (6-100 chars, meets complexity requirements)
     */
    static bool validatePassword(const std::string& password);

    /**
     * @brief Get password validation error message
     * @param password The invalid password
     * @return Error message describing the issue
     */
    static std::string getPasswordError(const std::string& password);

    /**
     * @brief Check password complexity
     * @param password The password to check
     * @return Complexity score (0-4)
     */
    static int getPasswordStrength(const std::string& password);

    // ==================== Phone Validation ====================

    /**
     * @brief Validate phone number format
     * @param phone The phone number to validate
     * @return True if valid (10 digits, starts with 0)
     */
    static bool validatePhone(const std::string& phone);

    /**
     * @brief Get phone validation error message
     * @param phone The invalid phone
     * @return Error message describing the issue
     */
    static std::string getPhoneError(const std::string& phone);

    // ==================== Date Validation ====================

    /**
     * @brief Validate date format
     * @param date The date to validate
     * @return True if valid YYYY-MM-DD format and valid date
     */
    static bool validateDate(const std::string& date);

    /**
     * @brief Validate date is not in the past
     * @param date The date to validate
     * @return True if date is today or future
     */
    static bool validateFutureDate(const std::string& date);

    /**
     * @brief Validate date is not in the future
     * @param date The date to validate
     * @return True if date is today or past
     */
    static bool validatePastDate(const std::string& date);

    /**
     * @brief Get date validation error message
     * @param date The invalid date
     * @return Error message describing the issue
     */
    static std::string getDateError(const std::string& date);

    // ==================== Time Validation ====================

    /**
     * @brief Validate time format
     * @param time The time to validate
     * @return True if valid HH:MM format (24-hour)
     */
    static bool validateTime(const std::string& time);

    /**
     * @brief Validate time is within working hours
     * @param time The time to validate
     * @return True if within 08:00-17:00
     */
    static bool validateWorkingHours(const std::string& time);

    /**
     * @brief Get time validation error message
     * @param time The invalid time
     * @return Error message describing the issue
     */
    static std::string getTimeError(const std::string& time);

    // ==================== Name Validation ====================

    /**
     * @brief Validate name format
     * @param name The name to validate
     * @return True if valid (2-100 chars, letters and spaces)
     */
    static bool validateName(const std::string& name);

    /**
     * @brief Get name validation error message
     * @param name The invalid name
     * @return Error message describing the issue
     */
    static std::string getNameError(const std::string& name);

    // ==================== Gender Validation ====================

    /**
     * @brief Validate gender input
     * @param gender The gender string to validate
     * @return True if valid (Male/Female/Other, case insensitive)
     */
    static bool validateGender(const std::string& gender);

    /**
     * @brief Normalize gender input
     * @param gender The gender string to normalize
     * @return Normalized gender (Male/Female/Other)
     */
    static std::string normalizeGender(const std::string& gender);

    // ==================== Money Validation ====================

    /**
     * @brief Validate money amount format
     * @param amount The amount string to validate
     * @return True if valid positive number
     */
    static bool validateMoney(const std::string& amount);

    /**
     * @brief Parse money string to double
     * @param amount The amount string
     * @return Parsed amount or -1 if invalid
     */
    static double parseMoney(const std::string& amount);

    // ==================== ID Validation ====================

    /**
     * @brief Validate patient ID format
     * @param id The ID to validate
     * @return True if valid (P followed by digits)
     */
    static bool validatePatientID(const std::string& id);

    /**
     * @brief Validate doctor ID format
     * @param id The ID to validate
     * @return True if valid (D followed by digits)
     */
    static bool validateDoctorID(const std::string& id);

    /**
     * @brief Validate appointment ID format
     * @param id The ID to validate
     * @return True if valid (APT followed by digits)
     */
    static bool validateAppointmentID(const std::string& id);

    // ==================== Menu Choice Validation ====================

    /**
     * @brief Validate menu choice
     * @param choice The choice string
     * @param min Minimum valid value
     * @param max Maximum valid value
     * @return True if valid integer in range
     */
    static bool validateMenuChoice(const std::string& choice, int min, int max);

    /**
     * @brief Parse menu choice to integer
     * @param choice The choice string
     * @return Parsed choice or -1 if invalid
     */
    static int parseMenuChoice(const std::string& choice);

    // ==================== General Validation ====================

    /**
     * @brief Check if string is empty or whitespace only
     * @param str The string to check
     * @return True if empty or whitespace
     */
    static bool isEmpty(const std::string& str);

    /**
     * @brief Check if string contains only digits
     * @param str The string to check
     * @return True if all digits
     */
    static bool isNumeric(const std::string& str);

    /**
     * @brief Check if string contains only letters and spaces
     * @param str The string to check
     * @return True if alphabetic
     */
    static bool isAlphabetic(const std::string& str);

    /**
     * @brief Sanitize input string
     * @param input The input to sanitize
     * @return Sanitized string (trimmed, special chars removed)
     */
    static std::string sanitizeInput(const std::string& input);

    /**
     * @brief Confirm yes/no input
     * @param input The input to check
     * @return True if 'y' or 'yes' (case insensitive)
     */
    static bool isYesConfirmation(const std::string& input);

private:
    /**
     * @brief Private constructor to prevent instantiation
     */
    InputValidator() = default;
};

} // namespace UI
} // namespace HMS
