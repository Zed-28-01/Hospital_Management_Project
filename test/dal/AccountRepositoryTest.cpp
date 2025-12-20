#include <gtest/gtest.h>
#include "dal/AccountRepository.h"
#include "model/Account.h"
#include "common/Types.h"
#include <fstream>
#include <filesystem>

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace {
    const std::string TEST_DATA_DIR = "test/fixtures/";
    const std::string TEST_DATA_FILE = "test/fixtures/AccountTest.txt";
}

class AccountRepositoryTest : public ::testing::Test {
protected:
    AccountRepository* repo;
    std::string testFilePath;

    void SetUp() override {
        // Reset singleton and get fresh instance
        AccountRepository::resetInstance();
        repo = AccountRepository::getInstance();

        // Use fixture file
        testFilePath = TEST_DATA_FILE;
        repo->setFilePath(testFilePath);
        std::filesystem::create_directories(TEST_DATA_DIR);
        std::ofstream ofs(testFilePath, std::ios::trunc);
        ofs.close();
        repo->clear();
    }

    void TearDown() override {
        repo->clear();
        AccountRepository::resetInstance();
    }

    Account createTestAccount(const std::string& username = "testuser",
                               const std::string& password = "hashedpass123",
                               Role role = Role::PATIENT) {
        return Account(username, password, role, true, "2025-01-01");
    }
};

// ==================== Singleton Tests ====================

TEST_F(AccountRepositoryTest, GetInstance_ReturnsSameInstance) {
    AccountRepository* instance1 = AccountRepository::getInstance();
    AccountRepository* instance2 = AccountRepository::getInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(AccountRepositoryTest, ResetInstance_ClearsInstance) {
    AccountRepository* oldInstance = AccountRepository::getInstance();
    oldInstance->clear();

    AccountRepository::resetInstance();
    AccountRepository* newInstance = AccountRepository::getInstance();
    newInstance->setFilePath(TEST_DATA_FILE);  // Restore test file path

    // New instance should be in fresh state
    EXPECT_EQ(newInstance->count(), 0u);
}

// ==================== Add Tests ====================

TEST_F(AccountRepositoryTest, Add_ValidAccount_ReturnsTrue) {
    Account acc = createTestAccount();
    EXPECT_TRUE(repo->add(acc));
}

TEST_F(AccountRepositoryTest, Add_DuplicateUsername_ReturnsFalse) {
    Account acc1 = createTestAccount("user1");
    Account acc2 = createTestAccount("user1");  // Same username

    EXPECT_TRUE(repo->add(acc1));
    EXPECT_FALSE(repo->add(acc2));
}

TEST_F(AccountRepositoryTest, Add_MultipleAccounts_AllAdded) {
    Account acc1 = createTestAccount("user1");
    Account acc2 = createTestAccount("user2");
    Account acc3 = createTestAccount("user3");

    EXPECT_TRUE(repo->add(acc1));
    EXPECT_TRUE(repo->add(acc2));
    EXPECT_TRUE(repo->add(acc3));
    EXPECT_EQ(repo->count(), 3u);
}

// ==================== GetById/GetByUsername Tests ====================

TEST_F(AccountRepositoryTest, GetById_ExistingAccount_ReturnsAccount) {
    Account acc = createTestAccount("findme");
    repo->add(acc);

    auto result = repo->getById("findme");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getUsername(), "findme");
}

TEST_F(AccountRepositoryTest, GetById_NonExistingAccount_ReturnsNullopt) {
    auto result = repo->getById("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AccountRepositoryTest, GetByUsername_ExistingAccount_ReturnsAccount) {
    Account acc = createTestAccount("myuser");
    repo->add(acc);

    auto result = repo->getByUsername("myuser");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getUsername(), "myuser");
}

// ==================== GetAll Tests ====================

