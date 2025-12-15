#pragma once

#include "Person.h"
#include <string>

namespace HMS {
namespace Model {

/**
 * @class Admin
 * @brief Represents an administrator in the hospital management system
 *
 * Inherits from Person. Admin has full access to manage doctors
 * and view system statistics.
 */
class Admin : public Person {
private:
    std::string m_adminID;
    std::string m_username;     // Links to Account

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Admin() = default;

    /**
     * @brief Parameterized constructor
     * @param adminID Unique admin identifier
     * @param username Account username
     * @param name Admin's full name
     * @param phone Phone number
     * @param gender Gender
     * @param dateOfBirth Date of birth
     */
    Admin(const std::string& adminID,
          const std::string& username,
          const std::string& name,
          const std::string& phone,
          Gender gender,
          const std::string& dateOfBirth);

    /**
     * @brief Destructor
     */
    ~Admin() override = default;

    // ==================== Getters ====================

    /**
     * @brief Get admin's unique ID
     * @return Admin ID string
     */
    std::string getID() const override;

    /**
     * @brief Get admin's unique ID (alias)
     * @return Admin ID string
     */
    std::string getAdminID() const;

    /**
     * @brief Get admin's account username
     * @return Username string
     */
    std::string getUsername() const;

    // ==================== Override Methods ====================

    /**
     * @brief Display admin information to console
     */
    void displayInfo() const override;

    /**
     * @brief Serialize admin to string for file storage
     * @return Pipe-delimited string representation
     *
     * Format: adminID|username|name|phone|gender|dateOfBirth
     */
    std::string serialize() const override;

    // ==================== Static Factory Method ====================

    /**
     * @brief Deserialize admin from string
     * @param line Pipe-delimited string from file
     * @return Admin object or nullopt if parsing fails
     */
    static Result<Admin> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS
