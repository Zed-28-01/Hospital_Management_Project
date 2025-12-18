#include "bll/AuthService.h"
#include "common/Types.h"
#include "common/Utils.h"

namespace HMS {
namespace BLL {

std::unique_ptr<AuthService> AuthService::s_instance = nullptr;
std::mutex AuthService::s_mutex;

// ==================== Constructor ====================

AuthService::AuthService()
    : m_currentUsername(""),
      m_currentRole(Role::UNKNOWN),
      m_isLoggedIn(false)
{
    m_accountRepo = DAL::AccountRepository::getInstance();
}
// ==================== Singleton ====================

AuthService* AuthService::getInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = std::unique_ptr<AuthService>(new AuthService());
    }
    return s_instance.get();
}

void AuthService::resetInstance() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

AuthService::~AuthService() = default;

// ==================== Authentication ====================

bool AuthService::login(const std::string& username,
                        const std::string& password) {
    std::lock_guard<std::mutex> lock(s_mutex);

    std::string hash = hashPassword(password);
    if (!m_accountRepo->validateCredentials(username, hash))
        return false;

    auto acc = m_accountRepo->getByUsername(username);
    if (!acc) return false;

    // Check if account is active
    if (!acc->isActive()) return false;

    m_currentUsername = username;
    m_currentRole = acc->getRole();
    m_isLoggedIn = true;
    return true;
}

void AuthService::logout() {
    std::lock_guard<std::mutex> lock(s_mutex);

    m_isLoggedIn = false;
    m_currentUsername.clear();
    m_currentRole = Role::UNKNOWN;
}

bool AuthService::isLoggedIn() const {
    return m_isLoggedIn;
}

// ==================== Session ====================

std::string AuthService::getCurrentUsername() const {
    return m_currentUsername;
}

Role AuthService::getCurrentRole() const {
    return m_currentRole;
}

std::string AuthService::getCurrentRoleString() const {
    return roleToString(m_currentRole);
}

std::optional<Model::Account>
AuthService::getCurrentAccount() const {
    if (!m_isLoggedIn) return std::nullopt;
    return m_accountRepo->getByUsername(m_currentUsername);
}

// ==================== Account Management ====================

bool AuthService::registerAccount(const std::string& username,
                                  const std::string& password,
                                  Role role) {
    // Validate format first (cheap operations)
    if (!validateUsername(username)) return false;
    if (!validatePassword(password)) return false;

    // Then check availability (requires DB lookup)
    if (!isUsernameAvailable(username)) return false;

    Model::Account acc(
        username,
        hashPassword(password),
        role,
        true,
        Utils::getCurrentDate()
    );

    return m_accountRepo->add(acc);
}

bool AuthService::changePassword(const std::string& oldPassword,
                                 const std::string& newPassword) {
    if (!m_isLoggedIn) return false;
    if (!validatePassword(newPassword)) return false;

    auto accOpt = getCurrentAccount();
    if (!accOpt) return false;

    if (!verifyPassword(oldPassword, accOpt->getPasswordHash()))
        return false;

    accOpt->setPasswordHash(hashPassword(newPassword));
    return m_accountRepo->update(*accOpt);
}

bool AuthService::resetPassword(const std::string& username,
                                const std::string& newPassword) {
    if (!isAdmin()) return false;
    if (!validatePassword(newPassword)) return false;

    auto accOpt = m_accountRepo->getByUsername(username);
    if (!accOpt) return false;

    accOpt->setPasswordHash(hashPassword(newPassword));
    return m_accountRepo->update(*accOpt);
}

bool AuthService::deactivateAccount(const std::string& username) {
    if (!isAdmin()) return false;

    auto accOpt = m_accountRepo->getByUsername(username);
    if (!accOpt) return false;

    accOpt->setActive(false);
    return m_accountRepo->update(*accOpt);
}

bool AuthService::activateAccount(const std::string& username) {
    if (!isAdmin()) return false;

    auto accOpt = m_accountRepo->getByUsername(username);
    if (!accOpt) return false;

    accOpt->setActive(true);
    return m_accountRepo->update(*accOpt);
}

// ==================== Validation ====================

bool AuthService::isUsernameAvailable(const std::string& username) {
    return !m_accountRepo->exists(username);
}

bool AuthService::validatePassword(const std::string& password) {
    return Utils::isValidPassword(password);
}

bool AuthService::validateUsername(const std::string& username) {
    return Utils::isValidUsername(username);
}

// ==================== Authorization ====================

bool AuthService::isPatient() const {
    return m_currentRole == Role::PATIENT;
}

bool AuthService::isDoctor() const {
    return m_currentRole == Role::DOCTOR;
}

bool AuthService::isAdmin() const {
    return m_currentRole == Role::ADMIN;
}

bool AuthService::canPerformAdminActions() const {
    return isAdmin();
}

// ==================== Persistence ====================

bool AuthService::saveData() {
    return m_accountRepo->save();
}

bool AuthService::loadData() {
    return m_accountRepo->load();
}

// ==================== Helpers ====================

std::string AuthService::hashPassword(const std::string& password) {
    // demo hash (sẽ thay bằng bcrypt / sha256 sau)
    return "HASH_" + password;
}

bool AuthService::verifyPassword(const std::string& password,
                                 const std::string& hash) {
    return hashPassword(password) == hash;
}

} // namespace BLL
} // namespace HMS
