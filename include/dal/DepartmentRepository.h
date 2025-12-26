#pragma once

#include "IRepository.h"
#include "../advance/Department.h"
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>

namespace HMS {
namespace DAL {

/**
 * @class DepartmentRepository
 * @brief Repository for Department entity persistence
 *
 * Implements Singleton pattern. Handles CRUD operations
 * and file persistence for Department entities.
 */
class DepartmentRepository : public IRepository<Model::Department> {
private:
    // ==================== Singleton ====================
    static std::unique_ptr<DepartmentRepository> s_instance;
    static std::mutex s_mutex;

    // ==================== Data ====================
    std::vector<Model::Department> m_departments;
    std::string m_filePath;
    bool m_isLoaded;
    mutable std::mutex m_dataMutex;

    // ==================== Private Constructor ====================
    DepartmentRepository();

    // ==================== Private Helpers ====================

    /**
     * @brief Ensure data is loaded (const-safe helper)
     */
    void ensureLoaded() const;

    /**
     * @brief Internal load implementation (without lock)
     * @return True if successful
     */
    bool loadInternal();

    /**
     * @brief Internal save implementation (without lock)
     * @return True if successful
     */
    bool saveInternal();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static DepartmentRepository* getInstance();

    /**
     * @brief Reset the singleton instance (for testing)
     */
    static void resetInstance();

    /**
     * @brief Delete copy constructor
     */
    DepartmentRepository(const DepartmentRepository&) = delete;

    /**
     * @brief Delete assignment operator
     */
    DepartmentRepository& operator=(const DepartmentRepository&) = delete;

    /**
     * @brief Destructor
     */
    ~DepartmentRepository() override;

    // ==================== CRUD Operations ====================

    /**
     * @brief Get all departments
     * @return Vector of all departments
     */
    std::vector<Model::Department> getAll() override;

    /**
     * @brief Get department by department ID
     * @param id Department ID
     * @return Department if found, nullopt otherwise
     */
    std::optional<Model::Department> getById(const std::string& id) override;

    /**
     * @brief Add a new department
     * @param department The department to add
     * @return True if successful
     */
    bool add(const Model::Department& department) override;

    /**
     * @brief Update an existing department
     * @param department The department with updated values
     * @return True if successful
     */
    bool update(const Model::Department& department) override;

    /**
     * @brief Remove a department by ID
     * @param id Department ID to remove
     * @return True if successful
     */
    bool remove(const std::string& id) override;

    // ==================== Persistence ====================

    /**
     * @brief Save all departments to file
     * @return True if successful
     */
    bool save() override;

    /**
     * @brief Load all departments from file
     * @return True if successful
     */
    bool load() override;

    // ==================== Query Operations ====================

    /**
     * @brief Get total number of departments
     * @return Department count
     */
    size_t count() const override;

    /**
     * @brief Check if department ID exists
     * @param id Department ID to check
     * @return True if exists
     */
    bool exists(const std::string& id) const override;

    /**
     * @brief Clear all departments from memory
     * @return True if successful
     */
    bool clear() override;

    // ==================== Department-Specific Queries ====================

    /**
     * @brief Get department by name
     * @param name The department name
     * @return Department if found, nullopt otherwise
     */
    std::optional<Model::Department> getByName(const std::string& name);

    /**
     * @brief Get department by head doctor ID
     * @param doctorID The head doctor's ID
     * @return Department if found, nullopt otherwise
     */
    std::optional<Model::Department> getByHeadDoctor(const std::string& doctorID);

    /**
     * @brief Get department that contains a specific doctor
     * @param doctorID The doctor's ID
     * @return Department if found, nullopt otherwise
     */
    std::optional<Model::Department> getDepartmentByDoctor(const std::string& doctorID);

    /**
     * @brief Get all departments that a doctor belongs to
     * @param doctorID The doctor's ID
     * @return Vector of departments containing this doctor
     */
    std::vector<Model::Department> getDepartmentsByDoctor(const std::string& doctorID);

    /**
     * @brief Search departments by name (partial match)
     * @param keyword Keyword to search for
     * @return Vector of matching departments
     */
    std::vector<Model::Department> searchByName(const std::string& keyword);

    /**
     * @brief Get all department names
     * @return Vector of department names
     */
    std::vector<std::string> getAllNames();

    /**
     * @brief Get the next available department ID
     * @return New department ID string (e.g., "DEP001")
     */
    std::string getNextId();

    // ==================== File Path ====================

    /**
     * @brief Set the file path for persistence
     * @param filePath Path to the department file
     */
    void setFilePath(const std::string& filePath);

    /**
     * @brief Get the current file path
     * @return File path string
     */
    std::string getFilePath() const;
};

} // namespace DAL
} // namespace HMS
