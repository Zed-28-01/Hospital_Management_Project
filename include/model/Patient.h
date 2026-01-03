#pragma once

#include "Person.h"
#include <string>

namespace HMS {
namespace Model {

/**
 * @class Patient
 * @brief Represents a patient in the hospital management system
 *
 * Inherits from Person and adds patient-specific attributes
 * such as patient ID, username (for account linking), address,
 * and medical history.
 */
class Patient : public Person {
private:
    std::string m_patientID;
    std::string m_username;      // Links to Account
    std::string m_address;
    std::string m_medicalHistory;

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Patient() = default;

    /**
     * @brief Parameterized constructor
     * @param patientID Unique patient identifier
     * @param username Account username
     * @param name Patient's full name
     * @param phone Phone number
     * @param gender Gender
     * @param dateOfBirth Date of birth
     * @param address Residential address
     * @param medicalHistory Medical history notes
     */
    Patient(const std::string& patientID,
            const std::string& username,
            const std::string& name,
            const std::string& phone,
            Gender gender,
            const std::string& dateOfBirth,
            const std::string& address,
            const std::string& medicalHistory);

    /**
     * @brief Destructor
     */
    ~Patient() override = default;

    // ==================== Getters ====================

    /**
     * @brief Get patient's unique ID
     * @return Patient ID string
     */
    std::string getID() const override;

    /**
     * @brief Get patient's unique ID (alias)
     * @return Patient ID string
     */
    std::string getPatientID() const;

    /**
     * @brief Get patient's account username
     * @return Username string
     */
    std::string getUsername() const;

    /**
     * @brief Get patient's address
     * @return Address string
     */
    std::string getAddress() const;

    /**
     * @brief Get patient's medical history
     * @return Medical history string
     */
    std::string getMedicalHistory() const;

    // ==================== Setters ====================

    /**
     * @brief Set patient's account username (for linking to account)
     * @param username Account username
     */
    void setUsername(const std::string& username);

    /**
     * @brief Set patient's address
     * @param address New address
     */
    void setAddress(const std::string& address);

    /**
     * @brief Set patient's medical history
     * @param medicalHistory New medical history
     */
    void setMedicalHistory(const std::string& medicalHistory);

    /**
     * @brief Append to patient's medical history
     * @param entry New entry to append
     */
    void appendMedicalHistory(const std::string& entry);

    // ==================== Override Methods ====================

    /**
     * @brief Display patient information to console
     */
    void displayInfo() const override;

    /**
     * @brief Serialize patient to string for file storage
     * @return Pipe-delimited string representation
     *
     * Format: patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory
     */
    std::string serialize() const override;

    // ==================== Static Factory Method ====================

    /**
     * @brief Deserialize patient from string
     * @param line Pipe-delimited string from file
     * @return Patient object or nullopt if parsing fails
     */
    static Result<Patient> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS
