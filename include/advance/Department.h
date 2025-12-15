#pragma once

/**
 * @file Department.h
 * @brief Placeholder for future Department feature
 *
 * This file is a placeholder for the Department management feature
 * planned for future implementation. When implemented, this will
 * allow organizing doctors into departments.
 *
 * Planned features:
 * - Department CRUD operations
 * - Assign doctors to departments
 * - Department head management
 * - Department-based statistics
 */

#include <string>
#include <vector>
#include "../common/Types.h"

namespace HMS {
namespace Model {

/**
 * @class Department
 * @brief Represents a hospital department (FUTURE IMPLEMENTATION)
 *
 * @note This is a placeholder. Implementation pending.
 */
class Department {
private:
    std::string m_departmentID;
    std::string m_name;
    std::string m_description;
    std::string m_headDoctorID;
    std::vector<std::string> m_doctorIDs;
    std::string m_location;
    std::string m_phone;

public:
    // ==================== Constructors ====================

    Department() = default;

    Department(const std::string& departmentID,
               const std::string& name,
               const std::string& description,
               const std::string& headDoctorID);

    ~Department() = default;

    // ==================== Getters ====================

    std::string getDepartmentID() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getHeadDoctorID() const;
    std::vector<std::string> getDoctorIDs() const;
    std::string getLocation() const;
    std::string getPhone() const;

    // ==================== Setters ====================

    void setName(const std::string& name);
    void setDescription(const std::string& description);
    void setHeadDoctorID(const std::string& headDoctorID);
    void setLocation(const std::string& location);
    void setPhone(const std::string& phone);

    // ==================== Doctor Management ====================

    void addDoctor(const std::string& doctorID);
    void removeDoctor(const std::string& doctorID);
    bool hasDoctor(const std::string& doctorID) const;
    size_t getDoctorCount() const;

    // ==================== Serialization ====================

    void displayInfo() const;
    std::string serialize() const;
    static Result<Department> deserialize(const std::string& line);
};

} // namespace Model
} // namespace HMS

/**
 * @note Future files to create when implementing Department feature:
 *
 * Data Access Layer:
 * - include/dal/DepartmentRepository.h
 * - src/dal/DepartmentRepository.cpp
 *
 * Business Logic Layer:
 * - include/bll/DepartmentService.h
 * - src/bll/DepartmentService.cpp
 *
 * Data File:
 * - data/Department.txt
 *
 * Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
 * Example: DEP001|Cardiology|Heart and cardiovascular care|D001|D001,D005,D012|Building A, Floor 2|0281234567
 */
