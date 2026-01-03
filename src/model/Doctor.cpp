#include "model/Doctor.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <format>
#include <iostream>

namespace HMS
{
    namespace Model
    {

        // ==================== Constructor ====================
        Doctor::Doctor(const std::string &doctorID,
                       const std::string &username,
                       const std::string &name,
                       const std::string &phone,
                       Gender gender,
                       const std::string &dateOfBirth,
                       const std::string &specialization,
                       const std::string &schedule,
                       double consultationFee)
            : Person(name, phone, gender, dateOfBirth),
              m_doctorID(doctorID),
              m_username(username),
              m_specialization(specialization),
              m_schedule(schedule),
              m_consultationFee(consultationFee) {}

        // ==================== Getters ====================
        std::string Doctor::getID() const
        {
            return m_doctorID;
        }

        std::string Doctor::getDoctorID() const
        {
            return m_doctorID;
        }

        std::string Doctor::getUsername() const
        {
            return m_username;
        }

        std::string Doctor::getSpecialization() const
        {
            return m_specialization;
        }

        std::string Doctor::getSchedule() const
        {
            return m_schedule;
        }

        double Doctor::getConsultationFee() const
        {
            return m_consultationFee;
        }

        // ==================== Setters ====================
        void Doctor::setSpecialization(const std::string &specialization)
        {
            m_specialization = specialization;
        }

        void Doctor::setSchedule(const std::string &schedule)
        {
            m_schedule = schedule;
        }

        void Doctor::setConsultationFee(double fee)
        {
            m_consultationFee = fee;
        }

        // ==================== Override Methods ====================
        void Doctor::displayInfo() const
        {
            std::cout << "\n========================================\n";
            std::cout << "         DOCTOR INFORMATION           \n";
            std::cout << "========================================\n";
            std::cout << std::format("{:<18}: {}\n", "Doctor ID", m_doctorID);
            std::cout << std::format("{:<18}: {}\n", "Username", m_username);
            std::cout << std::format("{:<18}: {}\n", "Name", m_name);
            std::cout << std::format("{:<18}: {}\n", "Phone", m_phone);
            std::cout << std::format("{:<18}: {}\n", "Gender", genderToString(m_gender));
            std::cout << std::format("{:<18}: {}\n", "Date of Birth", m_dateOfBirth);
            std::cout << std::format("{:<18}: {}\n", "Specialization", m_specialization);
            std::cout << std::format("{:<18}: {}\n", "Schedule", m_schedule);
            std::cout << std::format("{:<18}: {}\n", "Consultation Fee", Utils::formatMoney(m_consultationFee));
            std::cout << "========================================\n\n";
        }

        std::string Doctor::serialize() const
        {
            return std::format("{}|{}|{}|{}|{}|{}|{}|{}|{:.0f}",
                               m_doctorID,
                               m_username,
                               m_name,
                               m_phone,
                               genderToString(m_gender),
                               m_dateOfBirth,
                               m_specialization,
                               m_schedule,
                               m_consultationFee);
        }

        // ==================== Static Factory Method ====================
        Result<Doctor> Doctor::deserialize(const std::string &line)
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == Constants::COMMENT_CHAR)
            {
                return std::nullopt;
            }

            // Split by delimiter
            auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

            // Validate field count
            if (parts.size() != 9)
            {
                std::cerr << std::format("Error: Invalid doctor format. Expected 9 fields, got {}\n",
                                         parts.size());
                return std::nullopt;
            }

            try
            {
                // Extract and trim fields
                std::string doctorID = Utils::trim(parts[0]);
                std::string username = Utils::trim(parts[1]);
                std::string name = Utils::trim(parts[2]);
                std::string phone = Utils::trim(parts[3]);
                std::string genderStr = Utils::trim(parts[4]);
                std::string dateOfBirth = Utils::trim(parts[5]);
                std::string specialization = Utils::trim(parts[6]);
                std::string schedule = Utils::trim(parts[7]);
                double consultationFee = std::stod(Utils::trim(parts[8]));

                // Validate required fields are not empty
                if (doctorID.empty() || username.empty() || name.empty())
                {
                    std::cerr << "Error: Doctor record has empty required fields\n";
                    return std::nullopt;
                }

                // Validate username format
                if (!Utils::isValidUsername(username))
                {
                    std::cerr << std::format("Error: Invalid username '{}' for doctor {}\n",
                                             username, doctorID);
                    return std::nullopt;
                }

                // Validate phone number
                if (!Utils::isValidPhone(phone))
                {
                    std::cerr << std::format("Error: Invalid phone number '{}' for doctor {}\n",
                                             phone, doctorID);
                    return std::nullopt;
                }

                // Validate date format (internal format YYYY-MM-DD)
                if (!Utils::isValidDateInternal(dateOfBirth))
                {
                    std::cerr << std::format("Error: Invalid date format '{}' for doctor {}\n",
                                             dateOfBirth, doctorID);
                    return std::nullopt;
                }

                // Validate consultation fee
                if (consultationFee < 0)
                {
                    std::cerr << std::format("Error: Invalid consultation fee for doctor {}\n",
                                             doctorID);
                    return std::nullopt;
                }

                // Parse gender
                Gender gender = stringToGender(genderStr);
                if (gender == Gender::UNKNOWN)
                {
                    std::cerr << std::format("Warning: Unknown gender '{}' for doctor {}, defaulting to UNKNOWN\n",
                                             genderStr, doctorID);
                }

                return Doctor(doctorID, username, name, phone, gender,
                              dateOfBirth, specialization, schedule, consultationFee);
            }
            catch (const std::exception &e)
            {
                std::cerr << std::format("Error: Failed to parse doctor record: {}\n", e.what());
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS