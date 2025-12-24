#pragma once

/**
 * @file Department.h
 * @brief Department model for hospital department management
 *
 * Provides functionality for organizing doctors into departments,
 * tracking department heads, and managing department information.
 */

#include <string>
#include <vector>
#include "../common/Types.h"

namespace HMS
{
    namespace Model
    {

        /**
         * @class Department
         * @brief Represents a hospital department in the management system
         *
         * Manages department information including assigned doctors,
         * department head, location, and contact details. Supports
         * serialization for persistent storage.
         */
        class Department
        {
        private:
            std::string m_departmentID;            ///< Unique department identifier (e.g., "DEP001")
            std::string m_name;                    ///< Department name (e.g., "Cardiology")
            std::string m_description;             ///< Department description
            std::string m_headDoctorID;            ///< ID of the department head doctor
            std::vector<std::string> m_doctorIDs;  ///< List of assigned doctor IDs
            std::string m_location;                ///< Physical location (e.g., "Building A, Floor 2")
            std::string m_phone;                   ///< Department contact phone number

        public:
            // ==================== Constructors ====================

            /**
             * @brief Default constructor
             */
            Department() = default;

            /**
             * @brief Parameterized constructor
             * @param departmentID Unique department identifier
             * @param name Department name
             * @param description Department description
             * @param headDoctorID ID of the department head doctor
             */
            Department(const std::string &departmentID,
                       const std::string &name,
                       const std::string &description,
                       const std::string &headDoctorID);

            /**
             * @brief Destructor
             */
            ~Department() = default;

            // ==================== Getters ====================

            /**
             * @brief Get department's unique ID
             * @return Department ID string
             */
            std::string getDepartmentID() const;

            /**
             * @brief Get department name
             * @return Department name string
             */
            std::string getName() const;

            /**
             * @brief Get department description
             * @return Description string
             */
            std::string getDescription() const;

            /**
             * @brief Get head doctor's ID
             * @return Head doctor ID string
             */
            std::string getHeadDoctorID() const;

            /**
             * @brief Get list of assigned doctor IDs
             * @return Vector of doctor ID strings
             */
            std::vector<std::string> getDoctorIDs() const;

            /**
             * @brief Get department location
             * @return Location string
             */
            std::string getLocation() const;

            /**
             * @brief Get department phone number
             * @return Phone number string
             */
            std::string getPhone() const;

            // ==================== Setters ====================

            /**
             * @brief Set department name
             * @param name New department name
             */
            void setName(const std::string &name);

            /**
             * @brief Set department description
             * @param description New description
             */
            void setDescription(const std::string &description);

            /**
             * @brief Set head doctor ID
             * @param headDoctorID New head doctor's ID
             */
            void setHeadDoctorID(const std::string &headDoctorID);

            /**
             * @brief Set department location
             * @param location New location
             */
            void setLocation(const std::string &location);

            /**
             * @brief Set department phone number
             * @param phone New phone number (validated)
             */
            void setPhone(const std::string &phone);

            // ==================== Doctor Management ====================

            /**
             * @brief Add a doctor to the department
             * @param doctorID Doctor ID to add
             * @note Duplicate IDs are ignored
             */
            void addDoctor(const std::string &doctorID);

            /**
             * @brief Remove a doctor from the department
             * @param doctorID Doctor ID to remove
             */
            void removeDoctor(const std::string &doctorID);

            /**
             * @brief Check if a doctor is assigned to this department
             * @param doctorID Doctor ID to check
             * @return True if doctor is assigned
             */
            bool hasDoctor(const std::string &doctorID) const;

            /**
             * @brief Get the number of doctors in the department
             * @return Number of assigned doctors
             */
            size_t getDoctorCount() const;

            // ==================== Serialization ====================

            /**
             * @brief Display department information to console
             */
            void displayInfo() const;

            /**
             * @brief Serialize department to string for file storage
             * @return Pipe-delimited string representation
             *
             * Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
             * @note doctorIDs is a comma-separated list
             */
            std::string serialize() const;

            /**
             * @brief Deserialize department from string
             * @param line Pipe-delimited string from file
             * @return Department object or nullopt if parsing fails
             */
            static Result<Department> deserialize(const std::string &line);
        };

    } // namespace Model
} // namespace HMS
