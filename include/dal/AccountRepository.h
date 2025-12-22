#pragma once

#include "IRepository.h"
#include "../model/Account.h"
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
         * @class AccountRepository
         * @brief Repository for Account entity persistence
         *
         * Implements Singleton pattern to ensure single point of
         * access to account data. Handles CRUD operations and
         * file persistence for Account entities.
         */
        class AccountRepository : public IRepository<Model::Account>
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<AccountRepository> s_instance;
            static std::mutex s_mutex;

            // ==================== Data ====================
            mutable std::mutex m_dataMutex;
            std::vector<Model::Account> m_accounts;
            std::string m_filePath;
            mutable bool m_isLoaded;

            // ==================== Private Constructor ====================
            AccountRepository();

        public:
            // ==================== Singleton Access ====================

            /**
             * @brief Get the singleton instance
             * @return Pointer to the singleton instance
             */
            static AccountRepository *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             * Memory is automatically freed when program exits
             */
            static void resetInstance();

            /**
             * @brief Delete copy constructor
             */
            AccountRepository(const AccountRepository &) = delete;

            /**
             * @brief Delete assignment operator
             */
            AccountRepository &operator=(const AccountRepository &) = delete;

            /**
             * @brief Destructor
             */
            ~AccountRepository() override;

            // ==================== CRUD Operations ====================

            /**
             * @brief Get all accounts
             * @return Vector of all accounts
             */
            std::vector<Model::Account> getAll() override;

            /**
             * @brief Get account by username
             * @param id Username (used as ID)
             * @return Account if found, nullopt otherwise
             */
            std::optional<Model::Account> getById(const std::string &id) override;

            /**
             * @brief Get account by username (alias for getById)
             * @param username The username to search for
             * @return Account if found, nullopt otherwise
             */
            std::optional<Model::Account> getByUsername(const std::string &username);

            /**
             * @brief Add a new account
             * @param account The account to add
             * @return True if successful (username must be unique)
             */
            bool add(const Model::Account &account) override;

            /**
             * @brief Update an existing account
             * @param account The account with updated values
             * @return True if successful
             */
            bool update(const Model::Account &account) override;

            /**
             * @brief Remove an account by username
             * @param id Username to remove
             * @return True if successful
             */
            bool remove(const std::string &id) override;

            // ==================== Persistence ====================

            /**
             * @brief Save all accounts to file
             * @return True if successful
             */
            bool save() override;

            /**
             * @brief Load all accounts from file
             * @return True if successful
             */
            bool load() override;

            // ==================== Query Operations ====================

            /**
             * @brief Get total number of accounts
             * @return Account count
             */
            size_t count() const override;

            /**
             * @brief Check if username exists
             * @param id Username to check
             * @return True if exists
             */
            bool exists(const std::string &id) const override;

            /**
             * @brief Clear all accounts from memory
             * @return True if successful
             */
            bool clear() override;

            // ==================== Account-Specific Queries ====================

            /**
             * @brief Get all accounts with a specific role
             * @param role The role to filter by
             * @return Vector of matching accounts
             */
            std::vector<Model::Account> getByRole(Role role);

            /**
             * @brief Get all active accounts
             * @return Vector of active accounts
             */
            std::vector<Model::Account> getActiveAccounts();

            /**
             * @brief Validate login credentials
             * @param username The username
             * @param passwordHash The hashed password
             * @return True if credentials are valid
             */
            bool validateCredentials(const std::string &username,
                                     const std::string &passwordHash);

            // ==================== File Path ====================

            /**
             * @brief Set the file path for persistence
             * @param filePath Path to the account file
             */
            void setFilePath(const std::string &filePath);

            /**
             * @brief Get the current file path
             * @return File path string
             */
            std::string getFilePath() const;

        private:
            // ==================== Private Helpers ====================

            /**
             * @brief Ensure data is loaded (lazy loading)
             */
            void ensureLoaded() const;

            /**
             * @brief Internal load without mutex (called from locked context)
             */
            bool loadInternal();

            /**
             * @brief Internal save without mutex (called from locked context)
             */
            bool saveInternal();
        };

    } // namespace DAL
} // namespace HMS