TEST_F(AccountRepositoryTest, GetAll_EmptyRepo_ReturnsEmptyVector) {
    auto accounts = repo->getAll();
    EXPECT_TRUE(accounts.empty());
}

TEST_F(AccountRepositoryTest, GetAll_WithAccounts_ReturnsAllAccounts) {
    repo->add(createTestAccount("user1"));
    repo->add(createTestAccount("user2"));

    auto accounts = repo->getAll();
    EXPECT_EQ(accounts.size(), 2u);
}

// ==================== Update Tests ====================

TEST_F(AccountRepositoryTest, Update_ExistingAccount_ReturnsTrue) {
    Account acc = createTestAccount("updateme");
    repo->add(acc);

    Account updated("updateme", "newpasshash", Role::ADMIN, true, "2025-01-01");
    EXPECT_TRUE(repo->update(updated));

    auto result = repo->getById("updateme");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getRole(), Role::ADMIN);
}

TEST_F(AccountRepositoryTest, Update_NonExistingAccount_ReturnsFalse) {
    Account acc("nonexistent", "pass", Role::PATIENT, true, "2025-01-01");
    EXPECT_FALSE(repo->update(acc));
}

TEST_F(AccountRepositoryTest, Update_PasswordChange_Persisted) {
    Account acc = createTestAccount("passchange");
    repo->add(acc);

    Account updated("passchange", "newpassword123", Role::PATIENT, true, "2025-01-01");
    repo->update(updated);

    auto result = repo->getById("passchange");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPasswordHash(), "newpassword123");
}

// ==================== Remove Tests ====================

TEST_F(AccountRepositoryTest, Remove_ExistingAccount_ReturnsTrue) {
    repo->add(createTestAccount("removeme"));
    EXPECT_TRUE(repo->remove("removeme"));
    EXPECT_FALSE(repo->exists("removeme"));
}

TEST_F(AccountRepositoryTest, Remove_NonExistingAccount_ReturnsFalse) {
    EXPECT_FALSE(repo->remove("nonexistent"));
}

TEST_F(AccountRepositoryTest, Remove_DecreasesCount) {
    repo->add(createTestAccount("user1"));
    repo->add(createTestAccount("user2"));
    EXPECT_EQ(repo->count(), 2u);

    repo->remove("user1");
    EXPECT_EQ(repo->count(), 1u);
}

// ==================== Exists Tests ====================

TEST_F(AccountRepositoryTest, Exists_ExistingAccount_ReturnsTrue) {
    repo->add(createTestAccount("iexist"));
    EXPECT_TRUE(repo->exists("iexist"));
}

TEST_F(AccountRepositoryTest, Exists_NonExistingAccount_ReturnsFalse) {
    EXPECT_FALSE(repo->exists("idontexist"));
}

// ==================== Count Tests ====================

