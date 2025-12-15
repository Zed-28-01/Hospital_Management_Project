#pragma once

#include <string>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @class Person
 * @brief Abstract base class for all person entities in the system
 *
 * Provides common attributes and interface for Patient, Doctor, and Admin.
 * Uses protected members to allow access by derived classes.
 */
class Person {
protected:
    std::string m_name;
    std::string m_phone;
    Gender m_gender;
    std::string m_dateOfBirth;

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Person() = default;

    /**
     * @brief Parameterized constructor
     * @param name Person's full name
     * @param phone Phone number
     * @param gender Gender enum value
     * @param dateOfBirth Date of birth (YYYY-MM-DD format)
     */
    Person(const std::string& name,
           const std::string& phone,
           Gender gender,
           const std::string& dateOfBirth);

    /**
     * @brief Virtual destructor for proper inheritance
     */
    virtual ~Person() = default;

    // ==================== Getters ====================

    /**
     * @brief Get person's name
     * @return Name string
     */
    std::string getName() const;

    /**
     * @brief Get person's phone number
     * @return Phone string
     */
    std::string getPhone() const;

    /**
     * @brief Get person's gender
     * @return Gender enum value
     */
    Gender getGender() const;

    /**
     * @brief Get person's gender as string
     * @return Gender string
     */
    std::string getGenderString() const;

    /**
     * @brief Get person's date of birth
     * @return Date string
     */
    std::string getDateOfBirth() const;

    // ==================== Setters ====================

    /**
     * @brief Set person's name
     * @param name New name
     */
    void setName(const std::string& name);

    /**
     * @brief Set person's phone number
     * @param phone New phone number
     */
    void setPhone(const std::string& phone);

    /**
     * @brief Set person's gender
     * @param gender New gender
     */
    void setGender(Gender gender);

    /**
     * @brief Set person's date of birth
     * @param dateOfBirth New date of birth
     */
    void setDateOfBirth(const std::string& dateOfBirth);

    // ==================== Pure Virtual Methods ====================

    /**
     * @brief Display person information (pure virtual)
     *
     * Must be implemented by derived classes to display
     * role-specific information.
     */
    virtual void displayInfo() const = 0;

    /**
     * @brief Serialize person to string for file storage (pure virtual)
     * @return Delimited string representation
     */
    virtual std::string serialize() const = 0;

    /**
     * @brief Get the unique identifier (pure virtual)
     * @return ID string
     */
    virtual std::string getID() const = 0;
};

} // namespace Model
} // namespace HMS
