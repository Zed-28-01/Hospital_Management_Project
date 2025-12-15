#pragma once

#include <string>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @class Account
 * @brief Represents a user account for authentication
 *
 * Stores credentials and role information. Linked to Person
 * entities via username.
 */
class Account {
private:
    std::string m_username;
    std::string m_passwordHash;
    Role m_role;
    bool m_isActive;
    std::string m_createdDate;

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Account() = default;

    /**
     * @brief Parameterized constructor
     * @param username Unique username
     * @param passwordHash Hashed password
     * @param role User role (patient, doctor, admin)
     * @param isActive Account active status
     * @param createdDate Account creation date
     */
    Account(const std::string& username,
            const std::string& passwordHash,
            Role role,
            bool isActive = true,
            const std::string& createdDate = "");

    /**
     * @brief Destructor
     */
    ~Account() = default;

    // ==================== Getters ====================

    /**
     * @brief Get account username
     * @return Username string
     */
    std::string getUsername() const;

    /**
     * @brief Get password hash
     * @return Password hash string
     */
    std::string getPasswordHash() const;

    /**
     * @brief Get user role
     * @return Role enum value
     */
    Role getRole() const;

    /**
     * @brief Get role as string
     * @return Role string
     */
    std::string getRoleString() const;

    /**
     * @brief Check if account is active
     * @return True if active
     */
    bool isActive() const;

    /**
     * @brief Get account creation date
     * @return Creation date string
     */
    std::string getCreatedDate() const;

    // ==================== Setters ====================

    /**
     * @brief Set password hash
     * @param passwordHash New password hash
     */
    void setPasswordHash(const std::string& passwordHash);

    /**
     * @brief Set account active status
     * @param active New active status
     */
    void setActive(bool active);

    /**
     * @brief Set user role
     * @param role New role
     */
    void setRole(Role role);

    // ==================== Utility Methods ====================

    /**
     * @brief Verify password against stored hash
     * @param password Plain text password
     * @return True if password matches
     */
    bool verifyPassword(const std::string& password) const;

    /**
     * @brief Serialize account to string for file storage
     * @return Pipe-delimited string representation
     *
     * Format: username|passwordHash|role|isActive|createdDate
     */
    std::string serialize() const;

    // ==================== Static Factory Method ====================

    /**
     * @brief Deserialize account from string
     * @param line Pipe-delimited string from file
     * @return Account object or nullopt if parsing fails
     */
    static Result<Account> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS
