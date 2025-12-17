#include "model/Appointment.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include <iostream>
#include <iomanip>

namespace HMS
{
    namespace Model
    {

        // ==================== Constructors ====================

        Appointment::Appointment(const std::string &appointmentID,
                                 const std::string &patientUsername,
                                 const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time,
                                 const std::string &disease,
                                 double price)
            : m_appointmentID(appointmentID),
              m_patientUsername(patientUsername),
              m_doctorID(doctorID),
              m_appointmentDate(date),
              m_appointmentTime(time),
              m_disease(disease),
              m_price(price),
              m_isPaid(false),
              m_status(AppointmentStatus::SCHEDULED),
              m_notes("") {}

        Appointment::Appointment(const std::string &appointmentID,
                                 const std::string &patientUsername,
                                 const std::string &doctorID,
                                 const std::string &date,
                                 const std::string &time,
                                 const std::string &disease,
                                 double price,
                                 bool isPaid,
                                 AppointmentStatus status,
                                 const std::string &notes)
            : m_appointmentID(appointmentID),
              m_patientUsername(patientUsername),
              m_doctorID(doctorID),
              m_appointmentDate(date),
              m_appointmentTime(time),
              m_disease(disease),
              m_price(price),
              m_isPaid(isPaid),
              m_status(status),
              m_notes(notes) {}

        // ==================== Getters ====================

        std::string Appointment::getAppointmentID() const
        {
            return m_appointmentID;
        }

        std::string Appointment::getPatientUsername() const
        {
            return m_patientUsername;
        }

        std::string Appointment::getDoctorID() const
        {
            return m_doctorID;
        }

        std::string Appointment::getDate() const
        {
            return m_appointmentDate;
        }

        std::string Appointment::getTime() const
        {
            return m_appointmentTime;
        }

        std::string Appointment::getDateTime() const
        {
            return m_appointmentDate + " " + m_appointmentTime;
        }

        std::string Appointment::getDisease() const
        {
            return m_disease;
        }

        double Appointment::getPrice() const
        {
            return m_price;
        }

        bool Appointment::isPaid() const
        {
            return m_isPaid;
        }

        AppointmentStatus Appointment::getStatus() const
        {
            return m_status;
        }

        std::string Appointment::getStatusString() const
        {
            return statusToString(m_status);
        }

        std::string Appointment::getNotes() const
        {
            return m_notes;
        }

        // ==================== Setters ====================

        void Appointment::setDate(const std::string &date)
        {
            if (Utils::isValidDate(date))
            {
                m_appointmentDate = date;
            }
        }

        void Appointment::setTime(const std::string &time)
        {
            if (Utils::isValidTime(time))
            {
                m_appointmentTime = time;
            }
        }

        void Appointment::setDisease(const std::string &disease)
        {
            m_disease = disease;
        }

        void Appointment::setPrice(double price)
        {
            if (price >= 0)
            {
                m_price = price;
            }
        }

        void Appointment::setPaid(bool paid)
        {
            m_isPaid = paid;
        }

        void Appointment::setStatus(AppointmentStatus status)
        {
            m_status = status;
        }

        void Appointment::setNotes(const std::string &notes)
        {
            m_notes = notes;
        }

        // ==================== Status Methods ====================

        void Appointment::markAsCompleted()
        {
            m_status = AppointmentStatus::COMPLETED;
        }

        void Appointment::markAsCancelled()
        {
            m_status = AppointmentStatus::CANCELLED;
        }

        void Appointment::markAsNoShow()
        {
            m_status = AppointmentStatus::NO_SHOW;
        }

        bool Appointment::isUpcoming() const
        {
            return m_status == AppointmentStatus::SCHEDULED &&
                   Utils::isFutureDate(m_appointmentDate);
        }

        bool Appointment::canCancel() const
        {
            return m_status == AppointmentStatus::SCHEDULED &&
                   Utils::isFutureDate(m_appointmentDate);
        }

        bool Appointment::canEdit() const
        {
            return m_status == AppointmentStatus::SCHEDULED &&
                   Utils::isFutureDate(m_appointmentDate);
        }

        // ==================== Serialization ====================

        void Appointment::displayInfo() const
        {
            std::cout << "\n"
                      << std::string(50, '=') << "\n";
            std::cout << "Appointment Details\n";
            std::cout << std::string(50, '=') << "\n";
            std::cout << std::left;
            std::cout << std::setw(20) << "Appointment ID:" << m_appointmentID << "\n";
            std::cout << std::setw(20) << "Patient:" << m_patientUsername << "\n";
            std::cout << std::setw(20) << "Doctor ID:" << m_doctorID << "\n";
            std::cout << std::setw(20) << "Date & Time:" << getDateTime() << "\n";
            std::cout << std::setw(20) << "Disease:" << m_disease << "\n";
            std::cout << std::setw(20) << "Price:" << Utils::formatMoney(m_price) << "\n";
            std::cout << std::setw(20) << "Status:" << getStatusString() << "\n";
            std::cout << std::setw(20) << "Paid:" << (m_isPaid ? "Yes" : "No") << "\n";
            if (!m_notes.empty())
            {
                std::cout << std::setw(20) << "Notes:" << m_notes << "\n";
            }
            std::cout << std::string(50, '=') << "\n";
        }

        std::string Appointment::serialize() const
        {
            std::string result;
            result += m_appointmentID;
            result += Constants::FIELD_DELIMITER;
            result += m_patientUsername;
            result += Constants::FIELD_DELIMITER;
            result += m_doctorID;
            result += Constants::FIELD_DELIMITER;
            result += m_appointmentDate;
            result += Constants::FIELD_DELIMITER;
            result += m_appointmentTime;
            result += Constants::FIELD_DELIMITER;
            result += m_disease;
            result += Constants::FIELD_DELIMITER;
            result += std::to_string(m_price);
            result += Constants::FIELD_DELIMITER;
            result += (m_isPaid ? "1" : "0");
            result += Constants::FIELD_DELIMITER;
            result += statusToString(m_status);
            result += Constants::FIELD_DELIMITER;
            result += m_notes;
            return result;
        }

        // ==================== Static Factory Method ====================

        Result<Appointment> Appointment::deserialize(const std::string &line)
        {
            // Skip comments and empty lines
            if (line.empty() || line[0] == Constants::COMMENT_CHAR)
            {
                return std::nullopt;
            }

            // Split by delimiter
            auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

            // Expected 10 fields
            if (parts.size() != 10)
            {
                return std::nullopt;
            }

            try
            {
                std::string appointmentID = parts[0];
                std::string patientUsername = parts[1];
                std::string doctorID = parts[2];
                std::string date = parts[3];
                std::string time = parts[4];
                std::string disease = parts[5];
                double price = std::stod(parts[6]);
                bool isPaid = (parts[7] == "1");
                AppointmentStatus status = stringToStatus(parts[8]);
                std::string notes = parts[9];

                // Validate date and time formats
                if (!Utils::isValidDate(date) || !Utils::isValidTime(time))
                {
                    return std::nullopt;
                }

                // Validate price
                if (price < 0)
                {
                    return std::nullopt;
                }

                return Appointment(appointmentID, patientUsername, doctorID,
                                   date, time, disease, price, isPaid, status, notes);
            }
            catch (const std::exception &e)
            {
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS
