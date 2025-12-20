#include "dal/AccountRepository.h"
#include "dal/FileHelper.h"
#include "common/Types.h"
#include "common/Constants.h"

#include <algorithm>
#include <sstream>

namespace HMS
{
    namespace DAL
    {

        std::unique_ptr<AccountRepository> AccountRepository::s_instance = nullptr;
        std::mutex AccountRepository::s_mutex;

        // ==================== Constructor ====================

        AccountRepository::AccountRepository()
            : m_filePath(Constants::ACCOUNT_FILE),
              m_isLoaded(false) {}

        // ==================== Destructor ====================

        AccountRepository::~AccountRepository() = default;

        // ==================== Singleton ====================

        AccountRepository *AccountRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<AccountRepository>(new AccountRepository());
            }
            return s_instance.get();
        }

        void AccountRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Private Helper ====================

        void AccountRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<AccountRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD ====================

        std::vector<Model::Account> AccountRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_accounts;
        }

        std::optional<Model::Account> AccountRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            for (const auto &acc : m_accounts)
            {
                if (acc.getUsername() == id)
                    return acc;
            }
            return std::nullopt;
        }

        std::optional<Model::Account>
        AccountRepository::getByUsername(const std::string &username)
        {
            return getById(username);
        }

        bool AccountRepository::add(const Model::Account &account)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if already exists
            for (const auto &acc : m_accounts)
            {
                if (acc.getUsername() == account.getUsername())
                    return false;
            }

            m_accounts.push_back(account);
            return saveInternal();
        }

        bool AccountRepository::update(const Model::Account &account)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            for (auto &acc : m_accounts)
            {
                if (acc.getUsername() == account.getUsername())
                {
                    acc = account;
                    return saveInternal();
                }
            }
            return false;
        }

        bool AccountRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::remove_if(
                m_accounts.begin(), m_accounts.end(),
                [&](const Model::Account &acc)
                {
                    return acc.getUsername() == id;
                });

            if (it == m_accounts.end())
                return false;
            m_accounts.erase(it, m_accounts.end());
            return saveInternal();
        }

        // ==================== Persistence ====================

        bool AccountRepository::load()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool AccountRepository::loadInternal()
        {
            try
            {
                FileHelper::createDirectoryIfNotExists(Constants::DATA_DIR);
                FileHelper::createFileIfNotExists(m_filePath);

                std::vector<std::string> lines = FileHelper::readLines(m_filePath);

                m_accounts.clear();

                for (const auto &line : lines)
                {
                    auto account = Model::Account::deserialize(line);
                    if (account)
                    {
                        m_accounts.push_back(account.value());
                    }
                }

                m_isLoaded = true;
                return true;
            }
            catch (...)
            {
                m_isLoaded = false;
                return false;
            }
        }

        bool AccountRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool AccountRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Account");
                std::stringstream hss(header);
                std::string headerLine;
                while (std::getline(hss, headerLine))
                {
                    if (!headerLine.empty())
                    {
                        lines.push_back(headerLine);
                    }
                }

                // Add data
                for (const auto &acc : m_accounts)
                {
                    lines.push_back(acc.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        // ==================== Queries ====================

        size_t AccountRepository::count() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_accounts.size();
        }

        bool AccountRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::any_of(
                m_accounts.begin(), m_accounts.end(),
                [&](const Model::Account &acc)
                {
                    return acc.getUsername() == id;
                });
        }

        bool AccountRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_accounts.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Account-specific ====================

        std::vector<Model::Account>
        AccountRepository::getByRole(Role role)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Account> result;
            for (const auto &acc : m_accounts)
            {
                if (acc.getRole() == role)
                    result.push_back(acc);
            }
            return result;
        }

        std::vector<Model::Account>
        AccountRepository::getActiveAccounts()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Account> result;
            for (const auto &acc : m_accounts)
            {
                if (acc.isActive())
                    result.push_back(acc);
            }
            return result;
        }

        bool AccountRepository::validateCredentials(
            const std::string &username,
            const std::string &passwordHash)
        {

            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            for (const auto &acc : m_accounts)
            {
                if (acc.getUsername() == username)
                {
                    return acc.getPasswordHash() == passwordHash && acc.isActive();
                }
            }
            return false;
        }

        // ==================== File Path ====================

        void AccountRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false; // Force reload with new file
        }

        std::string AccountRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS
