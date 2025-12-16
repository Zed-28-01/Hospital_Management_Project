#include "dal/AccountRepository.h"
#include "common/Types.h"
#include <fstream>
#include <algorithm>

namespace HMS {
namespace DAL {

AccountRepository* AccountRepository::s_instance = nullptr;
std::mutex AccountRepository::s_mutex;

// ==================== Constructor ====================

AccountRepository::AccountRepository()
    : m_filePath("data/Account.txt"),
      m_isLoaded(false) {}

// ==================== Destructor ====================

AccountRepository::~AccountRepository() {
    //do nothing
}

// ==================== Singleton ====================

AccountRepository* AccountRepository::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = new AccountRepository();
    }
    return s_instance;
}

// ==================== CRUD ====================

std::vector<Model::Account> AccountRepository::getAll() {
    if (!m_isLoaded) load();
    return m_accounts;
}

std::optional<Model::Account> AccountRepository::getById(const std::string& id) {
    if (!m_isLoaded) load();
    for (const auto& acc : m_accounts) {
        if (acc.getUsername() == id)
            return acc;
    }
    return std::nullopt;
}

std::optional<Model::Account>
AccountRepository::getByUsername(const std::string& username) {
    return getById(username);
}

bool AccountRepository::add(const Model::Account& account) {
    if (exists(account.getUsername()))
        return false;

    m_accounts.push_back(account);
    save();
    return true;
}

bool AccountRepository::update(const Model::Account& account) {
    for (auto& acc : m_accounts) {
        if (acc.getUsername() == account.getUsername()) {
            acc = account;
            save();
            return true;
        }
    }
    return false;
}

bool AccountRepository::remove(const std::string& id) {
    auto it = std::remove_if(
        m_accounts.begin(), m_accounts.end(),
        [&](const Model::Account& acc) {
            return acc.getUsername() == id;
        });

    if (it == m_accounts.end()) return false;
    m_accounts.erase(it, m_accounts.end());
    save();
    return true;
}

// ==================== Persistence ====================

bool AccountRepository::load() {
    m_accounts.clear();
    std::ifstream in(m_filePath);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        auto accOpt = Model::Account::deserialize(line);
        if (accOpt) {
            m_accounts.push_back(*accOpt);
        }
    }

    m_isLoaded = true;
    return true;
}

bool AccountRepository::save() {
    std::ofstream out(m_filePath);
    if (!out.is_open()) return false;

    for (const auto& acc : m_accounts) {
        out << acc.serialize() << "\n";
    }
    return true;
}

// ==================== Queries ====================

size_t AccountRepository::count() const {
    if (!m_isLoaded) {
        const_cast<AccountRepository*>(this)->load();
    }

    return m_accounts.size();
}

bool AccountRepository::exists(const std::string& id) const {
    if (!m_isLoaded) {
        const_cast<AccountRepository*>(this)->load();
    }

    return std::any_of(
        m_accounts.begin(), m_accounts.end(),
        [&](const Model::Account& acc) {
            return acc.getUsername() == id;
        });
}

bool AccountRepository::clear() {
    m_accounts.clear();
    return true;
}

// ==================== Account-specific ====================

std::vector<Model::Account>
AccountRepository::getByRole(Role role) {
    std::vector<Model::Account> result;
    for (const auto& acc : m_accounts) {
        if (acc.getRole() == role)
            result.push_back(acc);
    }
    return result;
}

std::vector<Model::Account>
AccountRepository::getActiveAccounts() {
    std::vector<Model::Account> result;
    for (const auto& acc : m_accounts) {
        if (acc.isActive())
            result.push_back(acc);
    }
    return result;
}

bool AccountRepository::validateCredentials(
    const std::string& username,
    const std::string& passwordHash) {

    auto acc = getById(username);
    return acc && acc->getPasswordHash() == passwordHash && acc->isActive();
}

// ==================== File Path ====================

void AccountRepository::setFilePath(const std::string& filePath) {
    m_filePath = filePath;
}

std::string AccountRepository::getFilePath() const {
    return m_filePath;
}

} // namespace DAL
} // namespace HMS
