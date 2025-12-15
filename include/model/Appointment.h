#pragma once

#include <string>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @class Appointment
 * @brief Represents a medical appointment in the system
 *
 * Links patients to doctors with scheduling information,
 * payment status, and appointment status tracking.
 */
class Appointment {
private:
    std::string m_appointmentID;
    std::string m_patientUsername;
    std::string m_doctorID;
    std::string m_appointmentDate;  // YYYY-MM-DD
    std::string m_appointmentTime;  // HH:MM
    std::string m_disease;
    double m_price;
    bool m_isPaid;
    AppointmentStatus m_status;
    std::string m_notes;

public:
    // ==================== Constructors ====================

    /**
     * @brief Default constructor
     */
    Appointment() = default;

    /**
     * @brief Parameterized constructor
     * @param appointmentID Unique appointment identifier
     * @param patientUsername Patient's username
     * @param doctorID Doctor's ID
     * @param date Appointment date (YYYY-MM-DD)
     * @param time Appointment time (HH:MM)
     * @param disease Description of disease/symptoms
     * @param price Consultation price
     */
    Appointment(const std::string& appointmentID,
                const std::string& patientUsername,
                const std::string& doctorID,
                const std::string& date,
                const std::string& time,
                const std::string& disease,
                double price);

    /**
     * @brief Full parameterized constructor
     * @param appointmentID Unique appointment identifier
     * @param patientUsername Patient's username
     * @param doctorID Doctor's ID
     * @param date Appointment date (YYYY-MM-DD)
     * @param time Appointment time (HH:MM)
     * @param disease Description of disease/symptoms
     * @param price Consultation price
     * @param isPaid Payment status
     * @param status Appointment status
     * @param notes Additional notes
     */
    Appointment(const std::string& appointmentID,
                const std::string& patientUsername,
                const std::string& doctorID,
                const std::string& date,
                const std::string& time,
                const std::string& disease,
                double price,
                bool isPaid,
                AppointmentStatus status,
                const std::string& notes);

    /**
     * @brief Destructor
     */
    ~Appointment() = default;

    // ==================== Getters ====================

    /**
     * @brief Get appointment ID
     * @return Appointment ID string
     */
    std::string getAppointmentID() const;

    /**
     * @brief Get patient's username
     * @return Patient username string
     */
    std::string getPatientUsername() const;

    /**
     * @brief Get doctor's ID
     * @return Doctor ID string
     */
    std::string getDoctorID() const;

    /**
     * @brief Get appointment date
     * @return Date string (YYYY-MM-DD)
     */
    std::string getDate() const;

    /**
     * @brief Get appointment time
     * @return Time string (HH:MM)
     */
    std::string getTime() const;

    /**
     * @brief Get datetime combined
     * @return DateTime string (YYYY-MM-DD HH:MM)
     */
    std::string getDateTime() const;

    /**
     * @brief Get disease/symptoms description
     * @return Disease string
     */
    std::string getDisease() const;

    /**
     * @brief Get consultation price
     * @return Price amount
     */
    double getPrice() const;

    /**
     * @brief Check if appointment is paid
     * @return True if paid
     */
    bool isPaid() const;

    /**
     * @brief Get appointment status
     * @return Status enum value
     */
    AppointmentStatus getStatus() const;

    /**
     * @brief Get status as string
     * @return Status string
     */
    std::string getStatusString() const;

    /**
     * @brief Get additional notes
     * @return Notes string
     */
    std::string getNotes() const;

    // ==================== Setters ====================

    /**
     * @brief Set appointment date
     * @param date New date (YYYY-MM-DD)
     */
    void setDate(const std::string& date);

    /**
     * @brief Set appointment time
     * @param time New time (HH:MM)
     */
    void setTime(const std::string& time);

    /**
     * @brief Set disease/symptoms
     * @param disease New disease description
     */
    void setDisease(const std::string& disease);

    /**
     * @brief Set consultation price
     * @param price New price
     */
    void setPrice(double price);

    /**
     * @brief Set payment status
     * @param paid New payment status
     */
    void setPaid(bool paid);

    /**
     * @brief Set appointment status
     * @param status New status
     */
    void setStatus(AppointmentStatus status);

    /**
     * @brief Set additional notes
     * @param notes New notes
     */
    void setNotes(const std::string& notes);

    // ==================== Status Methods ====================

    /**
     * @brief Mark appointment as completed
     */
    void markAsCompleted();

    /**
     * @brief Mark appointment as cancelled
     */
    void markAsCancelled();

    /**
     * @brief Mark patient as no-show
     */
    void markAsNoShow();

    /**
     * @brief Check if appointment is in the future
     * @return True if scheduled for future date/time
     */
    bool isUpcoming() const;

    /**
     * @brief Check if appointment can be cancelled
     * @return True if can be cancelled
     */
    bool canCancel() const;

    /**
     * @brief Check if appointment can be edited
     * @return True if can be edited
     */
    bool canEdit() const;

    // ==================== Serialization ====================

    /**
     * @brief Display appointment information to console
     */
    void displayInfo() const;

    /**
     * @brief Serialize appointment to string for file storage
     * @return Pipe-delimited string representation
     *
     * Format: appointmentID|patientUsername|doctorID|date|time|disease|price|isPaid|status|notes
     */
    std::string serialize() const;

    // ==================== Static Factory Method ====================

    /**
     * @brief Deserialize appointment from string
     * @param line Pipe-delimited string from file
     * @return Appointment object or nullopt if parsing fails
     */
    static Result<Appointment> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS
