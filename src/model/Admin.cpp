#include "model/Admin.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <iostream>
#include <format>

namespace HMS {
namespace Model {

// ==================== Constructor ====================
Admin::Admin(const std::string& adminID,
            const std::string& username,
            const std::string& name,
            const std::string& phone,
            Gender gender,
            const std::string& dateOfBirth)
    :   Person(name, phone, gender, dateOfBirth),
        m_adminID(adminID),
        m_username(username) {
}

// ==================== Getters ====================
std::string Admin::getID() const {
    return m_adminID;
}

std::string Admin::getAdminID() const {
    return m_adminID;
}

std::string Admin::getUsername() const {
    return m_username;
}

// ==================== Helper Methods ====================
std::string Admin::truncateField(const std::string& field, size_t maxLen) const {
    if (field.length() <= maxLen) {
        return field;
    }
    return field.substr(0, maxLen - 3) + "...";
}

std::string Admin::sanitizeField(const std::string& field) const {
    std::string result;
    result.reserve(field.size());
    for (char c : field) {
        if (c == Constants::FIELD_DELIMITER) {
            result += ' ';  // Replace delimiter with space
        } else {
            result += c;
        }
    }
    return result;
}

// ==================== Override Methods ====================
void Admin::displayInfo() const {
    constexpr size_t MAX_FIELD_WIDTH = 43;

    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    ADMIN INFORMATION                       ║\n";
    std::cout << "╠════════════════════════════════════════════════════════════╣\n";
    std::cout << std::format("║ Admin ID      : {:<43} ║\n", truncateField(m_adminID, MAX_FIELD_WIDTH));
    std::cout << std::format("║ Username      : {:<43} ║\n", truncateField(m_username, MAX_FIELD_WIDTH));
    std::cout << std::format("║ Name          : {:<43} ║\n", truncateField(m_name, MAX_FIELD_WIDTH));
    std::cout << std::format("║ Phone         : {:<43} ║\n", truncateField(m_phone, MAX_FIELD_WIDTH));
    std::cout << std::format("║ Gender        : {:<43} ║\n", genderToString(m_gender));
    std::cout << std::format("║ Date of Birth : {:<43} ║\n",
                            Utils::formatDateDisplay(m_dateOfBirth));
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";
}

std::string Admin::serialize() const {
    return std::format("{}|{}|{}|{}|{}|{}",
                       sanitizeField(m_adminID),
                       sanitizeField(m_username),
                       sanitizeField(m_name),
                       sanitizeField(m_phone),
                       genderToString(m_gender),
                       m_dateOfBirth);
}

// ==================== Static Factory Method ====================
Result<Admin> Admin::deserialize(const std::string& line) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == Constants::COMMENT_CHAR) {
        return std::nullopt;
    }

    // Split by delimiter
    std::vector<std::string> fields = Utils::split(line, Constants::FIELD_DELIMITER);

    // Validate field count
    if (fields.size() != 6) {
        std::cerr << std::format(
            "Error: Invalid admin record format. Expected 6 fields, got {}\n",
            fields.size()
        );
        return std::nullopt;
    }

    // Extract and trim fields
    std::string adminID = Utils::trim(fields[0]);
    std::string username = Utils::trim(fields[1]);
    std::string name = Utils::trim(fields[2]);
    std::string phone = Utils::trim(fields[3]);
    std::string genderStr = Utils::trim(fields[4]);
    std::string dateOfBirth = Utils::trim(fields[5]);

    // Validate required fields
    if (adminID.empty() || username.empty() || name.empty()) {
        std::cerr << "Error: Admin record has empty required fields\n";
        return std::nullopt;
    }

    // Validate username format
    if (!Utils::isValidUsername(username)) {
        std::cerr << std::format(
            "Error: Invalid username '{}' for admin {}\n",
            username,
            adminID
        );
        return std::nullopt;
    }

    // Validate phone number
    if (!Utils::isValidPhone(phone)) {
        std::cerr << std::format(
            "Error: Invalid phone number '{}' for admin {}\n",
            phone,
            adminID
        );
        return std::nullopt;
    }

    // Validate date format
    if (!Utils::isValidDate(dateOfBirth)) {
        std::cerr << std::format(
            "Error: Invalid date format '{}' for admin {}\n",
            dateOfBirth,
            adminID
        );
        return std::nullopt;
    }

    // Parse gender
    Gender gender = stringToGender(genderStr);
    if (gender == Gender::UNKNOWN) {
        std::cerr << std::format(
            "Warning: Unknown or empty gender '{}' for admin {}, defaulting to UNKNOWN\n",
            genderStr,
            adminID
        );
    }

    // Create and return Admin object
    return Admin(adminID, username, name, phone, gender, dateOfBirth);
}

} // namespace Model
} // namespace HMS
