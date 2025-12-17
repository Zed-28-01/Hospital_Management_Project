#include <gtest/gtest.h>

#include "bll/AuthService.h"
#include "common/Types.h"

using namespace HMS;
using namespace HMS::BLL;

// ==================== Test Fixture ====================

class AuthServiceTest : public ::testing::Test {
protected:
    AuthService* authService;

    void SetUp() override {
        // Reset singleton before each test
        AuthService::resetInstance();
        authService = AuthService::getInstance();
        authService->loadData();
    }

    void TearDown() override {
        // Logout and reset after each test
        authService->logout();
        AuthService::resetInstance();
    }
};

// ==================== Singleton Tests ====================

TEST_F(AuthServiceTest, GetInstance_ReturnsSameInstance) {
    AuthService* instance1 = AuthService::getInstance();
    AuthService* instance2 = AuthService::getInstance();

    EXPECT_EQ(instance1, instance2);
}

TEST_F(AuthServiceTest, ResetInstance_ClearsInstance) {
    // Get initial instance
    AuthService* instance1 = AuthService::getInstance();
    EXPECT_NE(instance1, nullptr);

    // Reset should clear the instance
    AuthService::resetInstance();

    // Getting instance again creates a new one
    // We can't directly compare addresses since the old one is deleted
    // Instead, verify the new instance is in initial state
    AuthService* instance2 = AuthService::getInstance();
    EXPECT_NE(instance2, nullptr);
    EXPECT_FALSE(instance2->isLoggedIn());
    EXPECT_EQ(instance2->getCurrentUsername(), "");
}

// ==================== Validation Tests ====================

TEST_F(AuthServiceTest, ValidateUsername_Empty_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername(""));
}

TEST_F(AuthServiceTest, ValidateUsername_Valid_ReturnsTrue) {
    EXPECT_TRUE(authService->validateUsername("validuser"));
}

TEST_F(AuthServiceTest, ValidateUsername_WithPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername("user|name"));
}

TEST_F(AuthServiceTest, ValidateUsername_WithMultiplePipes_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername("user|name|test"));
}

TEST_F(AuthServiceTest, ValidatePassword_TooShort_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("12345"));
}

TEST_F(AuthServiceTest, ValidatePassword_ExactlyMinLength_ReturnsTrue) {
    EXPECT_TRUE(authService->validatePassword("123456"));
}

TEST_F(AuthServiceTest, ValidatePassword_Valid_ReturnsTrue) {
    EXPECT_TRUE(authService->validatePassword("validpassword123"));
}

TEST_F(AuthServiceTest, ValidatePassword_WithPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("pass|word"));
}

TEST_F(AuthServiceTest, ValidatePassword_WithMultiplePipes_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("pass|word|123"));
}

// ==================== Initial State Tests ====================

TEST_F(AuthServiceTest, InitialState_NotLoggedIn) {
    EXPECT_FALSE(authService->isLoggedIn());
}

TEST_F(AuthServiceTest, InitialState_EmptyUsername) {
    EXPECT_EQ(authService->getCurrentUsername(), "");
}

TEST_F(AuthServiceTest, InitialState_UnknownRole) {
    EXPECT_EQ(authService->getCurrentRole(), Role::UNKNOWN);
}

TEST_F(AuthServiceTest, InitialState_RoleStringIsUnknown) {
    EXPECT_EQ(authService->getCurrentRoleString(), "unknown");
}

// ==================== Authorization Role Check Tests ====================

TEST_F(AuthServiceTest, IsPatient_WhenNotLoggedIn_ReturnsFalse) {
    EXPECT_FALSE(authService->isPatient());
}

TEST_F(AuthServiceTest, IsDoctor_WhenNotLoggedIn_ReturnsFalse) {
    EXPECT_FALSE(authService->isDoctor());
}

TEST_F(AuthServiceTest, IsAdmin_WhenNotLoggedIn_ReturnsFalse) {
    EXPECT_FALSE(authService->isAdmin());
}

TEST_F(AuthServiceTest, CanPerformAdminActions_WhenNotLoggedIn_ReturnsFalse) {
    EXPECT_FALSE(authService->canPerformAdminActions());
}

// ==================== Logout Tests ====================

TEST_F(AuthServiceTest, Logout_ClearsSession) {
    // Even if not logged in, logout should work without error
    authService->logout();

    EXPECT_FALSE(authService->isLoggedIn());
    EXPECT_EQ(authService->getCurrentUsername(), "");
    EXPECT_EQ(authService->getCurrentRole(), Role::UNKNOWN);
}

