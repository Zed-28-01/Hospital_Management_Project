#pragma once

#include <vector>
#include <optional>
#include <string>

namespace HMS {
namespace DAL {

/**
 * @interface IRepository
 * @brief Generic repository interface for data access operations
 *
 * Defines the contract for all repository implementations.
 * Uses template to support different entity types.
 *
 * @tparam T The entity type (Patient, Doctor, Account, Appointment)
 */
template<typename T>
class IRepository {
public:
    /**
     * @brief Virtual destructor for proper inheritance
     */
    virtual ~IRepository() = default;

    // ==================== CRUD Operations ====================

    /**
     * @brief Get all entities from the repository
     * @return Vector of all entities
     */
    virtual std::vector<T> getAll() = 0;

    /**
     * @brief Get entity by its unique identifier
     * @param id The unique identifier
     * @return Entity if found, nullopt otherwise
     */
    virtual std::optional<T> getById(const std::string& id) = 0;

    /**
     * @brief Add a new entity to the repository
     * @param entity The entity to add
     * @return True if successful, false otherwise
     */
    virtual bool add(const T& entity) = 0;

    /**
     * @brief Update an existing entity
     * @param entity The entity with updated values
     * @return True if successful, false otherwise
     */
    virtual bool update(const T& entity) = 0;

    /**
     * @brief Remove an entity by its identifier
     * @param id The unique identifier
     * @return True if successful, false otherwise
     */
    virtual bool remove(const std::string& id) = 0;

    // ==================== Persistence Operations ====================

    /**
     * @brief Save all entities to persistent storage
     * @return True if successful, false otherwise
     */
    virtual bool save() = 0;

    /**
     * @brief Load all entities from persistent storage
     * @return True if successful, false otherwise
     */
    virtual bool load() = 0;

    // ==================== Query Operations ====================

    /**
     * @brief Get the total count of entities
     * @return Number of entities
     */
    virtual size_t count() const = 0;

    /**
     * @brief Check if an entity with the given ID exists
     * @param id The unique identifier
     * @return True if exists, false otherwise
     */
    virtual bool exists(const std::string& id) const = 0;

    /**
     * @brief Clear all entities from the repository
     * @return True if successful, false otherwise
     */
    virtual bool clear() = 0;
};

} // namespace DAL
} // namespace HMS