TEST_F(AccountRepositoryTest, Count_EmptyRepo_ReturnsZero) {
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(AccountRepositoryTest, Count_AfterAdds_ReturnsCorrectCount) {
    repo->add(createTestAccount("user1"));
    repo->add(createTestAccount("user2"));
    repo->add(createTestAccount("user3"));
    EXPECT_EQ(repo->count(), 3u);
}

// ==================== Clear Tests ====================

TEST_F(AccountRepositoryTest, Clear_RemovesAllAccounts) {
    repo->add(createTestAccount("user1"));
    repo->add(createTestAccount("user2"));

    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

// ==================== GetByRole Tests ====================

TEST_F(AccountRepositoryTest, GetByRole_ReturnsMatchingAccounts) {
    repo->add(Account("admin1", "pass", Role::ADMIN, true, "2025-01-01"));
    repo->add(Account("user1", "pass", Role::PATIENT, true, "2025-01-01"));
    repo->add(Account("admin2", "pass", Role::ADMIN, true, "2025-01-01"));
    repo->add(Account("doctor1", "pass", Role::DOCTOR, true, "2025-01-01"));

    auto admins = repo->getByRole(Role::ADMIN);
    EXPECT_EQ(admins.size(), 2u);

    auto doctors = repo->getByRole(Role::DOCTOR);
    EXPECT_EQ(doctors.size(), 1u);
}

TEST_F(AccountRepositoryTest, GetByRole_NoMatches_ReturnsEmptyVector) {
    repo->add(Account("user1", "pass", Role::PATIENT, true, "2025-01-01"));

    // Search for DOCTOR role when only PATIENT exists
    auto doctors = repo->getByRole(Role::DOCTOR);
    EXPECT_TRUE(doctors.empty());
}

// ==================== GetActiveAccounts Tests ====================

TEST_F(AccountRepositoryTest, GetActiveAccounts_ReturnsOnlyActive) {
    repo->add(Account("active1", "pass", Role::PATIENT, true, "2025-01-01"));
    repo->add(Account("inactive1", "pass", Role::PATIENT, false, "2025-01-01"));
    repo->add(Account("active2", "pass", Role::PATIENT, true, "2025-01-01"));

    auto activeAccounts = repo->getActiveAccounts();
    EXPECT_EQ(activeAccounts.size(), 2u);
}

TEST_F(AccountRepositoryTest, GetActiveAccounts_AllInactive_ReturnsEmpty) {
    repo->add(Account("inactive1", "pass", Role::PATIENT, false, "2025-01-01"));
    repo->add(Account("inactive2", "pass", Role::PATIENT, false, "2025-01-01"));

    auto activeAccounts = repo->getActiveAccounts();
    EXPECT_TRUE(activeAccounts.empty());
}

// ==================== ValidateCredentials Tests ====================

TEST_F(AccountRepositoryTest, ValidateCredentials_ValidCredentials_ReturnsTrue) {
    repo->add(Account("validuser", "correcthash", Role::PATIENT, true, "2025-01-01"));
    EXPECT_TRUE(repo->validateCredentials("validuser", "correcthash"));
}

TEST_F(AccountRepositoryTest, ValidateCredentials_WrongPassword_ReturnsFalse) {
    repo->add(Account("validuser", "correcthash", Role::PATIENT, true, "2025-01-01"));
    EXPECT_FALSE(repo->validateCredentials("validuser", "wronghash"));
}

TEST_F(AccountRepositoryTest, ValidateCredentials_NonExistentUser_ReturnsFalse) {
    EXPECT_FALSE(repo->validateCredentials("nouser", "anyhash"));
}

TEST_F(AccountRepositoryTest, ValidateCredentials_InactiveAccount_ReturnsFalse) {
    repo->add(Account("inactiveuser", "correcthash", Role::PATIENT, false, "2025-01-01"));
    EXPECT_FALSE(repo->validateCredentials("inactiveuser", "correcthash"));
}

// ==================== Persistence Tests ====================

TEST_F(AccountRepositoryTest, SaveAndLoad_DataPersisted) {
    repo->add(Account("persist1", "hash1", Role::ADMIN, true, "2025-01-01"));
    repo->add(Account("persist2", "hash2", Role::PATIENT, false, "2025-01-02"));
    repo->save();

    // Reset and reload
    AccountRepository::resetInstance();
    repo = AccountRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    EXPECT_EQ(repo->count(), 2u);

    auto acc1 = repo->getById("persist1");
    ASSERT_TRUE(acc1.has_value());
    EXPECT_EQ(acc1->getRole(), Role::ADMIN);
    EXPECT_TRUE(acc1->isActive());

    auto acc2 = repo->getById("persist2");
    ASSERT_TRUE(acc2.has_value());
    EXPECT_EQ(acc2->getRole(), Role::PATIENT);
    EXPECT_FALSE(acc2->isActive());
}

TEST_F(AccountRepositoryTest, Load_NonExistentFile_CreatesFileAndReturnsTrue) {
    std::string nonExistentFile = TEST_DATA_DIR + "nonexistent_file_12345.txt";

    // Ensure file doesn't exist
    if (std::filesystem::exists(nonExistentFile)) {
        std::filesystem::remove(nonExistentFile);
    }

    repo->setFilePath(nonExistentFile);
    EXPECT_TRUE(repo->load());
    EXPECT_EQ(repo->count(), 0u);

    // Cleanup
    if (std::filesystem::exists(nonExistentFile)) {
        std::filesystem::remove(nonExistentFile);
    }
}

TEST_F(AccountRepositoryTest, LazyLoading_AutoLoadsOnAccess) {
    // Add data and save
    repo->add(Account("lazyuser", "hash", Role::PATIENT, true, "2025-01-01"));
    repo->save();

    // Reset instance
    AccountRepository::resetInstance();
    repo = AccountRepository::getInstance();
    repo->setFilePath(testFilePath);

    // Access data without explicit load() - should auto-load
    EXPECT_TRUE(repo->exists("lazyuser"));
}

// ==================== SetFilePath Tests ====================

TEST_F(AccountRepositoryTest, SetFilePath_ChangesFilePath) {
    std::string newPath = TEST_DATA_DIR + "new_test_path.txt";
    repo->setFilePath(newPath);
    EXPECT_EQ(repo->getFilePath(), newPath);

    // Cleanup
    if (std::filesystem::exists(newPath)) {
        std::filesystem::remove(newPath);
    }
}

TEST_F(AccountRepositoryTest, SetFilePath_ForcesReload) {
    // Add to first file
    repo->add(createTestAccount("user1"));
    repo->save();

    // Create second file with different data
    std::string secondFile = TEST_DATA_DIR + "test_accounts_2.txt";
    {
        std::ofstream out(secondFile);
        Account acc("user2", "hash2", Role::DOCTOR, true, "2025-01-01");
        out << acc.serialize() << "\n";
    }

    // Switch to second file
    repo->setFilePath(secondFile);

    // Should load data from new file
    EXPECT_FALSE(repo->exists("user1"));
    EXPECT_TRUE(repo->exists("user2"));

    // Cleanup
    if (std::filesystem::exists(secondFile)) {
        std::filesystem::remove(secondFile);
    }
}

// ==================== Edge Cases ====================

TEST_F(AccountRepositoryTest, Add_AfterClear_Works) {
    repo->add(createTestAccount("user1"));
    repo->clear();
    repo->add(createTestAccount("user2"));

    EXPECT_EQ(repo->count(), 1u);
    EXPECT_TRUE(repo->exists("user2"));
}

TEST_F(AccountRepositoryTest, Update_ActiveStatus_Persisted) {
    repo->add(Account("toggleuser", "hash", Role::PATIENT, true, "2025-01-01"));

    Account updated("toggleuser", "hash", Role::PATIENT, false, "2025-01-01");
    repo->update(updated);

    auto result = repo->getById("toggleuser");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->isActive());
}

TEST_F(AccountRepositoryTest, MultipleOperations_CorrectState) {
    // Add accounts
    repo->add(createTestAccount("user1"));
    repo->add(createTestAccount("user2"));
    repo->add(createTestAccount("user3"));
    EXPECT_EQ(repo->count(), 3u);

    // Remove one
    repo->remove("user2");
    EXPECT_EQ(repo->count(), 2u);

    // Update one
    Account updated("user1", "newhash", Role::ADMIN, true, "2025-01-01");
    repo->update(updated);

    // Verify final state
    EXPECT_FALSE(repo->exists("user2"));
    auto user1 = repo->getById("user1");
    ASSERT_TRUE(user1.has_value());
    EXPECT_EQ(user1->getRole(), Role::ADMIN);
}



/*
To run only the AccountRepository tests, use the following command in the build directory:
cd build && ./HospitalTests --gtest_filter="AccountRepositoryTest.*"
*/
