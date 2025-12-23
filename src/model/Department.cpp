#include "advance/Department.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <algorithm>
#include <format>
#include <iostream>

namespace HMS
{
    namespace Model
    {

        // ==================== Constructor ====================
        Department::Department(const std::string &departmentID, const std::string &name,
                               const std::string &description,
                               const std::string &headDoctorID)
            : m_departmentID(departmentID), m_name(name), m_description(description),
              m_headDoctorID(headDoctorID), m_doctorIDs(), m_location(""), m_phone("")
        {
        }

        // ==================== Getters ====================
        std::string Department::getDepartmentID() const { return m_departmentID; }

        std::string Department::getName() const { return m_name; }

        std::string Department::getDescription() const { return m_description; }

        std::string Department::getHeadDoctorID() const { return m_headDoctorID; }

        std::vector<std::string> Department::getDoctorIDs() const
        {
            return m_doctorIDs;
        }

        std::string Department::getLocation() const { return m_location; }

        std::string Department::getPhone() const { return m_phone; }

        // ==================== Setters ====================
        void Department::setName(const std::string &name) { m_name = name; }

        void Department::setDescription(const std::string &description)
        {
            m_description = description;
        }

        void Department::setHeadDoctorID(const std::string &headDoctorID)
        {
            m_headDoctorID = headDoctorID;
        }

        void Department::setLocation(const std::string &location)
        {
            m_location = location;
        }

        void Department::setPhone(const std::string &phone)
        {
            // Validate phone if not empty
            if (phone.empty() || Utils::isValidPhone(phone))
            {
                m_phone = phone;
            }
        }

        // ==================== Doctor Management ====================
        void Department::addDoctor(const std::string &doctorID)
        {
            if (doctorID.empty())
            {
                return;
            }

            // Avoid duplicate entries
            if (!hasDoctor(doctorID))
            {
                m_doctorIDs.push_back(doctorID);
            }
        }

        void Department::removeDoctor(const std::string &doctorID)
        {
            auto it = std::find(m_doctorIDs.begin(), m_doctorIDs.end(), doctorID);
            if (it != m_doctorIDs.end())
            {
                m_doctorIDs.erase(it);
            }
        }

        bool Department::hasDoctor(const std::string &doctorID) const
        {
            return std::find(m_doctorIDs.begin(), m_doctorIDs.end(), doctorID) !=
                   m_doctorIDs.end();
        }

        size_t Department::getDoctorCount() const { return m_doctorIDs.size(); }

        // ==================== Serialization ====================
        void Department::displayInfo() const
        {
            std::cout << "\n========================================\n";
            std::cout << "       DEPARTMENT INFORMATION          \n";
            std::cout << "========================================\n";
            std::cout << std::format("{:<18}: {}\n", "Department ID", m_departmentID);
            std::cout << std::format("{:<18}: {}\n", "Name", m_name);
            if (!m_description.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Description", m_description);
            }
            if (!m_headDoctorID.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Head Doctor", m_headDoctorID);
            }
            std::cout << std::format("{:<18}: {}\n", "Doctor Count", m_doctorIDs.size());
            if (!m_doctorIDs.empty())
            {
                std::cout << std::format(
                    "{:<18}: {}\n", "Doctors",
                    Utils::join(m_doctorIDs, Constants::LIST_DELIMITER));
            }
            if (!m_location.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Location", m_location);
            }
            if (!m_phone.empty())
            {
                std::cout << std::format("{:<18}: {}\n", "Phone", m_phone);
            }
            std::cout << "========================================\n\n";
        }

        std::string Department::serialize() const
        {
            // Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
            // doctorIDs is comma-separated list
            std::string doctorIDsStr =
                Utils::join(m_doctorIDs, Constants::LIST_DELIMITER);

            return std::format("{}|{}|{}|{}|{}|{}|{}", m_departmentID, m_name,
                               m_description, m_headDoctorID, doctorIDsStr, m_location,
                               m_phone);
        }

        // ==================== Static Factory Method ====================
        Result<Department> Department::deserialize(const std::string &line)
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == Constants::COMMENT_CHAR)
            {
                return std::nullopt;
            }

            // Split by delimiter
            auto parts = Utils::split(line, Constants::FIELD_DELIMITER);

            // Expected 7 fields
            if (parts.size() != 7)
            {
                std::cerr << std::format(
                    "Error: Invalid department format. Expected 7 fields, got {}\n",
                    parts.size());
                return std::nullopt;
            }

            try
            {
                std::string departmentID = Utils::trim(parts[0]);
                std::string name = Utils::trim(parts[1]);
                std::string description = Utils::trim(parts[2]);
                std::string headDoctorID = Utils::trim(parts[3]);
                std::string doctorIDsStr = Utils::trim(parts[4]);
                std::string location = Utils::trim(parts[5]);
                std::string phone = Utils::trim(parts[6]);

                // Validate required fields are not empty
                if (departmentID.empty() || name.empty())
                {
                    std::cerr << "Error: Department record has empty required fields\n";
                    return std::nullopt;
                }

                // Validate phone format if provided
                if (!phone.empty() && !Utils::isValidPhone(phone))
                {
                    std::cerr << std::format(
                        "Error: Invalid phone number '{}' for department {}\n", phone,
                        departmentID);
                    return std::nullopt;
                }

                // Create Department object
                Department dept(departmentID, name, description, headDoctorID);
                dept.setLocation(location);
                dept.setPhone(phone);

                // Parse and add doctor IDs (comma-separated)
                if (!doctorIDsStr.empty())
                {
                    auto doctorIDs = Utils::split(doctorIDsStr, Constants::LIST_DELIMITER);
                    for (const auto &doctorID : doctorIDs)
                    {
                        std::string trimmedID = Utils::trim(doctorID);
                        if (!trimmedID.empty())
                        {
                            dept.addDoctor(trimmedID);
                        }
                    }
                }

                return dept;
            }
            catch (const std::exception &e)
            {
                std::cerr << std::format("Error: Failed to parse department record: {}\n",
                                         e.what());
                return std::nullopt;
            }
        }

    } // namespace Model
} // namespace HMS
