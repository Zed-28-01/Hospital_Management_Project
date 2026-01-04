#pragma once

#include "Person.h"
#include <string>
#include <vector>

namespace HMS {
namespace Model {

/**
 * @class Doctor
 * @brief Represents a doctor in the hospital management system
 *
 * Inherits from Person and adds doctor-specific attributes
 * such as doctor ID, specialization, and consultation fee.
 * All doctors work Monday-Sunday from 08:00 to 17:00.
 */
class Doctor : public Person {
private:
    std::string m_doctorID;
    std::string m_username;         // Links to Account
    std::string m_specialization;
    double m_consultationFee;

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Doctor() = default;

    /**
     * @brief Parameterized constructor
     * @param doctorID Unique doctor identifier
     * @param username Account username
     * @param name Doctor's full name
     * @param phone Phone number
     * @param gender Gender
     * @param dateOfBirth Date of birth
     * @param specialization Medical specialization
     * @param consultationFee Fee per consultation
     */
    Doctor(const std::string& doctorID,
           const std::string& username,
           const std::string& name,
           const std::string& phone,
           Gender gender,
           const std::string& dateOfBirth,
           const std::string& specialization,
           double consultationFee);

    /**
     * @brief Destructor
     */
    ~Doctor() override = default;

    // ==================== Getters ====================

    /**
     * @brief Get doctor's unique ID
     * @return Doctor ID string
     */
    std::string getID() const override;

    /**
     * @brief Get doctor's unique ID (alias)
     * @return Doctor ID string
     */
    std::string getDoctorID() const;

    /**
     * @brief Get doctor's account username
     * @return Username string
     */
    std::string getUsername() const;

    /**
     * @brief Get doctor's medical specialization (primary/first or all joined)
     * @return Specialization string
     * @note For backwards compatibility. Returns first specialization or all comma-joined
     */
    std::string getSpecialization() const;

    /**
     * @brief Get all doctor's medical specializations
     * @return Vector of specialization strings
     */
    std::vector<std::string> getSpecializations() const;

    /**
     * @brief Get doctor's consultation fee
     * @return Fee amount
     */
    double getConsultationFee() const;

    // ==================== Setters ====================

    /**
     * @brief Set doctor's specialization (overwrites all existing)
     * @param specialization New specialization
     * @note For backwards compatibility. Overwrites all specializations
     */
    void setSpecialization(const std::string& specialization);

    /**
     * @brief Add a specialization to the doctor
     * @param specialization Specialization to add
     * @note Duplicate specializations are ignored
     */
    void addSpecialization(const std::string& specialization);

    /**
     * @brief Remove a specialization from the doctor
     * @param specialization Specialization to remove
     * @return True if removed, false if not found
     */
    bool removeSpecialization(const std::string& specialization);

    /**
     * @brief Check if doctor has a specific specialization
     * @param specialization Specialization to check
     * @return True if doctor has this specialization
     */
    bool hasSpecialization(const std::string& specialization) const;

    /**
     * @brief Clear all specializations
     */
    void clearSpecializations();

    /**
     * @brief Set doctor's consultation fee
     * @param fee New fee amount
     */
    void setConsultationFee(double fee);

    // ==================== Override Methods ====================

    /**
     * @brief Display doctor information to console
     */
    void displayInfo() const override;

    /**
     * @brief Serialize doctor to string for file storage
     * @return Pipe-delimited string representation
     *
     * Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|consultationFee
     */
    std::string serialize() const override;

    // ==================== Static Factory Method ====================

    /**
     * @brief Deserialize doctor from string
     * @param line Pipe-delimited string from file
     * @return Doctor object or nullopt if parsing fails
     */
    static Result<Doctor> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS
