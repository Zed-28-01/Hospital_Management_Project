#include "ui/InputValidator.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <cctype>

namespace HMS {
namespace UI {

// ==================== Username Validation ====================

bool InputValidator::validateUsername(const std::string& username) {
    return Utils::isValidUsername(username);
}

std::string InputValidator::getUsernameError(const std::string& username) {
    if (username.empty()) {
        return "Username cannot be empty.";
    }
    if (username.length() < Constants::MIN_USERNAME_LENGTH) {
        return "Username must be at least " +
               std::to_string(Constants::MIN_USERNAME_LENGTH) + " characters.";
    }
    if (username.length() > Constants::MAX_USERNAME_LENGTH) {
        return "Username must be at most " +
               std::to_string(Constants::MAX_USERNAME_LENGTH) + " characters.";
    }
    return "Username can only contain letters, numbers, underscores, and dots.";
}

// ==================== Password Validation ====================

bool InputValidator::validatePassword(const std::string& password) {
    return Utils::isValidPassword(password);
}

std::string InputValidator::getPasswordError(const std::string& password) {
    if (password.empty()) {
        return "Password cannot be empty.";
    }
    if (password.length() < Constants::MIN_PASSWORD_LENGTH) {
        return "Password must be at least " +
               std::to_string(Constants::MIN_PASSWORD_LENGTH) + " characters.";
    }
    if (password.length() > Constants::MAX_PASSWORD_LENGTH) {
        return "Password must be at most " +
               std::to_string(Constants::MAX_PASSWORD_LENGTH) + " characters.";
    }
    return "Invalid password format.";
}

int InputValidator::getPasswordStrength(const std::string& password) {
    int strength = 0;
    if (password.length() >= 8) strength++;
    if (password.length() >= 12) strength++;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    if (hasUpper && hasLower) strength++;
    if (hasDigit) strength++;
    if (hasSpecial) strength++;

    return std::min(strength, 4);
}

// ==================== Phone Validation ====================

bool InputValidator::validatePhone(const std::string& phone) {
    return Utils::isValidPhone(phone);
}

std::string InputValidator::getPhoneError(const std::string& phone) {
    if (phone.empty()) {
        return "Phone number cannot be empty.";
    }
    if (phone.length() != Constants::PHONE_LENGTH) {
        return "Phone number must be exactly " +
               std::to_string(Constants::PHONE_LENGTH) + " digits.";
    }
    if (phone[0] != '0') {
        return "Phone number must start with 0.";
    }
    return "Phone number can only contain digits.";
}

// ==================== Date Validation ====================

bool InputValidator::validateDate(const std::string& date) {
    return Utils::isValidDate(date);
}

bool InputValidator::validateFutureDate(const std::string& date) {
    return Utils::isValidDate(date) && Utils::isFutureDate(date);
}

bool InputValidator::validatePastDate(const std::string& date) {
    return Utils::isValidDate(date) && !Utils::isFutureDate(date);
}

std::string InputValidator::getDateError(const std::string& date) {
    if (date.empty()) {
        return "Date cannot be empty.";
    }
    if (date.length() != 10) {
        return "Date must be in YYYY-MM-DD format.";
    }
    return "Invalid date. Please use YYYY-MM-DD format with valid values.";
}

// ==================== Time Validation ====================

bool InputValidator::validateTime(const std::string& time) {
    return Utils::isValidTime(time);
}

bool InputValidator::validateWorkingHours(const std::string& time) {
    if (!Utils::isValidTime(time)) return false;

    int hour = std::stoi(time.substr(0, 2));
    return hour >= 8 && hour < 17;
}

std::string InputValidator::getTimeError(const std::string& time) {
    if (time.empty()) {
        return "Time cannot be empty.";
    }
    if (time.length() != 5) {
        return "Time must be in HH:MM format.";
    }
    return "Invalid time. Please use HH:MM format (24-hour).";
}

// ==================== Name Validation ====================

bool InputValidator::validateName(const std::string& name) {
    if (name.length() < 2 || name.length() > 100) return false;

    for (char c : name) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '-' && c != '\'') {
            // Allow UTF-8 characters (Vietnamese names, etc.)
            if (static_cast<unsigned char>(c) < 128) {
                return false;
            }
        }
    }
    return true;
}

std::string InputValidator::getNameError(const std::string& name) {
    if (name.empty()) {
        return "Name cannot be empty.";
    }
    if (name.length() < 2) {
        return "Name must be at least 2 characters.";
    }
    if (name.length() > 100) {
        return "Name must be at most 100 characters.";
    }
    return "Name can only contain letters, spaces, hyphens, and apostrophes.";
}

// ==================== Gender Validation ====================

bool InputValidator::validateGender(const std::string& gender) {
    std::string lower = Utils::toLower(gender);
    return lower == "male" || lower == "female" || lower == "other" ||
           lower == "m" || lower == "f" || lower == "o";
}

std::string InputValidator::normalizeGender(const std::string& gender) {
    std::string lower = Utils::toLower(gender);
    if (lower == "male" || lower == "m") return "Male";
    if (lower == "female" || lower == "f") return "Female";
    if (lower == "other" || lower == "o") return "Other";
    return "Unknown";
}

// ==================== Money Validation ====================

bool InputValidator::validateMoney(const std::string& amount) {
    return Utils::isValidMoney(amount);
}

double InputValidator::parseMoney(const std::string& amount) {
    if (!validateMoney(amount)) return -1.0;
    try {
        return std::stod(amount);
    } catch (...) {
        return -1.0;
    }
}

// ==================== ID Validation ====================

bool InputValidator::validatePatientID(const std::string& id) {
    if (id.empty() || id[0] != 'P') return false;
    return Utils::isNumeric(id.substr(1));
}

bool InputValidator::validateDoctorID(const std::string& id) {
    if (id.empty() || id[0] != 'D') return false;
    return Utils::isNumeric(id.substr(1));
}

bool InputValidator::validateAppointmentID(const std::string& id) {
    if (id.length() < 4 || id.substr(0, 3) != "APT") return false;
    return Utils::isNumeric(id.substr(3));
}

// ==================== Menu Choice Validation ====================

bool InputValidator::validateMenuChoice(const std::string& choice, int min, int max) {
    int parsed = parseMenuChoice(choice);
    return parsed >= min && parsed <= max;
}

int InputValidator::parseMenuChoice(const std::string& choice) {
    if (!Utils::isNumeric(choice)) return -1;
    try {
        return std::stoi(choice);
    } catch (...) {
        return -1;
    }
}

// ==================== General Validation ====================

bool InputValidator::isEmpty(const std::string& str) {
    return Utils::trim(str).empty();
}

bool InputValidator::isNumeric(const std::string& str) {
    return Utils::isNumeric(str);
}

bool InputValidator::isAlphabetic(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ') {
            return false;
        }
    }
    return true;
}

std::string InputValidator::sanitizeInput(const std::string& input) {
    std::string result = Utils::trim(input);
    // Remove pipe characters to prevent delimiter injection
    std::string sanitized;
    sanitized.reserve(result.size());
    for (char c : result) {
        if (c != Constants::FIELD_DELIMITER) {
            sanitized += c;
        }
    }
    return sanitized;
}

bool InputValidator::isYesConfirmation(const std::string& input) {
    std::string lower = Utils::toLower(Utils::trim(input));
    return lower == "y" || lower == "yes";
}

} // namespace UI
} // namespace HMS
