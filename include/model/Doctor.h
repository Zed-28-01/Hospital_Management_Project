#pragma once

#include "Person.h"
#include <string>

namespace HMS {
namespace Model {

/**
 * @class Doctor
 * @brief Represents a doctor in the hospital management system
 *
 * Inherits from Person and adds doctor-specific attributes
 * such as doctor ID, specialization, schedule, and consultation fee.
 */
class Doctor : public Person {
private:
    std::string m_doctorID;
    std::string m_username;         // Links to Account
    std::string m_specialization;
    std::string m_schedule;         // e.g., "Mon-Fri 08:00-17:00"
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
     * @param schedule Working schedule
     * @param consultationFee Fee per consultation
     */
    Doctor(const std::string& doctorID,
           const std::string& username,
           const std::string& name,
           const std::string& phone,
           Gender gender,
           const std::string& dateOfBirth,
           const std::string& specialization,
           const std::string& schedule,
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
     * @brief Get doctor's medical specialization
     * @return Specialization string
     */
    std::string getSpecialization() const;

    /**
     * @brief Get doctor's working schedule
     * @return Schedule string
     */
    std::string getSchedule() const;

    /**
     * @brief Get doctor's consultation fee
     * @return Fee amount
     */
    double getConsultationFee() const;

    // ==================== Setters ====================

    /**
     * @brief Set doctor's specialization
     * @param specialization New specialization
     */
    void setSpecialization(const std::string& specialization);

    /**
     * @brief Set doctor's working schedule
     * @param schedule New schedule
     */
    void setSchedule(const std::string& schedule);

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
     * Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee
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
