#include "model/Account.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include <iostream>
#include <format>

namespace HMS {
namespace Model {

// ==================== Constructor ====================

Account::Account(const std::string& username,
                 const std::string& passwordHash,
                 Role role,
                 bool isActive,
                 const std::string& createdDate)
    : m_username(username),
      m_passwordHash(passwordHash),
      m_role(role),
      m_isActive(isActive),
      m_createdDate(createdDate) {}

// ==================== Getters ====================

std::string Account::getUsername() const {
    return m_username;
}

std::string Account::getPasswordHash() const {
    return m_passwordHash;
}

Role Account::getRole() const {
    return m_role;
}

std::string Account::getRoleString() const {
    return roleToString(m_role);
}

bool Account::isActive() const {
    return m_isActive;
}

std::string Account::getCreatedDate() const {
    return m_createdDate;
}

// ==================== Setters ====================

void Account::setPasswordHash(const std::string& passwordHash) {
    m_passwordHash = passwordHash;
}

void Account::setActive(bool active) {
    m_isActive = active;
}

void Account::setRole(Role role) {
    m_role = role;
}

// ==================== Utility Methods ====================

bool Account::verifyPassword(const std::string& password) const {
    // : thay bằng hash thực
    return password == m_passwordHash;
}

std::string Account::serialize() const {
    return std::format("{}|{}|{}|{}|{}",
        m_username,
        m_passwordHash,
        roleToString(m_role),
        (m_isActive ? "1" : "0"),
        m_createdDate);
}

// ==================== Deserialize ====================

Result<Account> Account::deserialize(const std::string& line) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == Constants::COMMENT_CHAR) {
        return std::nullopt;
    }

    // Split by delimiter
    auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

    // Validate field count
    if (parts.size() != 5) {
        std::cerr << std::format("Error: Invalid account format. Expected 5 fields, got {}\n",
                                parts.size());
        return std::nullopt;
    }

    // Extract and trim fields
    std::string username = Utils::trim(parts[0]);
    std::string passwordHash = Utils::trim(parts[1]);
    std::string roleStr = Utils::trim(parts[2]);
    std::string activeStr = Utils::trim(parts[3]);
    std::string createdDate = Utils::trim(parts[4]);

    // Validate required fields are not empty
    if (username.empty() || passwordHash.empty()) {
        std::cerr << "Error: Account record has empty required fields\n";
        return std::nullopt;
    }

    // Validate username format
    if (!Utils::isValidUsername(username)) {
        std::cerr << std::format("Error: Invalid username '{}'\n", username);
        return std::nullopt;
    }

    // Parse role
    Role role = stringToRole(roleStr);
    if (role == Role::UNKNOWN) {
        std::cerr << std::format("Error: Invalid role '{}' for account {}\n",
                                roleStr, username);
        return std::nullopt;
    }

    return Account(username, passwordHash, role, activeStr == "1", createdDate);
}

} // namespace Model
} // namespace HMS
