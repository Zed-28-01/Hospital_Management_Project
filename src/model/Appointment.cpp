#include "model/Appointment.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include <iostream>
#include <iomanip>
#include <format>
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
        // Model is a data container - validation is done at BLL layer

        void Appointment::setDate(const std::string &date)
        {
            m_appointmentDate = date;
        }

        void Appointment::setTime(const std::string &time)
        {
            m_appointmentTime = time;
        }

        void Appointment::setDisease(const std::string &disease)
        {
            m_disease = disease;
        }

        void Appointment::setPrice(double price)
        {
            m_price = price;
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
            if (m_status != AppointmentStatus::SCHEDULED)
            {
                return false;
            }

            std::string currentDate = Utils::getCurrentDate();
            int dateCompare = Utils::compareDates(m_appointmentDate, currentDate);

            if (dateCompare > 0)
            {
                return true; // Future date
            }
            if (dateCompare < 0)
            {
                return false; // Past date
            }
            // Same day - compare time
            return m_appointmentTime > Utils::getCurrentTime();
        }

        bool Appointment::canCancel() const
        {
            return isUpcoming();
        }

        bool Appointment::canEdit() const
        {
            return isUpcoming();
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
            return std::format("{}|{}|{}|{}|{}|{}|{:.0f}|{}|{}|{}",
                               m_appointmentID,
                               m_patientUsername,
                               m_doctorID,
                               m_appointmentDate,
                               m_appointmentTime,
                               m_disease,
                               m_price,
                               (m_isPaid ? "1" : "0"),
                               statusToString(m_status),
                               m_notes);
        }

        // ==================== Static Factory Method ====================

        Result<Appointment> Appointment::deserialize(const std::string &line)
        {
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
                // Trim all parts to remove leading/trailing whitespace
                std::string appointmentID = Utils::trim(parts[0]);
                std::string patientUsername = Utils::trim(parts[1]);
                std::string doctorID = Utils::trim(parts[2]);
                std::string date = Utils::trim(parts[3]);
                std::string time = Utils::trim(parts[4]);
                std::string disease = Utils::trim(parts[5]);
                double price = std::stod(Utils::trim(parts[6]));
                bool isPaid = (Utils::trim(parts[7]) == "1");
                AppointmentStatus status = stringToStatus(Utils::trim(parts[8]));
                std::string notes = Utils::trim(parts[9]);

                // Validate required fields are not empty
                if (appointmentID.empty() || patientUsername.empty() || doctorID.empty())
                {
                    return std::nullopt;
                }

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

                // Validate status is not unknown
                if (status == AppointmentStatus::UNKNOWN)
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
