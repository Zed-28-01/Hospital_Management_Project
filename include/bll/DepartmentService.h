#pragma once

#include "../dal/DepartmentRepository.h"
#include "../dal/DoctorRepository.h"
#include "../advance/Department.h"
#include "../model/Doctor.h"
#include "../common/Types.h"

#include <string>
#include <mutex>
#include <memory>
#include <map>

namespace HMS {
namespace BLL {

/**
 * @struct DepartmentStats
 * @brief Statistics for a department
 */
struct DepartmentStats {
    std::string departmentID;
    std::string departmentName;
    int doctorCount;
    int appointmentCount;
    double totalRevenue;
    std::vector<std::string> doctorNames;
};

/**
 * @class DepartmentService
 * @brief Service for department-related business logic
 *
 * Implements Singleton pattern. Handles department management,
 * doctor assignments, and department statistics.
 */
class DepartmentService {
private:
    // ==================== Singleton ====================
    static std::unique_ptr<DepartmentService> s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    DAL::DepartmentRepository* m_departmentRepo;
    DAL::DoctorRepository* m_doctorRepo;

    // ==================== Private Constructor ====================
    DepartmentService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static DepartmentService* getInstance();

    /**
     * @brief Reset the singleton instance (for testing)
     */
    static void resetInstance();

    /**
     * @brief Delete copy constructor
     */
    DepartmentService(const DepartmentService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    DepartmentService& operator=(const DepartmentService&) = delete;

    /**
     * @brief Destructor
     */
    ~DepartmentService();

    // ==================== CRUD Operations ====================

    /**
     * @brief Create a new department
     * @param department The department data
     * @return True if successful
     */
    bool createDepartment(const Model::Department& department);

    /**
     * @brief Create department with individual parameters
     * @param name Department name
     * @param description Department description
     * @param location Department location
     * @param phone Department phone
     * @return Created department or nullopt if failed
     */
    Result<Model::Department> createDepartment(
        const std::string& name,
        const std::string& description,
        const std::string& location,
        const std::string& phone);

    /**
     * @brief Update an existing department
     * @param department The department with updated data
     * @return True if successful
     */
    bool updateDepartment(const Model::Department& department);

    /**
     * @brief Delete a department
     * @param departmentID The department's ID
     * @return True if successful
     */
    bool deleteDepartment(const std::string& departmentID);

    // ==================== Query Operations ====================

    /**
     * @brief Get department by ID
     * @param departmentID The department's ID
     * @return Department if found, nullopt otherwise
     */
    Result<Model::Department> getDepartmentByID(const std::string& departmentID);

    /**
     * @brief Get department by name
     * @param name The department name
     * @return Department if found, nullopt otherwise
     */
    Result<Model::Department> getDepartmentByName(const std::string& name);

    /**
     * @brief Get all departments
     * @return Vector of all departments
     */
    List<Model::Department> getAllDepartments();

    /**
     * @brief Search departments by keyword
     * @param keyword Search keyword
     * @return Vector of matching departments
     */
    List<Model::Department> searchDepartments(const std::string& keyword);

    /**
     * @brief Get total department count
     * @return Number of departments
     */
    size_t getDepartmentCount() const;

    /**
     * @brief Get all department names
     * @return Vector of department names
     */
    List<std::string> getAllDepartmentNames();

    // ==================== Doctor Assignment ====================

    /**
     * @brief Assign a doctor to a department
     * @param departmentID The department's ID
     * @param doctorID The doctor's ID
     * @return True if successful
     */
    bool assignDoctor(const std::string& departmentID, const std::string& doctorID);

    /**
     * @brief Remove a doctor from a department
     * @param departmentID The department's ID
     * @param doctorID The doctor's ID
     * @return True if successful
     */
    bool unassignDoctor(const std::string& departmentID, const std::string& doctorID);

    /**
     * @brief Set the department head
     * @param departmentID The department's ID
     * @param doctorID The doctor's ID (must be in department)
     * @return True if successful
     */
    bool setDepartmentHead(const std::string& departmentID, const std::string& doctorID);

    /**
     * @brief Get department head doctor
     * @param departmentID The department's ID
     * @return Doctor if found, nullopt otherwise
     */
    Result<Model::Doctor> getDepartmentHead(const std::string& departmentID);

    /**
     * @brief Check if a doctor is in a department
     * @param departmentID The department's ID
     * @param doctorID The doctor's ID
     * @return True if doctor is in department
     */
    bool isDoctorInDepartment(const std::string& departmentID, const std::string& doctorID);

    // ==================== Doctor Queries ====================

    /**
     * @brief Get all doctors in a department
     * @param departmentID The department's ID
     * @return Vector of doctors in the department
     */
    List<Model::Doctor> getDoctorsInDepartment(const std::string& departmentID);

    /**
     * @brief Get the department a doctor belongs to
     * @param doctorID The doctor's ID
     * @return Department if found, nullopt otherwise
     */
    Result<Model::Department> getDoctorDepartment(const std::string& doctorID);

    /**
     * @brief Get all departments a doctor belongs to
     * @param doctorID The doctor's ID
     * @return Vector of departments
     */
    List<Model::Department> getDoctorDepartments(const std::string& doctorID);

    /**
     * @brief Get doctors without a department
     * @return Vector of unassigned doctors
     */
    List<Model::Doctor> getUnassignedDoctors();

    // ==================== Statistics ====================

    /**
     * @brief Get statistics for a department
     * @param departmentID The department's ID
     * @return Department statistics
     */
    DepartmentStats getDepartmentStats(const std::string& departmentID);

    /**
     * @brief Get statistics for all departments
     * @return Map of department ID to statistics
     */
    std::map<std::string, DepartmentStats> getAllDepartmentStats();

    /**
     * @brief Get doctor count by department
     * @return Map of department ID to doctor count
     */
    std::map<std::string, int> getDoctorCountByDepartment();

    // ==================== Validation ====================

    /**
     * @brief Validate department data
     * @param department The department to validate
     * @return True if valid
     */
    bool validateDepartment(const Model::Department& department);

    /**
     * @brief Check if department ID exists
     * @param departmentID The department ID to check
     * @return True if exists
     */
    bool departmentExists(const std::string& departmentID);

    /**
     * @brief Check if department name already exists
     * @param name The department name to check
     * @param excludeID Department ID to exclude from check (for updates)
     * @return True if name exists
     */
    bool departmentNameExists(const std::string& name, const std::string& excludeID = "");

    // ==================== Data Persistence ====================

    /**
     * @brief Save department data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load department data
     * @return True if successful
     */
    bool loadData();
};

} // namespace BLL
} // namespace HMS
