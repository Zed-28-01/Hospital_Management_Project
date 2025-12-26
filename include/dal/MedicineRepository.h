#pragma once

#include "IRepository.h"
#include "../advance/Medicine.h"
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>

namespace HMS
{
    namespace DAL
    {

        /**
         * @class MedicineRepository
         * @brief Repository for Medicine entity persistence
         *
         * Implements Singleton pattern. Handles CRUD operations
         * and file persistence for Medicine entities.
         */
        class MedicineRepository : public IRepository<Model::Medicine>
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<MedicineRepository> s_instance;
            static std::mutex s_mutex;

            // ==================== Data ====================
            std::vector<Model::Medicine> m_medicines;
            std::string m_filePath;
            bool m_isLoaded;
            mutable std::mutex m_dataMutex;

            // ==================== Private Constructor ====================
            MedicineRepository();

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
            static MedicineRepository *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            MedicineRepository(const MedicineRepository &) = delete;

            /**
             * @brief Delete assignment operator
             */
            MedicineRepository &operator=(const MedicineRepository &) = delete;

            /**
             * @brief Destructor
             */
            ~MedicineRepository() override;

            // ==================== CRUD Operations ====================

            /**
             * @brief Get all medicines
             * @return Vector of all medicines
             */
            std::vector<Model::Medicine> getAll() override;

            /**
             * @brief Get medicine by medicine ID
             * @param id Medicine ID
             * @return Medicine if found, nullopt otherwise
             */
            std::optional<Model::Medicine> getById(const std::string &id) override;

            /**
             * @brief Add a new medicine
             * @param medicine The medicine to add
             * @return True if successful
             */
            bool add(const Model::Medicine &medicine) override;

            /**
             * @brief Update an existing medicine
             * @param medicine The medicine with updated values
             * @return True if successful
             */
            bool update(const Model::Medicine &medicine) override;

            /**
             * @brief Remove a medicine by ID
             * @param id Medicine ID to remove
             * @return True if successful
             */
            bool remove(const std::string &id) override;

            // ==================== Persistence ====================

            /**
             * @brief Save all medicines to file
             * @return True if successful
             */
            bool save() override;

            /**
             * @brief Load all medicines from file
             * @return True if successful
             */
            bool load() override;

            // ==================== Query Operations ====================

            /**
             * @brief Get total number of medicines
             * @return Medicine count
             */
            size_t count() const override;

            /**
             * @brief Check if medicine ID exists
             * @param id Medicine ID to check
             * @return True if exists
             */
            bool exists(const std::string &id) const override;

            /**
             * @brief Clear all medicines from memory
             * @return True if successful
             */
            bool clear() override;

            // ==================== Medicine-Specific Queries ====================

            /**
             * @brief Get medicines by category
             * @param category The category name
             * @return Vector of medicines in that category
             */
            std::vector<Model::Medicine> getByCategory(const std::string &category);

            /**
             * @brief Get all medicines with low stock (quantity <= reorderLevel)
             * @return Vector of low stock medicines
             */
            std::vector<Model::Medicine> getLowStock();

            /**
             * @brief Get all expired medicines
             * @return Vector of expired medicines
             */
            std::vector<Model::Medicine> getExpired();

            /**
             * @brief Get medicines expiring within specified days
             * @param days Number of days threshold (default: 30)
             * @return Vector of medicines expiring soon
             */
            std::vector<Model::Medicine> getExpiringSoon(int days = 30);

            /**
             * @brief Search medicines by name (partial match)
             * @param name Name to search for
             * @return Vector of matching medicines
             */
            std::vector<Model::Medicine> searchByName(const std::string &name);

            /**
             * @brief Search medicines by any keyword
             * @param keyword Keyword to search in name, generic name, category, manufacturer
             * @return Vector of matching medicines
             */
            std::vector<Model::Medicine> search(const std::string &keyword);

            /**
             * @brief Get all unique categories
             * @return Vector of category names
             */
            std::vector<std::string> getAllCategories();

            /**
             * @brief Get all unique manufacturers
             * @return Vector of manufacturer names
             */
            std::vector<std::string> getAllManufacturers();

            /**
             * @brief Get the next available medicine ID
             * @return New medicine ID string (e.g., "MED001")
             */
            std::string getNextId();

            // ==================== Stock Operations ====================

            /**
             * @brief Update stock quantity for a medicine
             * @param id Medicine ID
             * @param quantity New quantity (absolute value)
             * @return True if successful
             */
            bool updateStock(const std::string &id, int quantity);

            // ==================== File Path ====================

            /**
             * @brief Set the file path for persistence
             * @param filePath Path to the medicine file
             */
            void setFilePath(const std::string &filePath);

            /**
             * @brief Get the current file path
             * @return File path string
             */
            std::string getFilePath() const;
        };

    } // namespace DAL
} // namespace HMS
