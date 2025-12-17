#pragma once

#include "../dal/AccountRepository.h"
#include "../model/Account.h"
#include "../common/Types.h"
#include <string>
#include <optional>
#include <mutex>

namespace HMS {
namespace BLL {

/**
 * @class AuthService
 * @brief Service for authentication and account management
 *
 * Implements Singleton pattern. Handles user authentication,
 * session management, and account operations.
 */
class AuthService {
private:
    // ==================== Singleton ====================
    static std::unique_ptr<AuthService> s_instance;
    static std::mutex s_mutex;

    // ==================== Dependencies ====================
    DAL::AccountRepository* m_accountRepo;

    // ==================== Session State ====================
    std::string m_currentUsername;
    Role m_currentRole;
    bool m_isLoggedIn;

    // ==================== Private Constructor ====================
    AuthService();

public:
    // ==================== Singleton Access ====================

    /**
     * @brief Get the singleton instance
     * @return Pointer to the singleton instance
     */
    static AuthService* getInstance();

    /**
     * @brief Reset the singleton instance (optional, for testing)
     * Memory is automatically freed when program exits
     */
    static void resetInstance();

    /**
     * @brief Delete copy constructor
     */
    AuthService(const AuthService&) = delete;

    /**
     * @brief Delete assignment operator
     */
    AuthService& operator=(const AuthService&) = delete;

    /**
     * @brief Destructor
     */
    ~AuthService();

    // ==================== Authentication ====================

    /**
     * @brief Authenticate user with credentials
     * @param username The username
     * @param password The plain text password
     * @return True if login successful
     */
    bool login(const std::string& username, const std::string& password);

    /**
     * @brief Log out the current user
     */
    void logout();

    /**
     * @brief Check if a user is currently logged in
     * @return True if logged in
     */
    bool isLoggedIn() const;

    // ==================== Session Info ====================

    /**
     * @brief Get the current logged-in username
     * @return Username or empty string if not logged in
     */
    std::string getCurrentUsername() const;

    /**
     * @brief Get the current user's role
     * @return Role enum value
     */
    Role getCurrentRole() const;

    /**
     * @brief Get the current user's role as string
     * @return Role string
     */
    std::string getCurrentRoleString() const;

    /**
     * @brief Get the current user's account
     * @return Account if logged in, nullopt otherwise
     */
    std::optional<Model::Account> getCurrentAccount() const;

    // ==================== Account Management ====================

    /**
     * @brief Register a new account
     * @param username The username
     * @param password The plain text password
     * @param role The user role
     * @return True if registration successful
     */
    bool registerAccount(const std::string& username,
                         const std::string& password,
                         Role role);

    /**
     * @brief Change password for current user
     * @param oldPassword The current password
     * @param newPassword The new password
     * @return True if password changed successfully
     */
    bool changePassword(const std::string& oldPassword,
                        const std::string& newPassword);

    /**
     * @brief Change password for a specific user (admin only)
     * @param username The target username
     * @param newPassword The new password
     * @return True if password changed successfully
     */
    bool resetPassword(const std::string& username,
                       const std::string& newPassword);

    /**
     * @brief Deactivate an account (admin only)
     * @param username The target username
     * @return True if account deactivated
     */
    bool deactivateAccount(const std::string& username);

    /**
     * @brief Activate an account (admin only)
     * @param username The target username
     * @return True if account activated
     */
    bool activateAccount(const std::string& username);

    // ==================== Validation ====================

    /**
     * @brief Check if username is available
     * @param username The username to check
     * @return True if available
     */
    bool isUsernameAvailable(const std::string& username);

    /**
     * @brief Validate password strength
     * @param password The password to validate
     * @return True if password meets requirements
     */
    bool validatePassword(const std::string& password);

    /**
     * @brief Validate username format
     * @param username The username to validate
     * @return True if username meets requirements
     */
    bool validateUsername(const std::string& username);

    // ==================== Authorization ====================

    /**
     * @brief Check if current user has patient role
     * @return True if patient
     */
    bool isPatient() const;

    /**
     * @brief Check if current user has doctor role
     * @return True if doctor
     */
    bool isDoctor() const;

    /**
     * @brief Check if current user has admin role
     * @return True if admin
     */
    bool isAdmin() const;

    /**
     * @brief Check if current user can perform admin actions
     * @return True if authorized
     */
    bool canPerformAdminActions() const;

    // ==================== Data Persistence ====================

    /**
     * @brief Save account data
     * @return True if successful
     */
    bool saveData();

    /**
     * @brief Load account data
     * @return True if successful
     */
    bool loadData();

private:
    // ==================== Helper Methods ====================

    /**
     * @brief Hash a password
     * @param password Plain text password
     * @return Hashed password
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Stored hash
     * @return True if matches
     */
    bool verifyPassword(const std::string& password, const std::string& hash);
};

} // namespace BLL
} // namespace HMS
