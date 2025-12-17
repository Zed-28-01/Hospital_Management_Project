#include "model/Account.h"
#include "common/Types.h"
#include <sstream>
#include <vector>

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
    std::ostringstream oss;
    oss << m_username << "|"
        << m_passwordHash << "|"
        << roleToString(m_role) << "|"
        << (m_isActive ? "1" : "0") << "|"
        << m_createdDate;
    return oss.str();
}

// ==================== Deserialize ====================

Result<Account> Account::deserialize(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(ss, token, '|')) {
        parts.push_back(token);
    }

    //Sai format → thất bại
    if (parts.size() != 5) {
        return std::nullopt;
    }

    Account acc(
        parts[0],                     // username
        parts[1],                     // password hash
        stringToRole(parts[2]),        // role
        parts[3] == "1",               // isActive
        parts[4]                      // createdDate
    );

    return acc;
}

} // namespace Model
} // namespace HMS
