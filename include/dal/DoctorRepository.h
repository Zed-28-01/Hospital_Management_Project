#pragma once

#include "IRepository.h"
#include "../model/Doctor.h"
#include <vector>
#include <optional>
#include <string>
#include <mutex>

namespace HMS {
namespace DAL {

/**
 * @class DoctorRepository
 * @brief Repository for Doctor entity persistence
 *
 * Implements Singleton pattern. Handles CRUD operations
 * and file persistence for Doctor entities.
 */
class DoctorRepository : public IRepository<Model::Doctor> {
private:
    // ==================== Singleton ====================
    static DoctorRepository* s_instance;
    static std::mutex s_mutex;

    // ==================== Data ====================
    std::vector<Model::Doctor> m_doctors;
    std::string m_filePath;
    bool m_isLoaded;

    // ==================== Private Constructor ====================
    DoctorRepository();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static DoctorRepository* getInstance();

    /**
     * @brief Delete copy constructor
     */
    DoctorRepository(const DoctorRepository&) = delete;

    /**
     * @brief Delete assignment operator
     */
    DoctorRepository& operator=(const DoctorRepository&) = delete;

    /**
     * @brief Destructor
     */
    ~DoctorRepository() override;

    // ==================== CRUD Operations ====================

    /**
     * @brief Get all doctors
     * @return Vector of all doctors
     */
    std::vector<Model::Doctor> getAll() override;

    /**
     * @brief Get doctor by doctor ID
     * @param id Doctor ID
     * @return Doctor if found, nullopt otherwise
     */
    std::optional<Model::Doctor> getById(const std::string& id) override;

    /**
     * @brief Add a new doctor
     * @param doctor The doctor to add
     * @return True if successful
     */
    bool add(const Model::Doctor& doctor) override;

    /**
     * @brief Update an existing doctor
     * @param doctor The doctor with updated values
     * @return True if successful
     */
    bool update(const Model::Doctor& doctor) override;

    /**
     * @brief Remove a doctor by ID
     * @param id Doctor ID to remove
     * @return True if successful
     */
    bool remove(const std::string& id) override;

    // ==================== Persistence ====================

    /**
     * @brief Save all doctors to file
     * @return True if successful
     */
    bool save() override;

    /**
     * @brief Load all doctors from file
     * @return True if successful
     */
    bool load() override;

    // ==================== Query Operations ====================

    /**
     * @brief Get total number of doctors
     * @return Doctor count
     */
    size_t count() const override;

    /**
     * @brief Check if doctor ID exists
     * @param id Doctor ID to check
     * @return True if exists
     */
    bool exists(const std::string& id) const override;

    /**
     * @brief Clear all doctors from memory
     * @return True if successful
     */
    bool clear() override;

    // ==================== Doctor-Specific Queries ====================

    /**
     * @brief Get doctor by username
     * @param username The account username
     * @return Doctor if found, nullopt otherwise
     */
    std::optional<Model::Doctor> getByUsername(const std::string& username);

    /**
     * @brief Get doctors by specialization
     * @param specialization The medical specialization
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> getBySpecialization(const std::string& specialization);

    /**
     * @brief Search doctors by name (partial match)
     * @param name Name to search for
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> searchByName(const std::string& name);

    /**
     * @brief Search doctors by any keyword
     * @param keyword Keyword to search in name, specialization
     * @return Vector of matching doctors
     */
    std::vector<Model::Doctor> search(const std::string& keyword);

    /**
     * @brief Get all unique specializations
     * @return Vector of specialization strings
     */
    std::vector<std::string> getAllSpecializations();

    /**
     * @brief Get the next available doctor ID
     * @return New doctor ID string
     */
    std::string getNextId();

    // ==================== File Path ====================

    /**
     * @brief Set the file path for persistence
     * @param filePath Path to the doctor file
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