// ==================== GetCurrentAccount Tests ====================

TEST_F(AuthServiceTest, GetCurrentAccount_WhenNotLoggedIn_ReturnsNullopt) {
    auto account = authService->getCurrentAccount();
    EXPECT_FALSE(account.has_value());
}

// ==================== ChangePassword Tests ====================

TEST_F(AuthServiceTest, ChangePassword_WhenNotLoggedIn_ReturnsFalse) {
    EXPECT_FALSE(authService->changePassword("oldpass", "newpassword"));
}

// ==================== ResetPassword Tests ====================

TEST_F(AuthServiceTest, ResetPassword_WhenNotAdmin_ReturnsFalse) {
    // Not logged in, so not admin
    EXPECT_FALSE(authService->resetPassword("someuser", "newpassword"));
}

// ==================== DeactivateAccount Tests ====================

TEST_F(AuthServiceTest, DeactivateAccount_WhenNotAdmin_ReturnsFalse) {
    // Not logged in, so not admin
    EXPECT_FALSE(authService->deactivateAccount("someuser"));
}

// ==================== ActivateAccount Tests ====================

TEST_F(AuthServiceTest, ActivateAccount_WhenNotAdmin_ReturnsFalse) {
    // Not logged in, so not admin
    EXPECT_FALSE(authService->activateAccount("someuser"));
}

// ==================== RegisterAccount Validation Tests ====================

TEST_F(AuthServiceTest, RegisterAccount_InvalidUsername_ReturnsFalse) {
    EXPECT_FALSE(authService->registerAccount("", "validpass123", Role::PATIENT));
}

TEST_F(AuthServiceTest, RegisterAccount_InvalidPassword_ReturnsFalse) {
    EXPECT_FALSE(authService->registerAccount("validuser", "short", Role::PATIENT));
}

TEST_F(AuthServiceTest, RegisterAccount_UsernameWithPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->registerAccount("user|name", "validpass123", Role::PATIENT));
}

TEST_F(AuthServiceTest, RegisterAccount_PasswordWithPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->registerAccount("validuser", "pass|word123", Role::PATIENT));
}

// ==================== Edge Case Tests ====================

TEST_F(AuthServiceTest, ValidateUsername_OnlyPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername("|"));
}

TEST_F(AuthServiceTest, ValidatePassword_OnlyPipe_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("|"));
}

TEST_F(AuthServiceTest, ValidateUsername_PipeAtStart_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername("|username"));
}

TEST_F(AuthServiceTest, ValidateUsername_PipeAtEnd_ReturnsFalse) {
    EXPECT_FALSE(authService->validateUsername("username|"));
}

TEST_F(AuthServiceTest, ValidatePassword_PipeAtStart_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("|password123"));
}

TEST_F(AuthServiceTest, ValidatePassword_PipeAtEnd_ReturnsFalse) {
    EXPECT_FALSE(authService->validatePassword("password123|"));
}

// ==================== Long Input Tests ====================

TEST_F(AuthServiceTest, ValidateUsername_LongUsername_ReturnsTrue) {
    std::string longUsername(100, 'a');
    EXPECT_TRUE(authService->validateUsername(longUsername));
}

TEST_F(AuthServiceTest, ValidatePassword_LongPassword_ReturnsTrue) {
    std::string longPassword(100, 'a');
    EXPECT_TRUE(authService->validatePassword(longPassword));
}

// ==================== Special Character Tests ====================

TEST_F(AuthServiceTest, ValidateUsername_WithSpaces_ReturnsTrue) {
    // Spaces are allowed (only pipe is forbidden)
    EXPECT_TRUE(authService->validateUsername("user name"));
}

TEST_F(AuthServiceTest, ValidatePassword_WithSpecialChars_ReturnsTrue) {
    // Special chars except pipe are allowed
    EXPECT_TRUE(authService->validatePassword("p@ss!w0rd#$%"));
}

TEST_F(AuthServiceTest, ValidateUsername_WithNumbers_ReturnsTrue) {
    EXPECT_TRUE(authService->validateUsername("user123"));
}

/*  
# Build and run all tests
cd /workspaces/Hospital_Management_Project/build
cmake .. && cmake --build . --target HospitalTests
./HospitalTests

# Run only AuthService tests
./HospitalTests --gtest_filter="AuthServiceTest*"

# Run with verbose output
./HospitalTests --gtest_filter="AuthServiceTest*" --gtest_output=xml:test_results.xml
*/