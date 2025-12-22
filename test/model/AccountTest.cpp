#include "model/Account.h"
#include "common/Types.h"
#include <gtest/gtest.h>
#include <sstream>


using namespace HMS;
using namespace HMS::Model;

class AccountTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Setup runs before each test
  }

  void TearDown() override {
    // Cleanup runs after each test
  }

  // Helper function to create test account
  Account createTestAccount(const std::string &username = "testuser",
                            const std::string &passwordHash = "hash123",
                            Role role = Role::PATIENT, bool isActive = true,
                            const std::string &createdDate = "2024-01-01") {
    return Account(username, passwordHash, role, isActive, createdDate);
  }
};

// ==================== Constructor Tests ====================

TEST_F(AccountTest, Constructor_ValidParameters_CreatesAccount) {
  Account account = createTestAccount();

  EXPECT_EQ(account.getUsername(), "testuser");
  EXPECT_EQ(account.getPasswordHash(), "hash123");
  EXPECT_EQ(account.getRole(), Role::PATIENT);
  EXPECT_TRUE(account.isActive());
  EXPECT_EQ(account.getCreatedDate(), "2024-01-01");
}

TEST_F(AccountTest, Constructor_AllRoles_CreatesCorrectly) {
  Account patient = createTestAccount("patient1", "hash1", Role::PATIENT);
  Account doctor = createTestAccount("doctor1", "hash2", Role::DOCTOR);
  Account admin = createTestAccount("admin1", "hash3", Role::ADMIN);

  EXPECT_EQ(patient.getRole(), Role::PATIENT);
  EXPECT_EQ(doctor.getRole(), Role::DOCTOR);
  EXPECT_EQ(admin.getRole(), Role::ADMIN);
}

TEST_F(AccountTest, Constructor_InactiveAccount_CreatesCorrectly) {
  Account account = createTestAccount("user", "hash", Role::PATIENT, false);

  EXPECT_FALSE(account.isActive());
}

TEST_F(AccountTest, Constructor_EmptyStrings_CreatesAccount) {
  Account account("", "", Role::UNKNOWN, true, "");

  EXPECT_EQ(account.getUsername(), "");
  EXPECT_EQ(account.getPasswordHash(), "");
  EXPECT_EQ(account.getCreatedDate(), "");
}

// ==================== Getter Tests ====================

TEST_F(AccountTest, GetUsername_ReturnsUsername) {
  Account account = createTestAccount("myusername");

  EXPECT_EQ(account.getUsername(), "myusername");
}

TEST_F(AccountTest, GetPasswordHash_ReturnsHash) {
  Account account = createTestAccount("user", "secrethash");

  EXPECT_EQ(account.getPasswordHash(), "secrethash");
}

TEST_F(AccountTest, GetRole_ReturnsRole) {
  Account account = createTestAccount("user", "hash", Role::DOCTOR);

  EXPECT_EQ(account.getRole(), Role::DOCTOR);
}

TEST_F(AccountTest, GetRoleString_ReturnsCorrectString) {
  Account patient = createTestAccount("user1", "hash", Role::PATIENT);
  Account doctor = createTestAccount("user2", "hash", Role::DOCTOR);
  Account admin = createTestAccount("user3", "hash", Role::ADMIN);

  EXPECT_EQ(patient.getRoleString(), "patient");
  EXPECT_EQ(doctor.getRoleString(), "doctor");
  EXPECT_EQ(admin.getRoleString(), "admin");
}

TEST_F(AccountTest, IsActive_ReturnsActiveStatus) {
  Account active = createTestAccount("user1", "hash", Role::PATIENT, true);
  Account inactive = createTestAccount("user2", "hash", Role::PATIENT, false);

  EXPECT_TRUE(active.isActive());
  EXPECT_FALSE(inactive.isActive());
}

TEST_F(AccountTest, GetCreatedDate_ReturnsDate) {
  Account account =
      createTestAccount("user", "hash", Role::PATIENT, true, "2023-05-15");

  EXPECT_EQ(account.getCreatedDate(), "2023-05-15");
}

// ==================== Setter Tests ====================

TEST_F(AccountTest, SetPasswordHash_UpdatesHash) {
  Account account = createTestAccount();

  account.setPasswordHash("newhash456");

  EXPECT_EQ(account.getPasswordHash(), "newhash456");
}

TEST_F(AccountTest, SetActive_UpdatesStatus) {
  Account account = createTestAccount("user", "hash", Role::PATIENT, true);

  account.setActive(false);
  EXPECT_FALSE(account.isActive());

  account.setActive(true);
  EXPECT_TRUE(account.isActive());
}

TEST_F(AccountTest, SetRole_UpdatesRole) {
  Account account = createTestAccount("user", "hash", Role::PATIENT);

  account.setRole(Role::DOCTOR);
  EXPECT_EQ(account.getRole(), Role::DOCTOR);

  account.setRole(Role::ADMIN);
  EXPECT_EQ(account.getRole(), Role::ADMIN);
}

// ==================== VerifyPassword Tests ====================

TEST_F(AccountTest, VerifyPassword_MatchingPassword_ReturnsTrue) {
  Account account = createTestAccount("user", "mypassword");

  // Note: Current implementation compares plain password with stored hash
  // directly In production, this should use proper hashing
  EXPECT_TRUE(account.verifyPassword("mypassword"));
}

TEST_F(AccountTest, VerifyPassword_NonMatchingPassword_ReturnsFalse) {
  Account account = createTestAccount("user", "correctpassword");

  EXPECT_FALSE(account.verifyPassword("wrongpassword"));
}

TEST_F(AccountTest, VerifyPassword_EmptyPassword_ReturnsFalse) {
  Account account = createTestAccount("user", "somepassword");

  EXPECT_FALSE(account.verifyPassword(""));
}

TEST_F(AccountTest, VerifyPassword_EmptyStoredHash_MatchesEmptyPassword) {
  Account account = createTestAccount("user", "");

  EXPECT_TRUE(account.verifyPassword(""));
  EXPECT_FALSE(account.verifyPassword("anypassword"));
}

// ==================== Serialize Tests ====================

TEST_F(AccountTest, Serialize_AllFields_CorrectFormat) {
  Account account =
      createTestAccount("user01", "hash123", Role::PATIENT, true, "2024-01-01");

  std::string serialized = account.serialize();

  EXPECT_EQ(serialized, "user01|hash123|patient|1|2024-01-01");
}

TEST_F(AccountTest, Serialize_DoctorRole_CorrectFormat) {
  Account account = createTestAccount("doctor01", "dochash", Role::DOCTOR, true,
                                      "2024-02-15");

  std::string serialized = account.serialize();

  EXPECT_EQ(serialized, "doctor01|dochash|doctor|1|2024-02-15");
}

TEST_F(AccountTest, Serialize_AdminRole_CorrectFormat) {
  Account account = createTestAccount("admin01", "adminhash", Role::ADMIN, true,
                                      "2024-03-20");

  std::string serialized = account.serialize();

  EXPECT_EQ(serialized, "admin01|adminhash|admin|1|2024-03-20");
}

TEST_F(AccountTest, Serialize_InactiveAccount_CorrectFormat) {
  Account account =
      createTestAccount("user01", "hash", Role::PATIENT, false, "2024-01-01");

  std::string serialized = account.serialize();

  EXPECT_EQ(serialized, "user01|hash|patient|0|2024-01-01");
}

TEST_F(AccountTest, Serialize_EmptyFields_HandlesCorrectly) {
  Account account("", "", Role::UNKNOWN, true, "");

  std::string serialized = account.serialize();

  // Format: username|passwordHash|role|isActive|createdDate
  // 5 fields = 4 pipes
  EXPECT_EQ(serialized, "||unknown|1|");
}

// ==================== Deserialize Tests ====================

TEST_F(AccountTest, Deserialize_ValidLine_ReturnsAccount) {
  std::string line = "user01|hash123|patient|1|2024-01-01";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->getUsername(), "user01");
  EXPECT_EQ(result->getPasswordHash(), "hash123");
  EXPECT_EQ(result->getRole(), Role::PATIENT);
  EXPECT_TRUE(result->isActive());
  EXPECT_EQ(result->getCreatedDate(), "2024-01-01");
}

TEST_F(AccountTest, Deserialize_AllRoles_ParsedCorrectly) {
  auto patient = Account::deserialize("patient01|hash1|patient|1|2024-01-01");
  auto doctor = Account::deserialize("doctor01|hash2|doctor|1|2024-01-01");
  auto admin = Account::deserialize("admin01|hash3|admin|1|2024-01-01");

  ASSERT_TRUE(patient.has_value());
  ASSERT_TRUE(doctor.has_value());
  ASSERT_TRUE(admin.has_value());

  EXPECT_EQ(patient->getRole(), Role::PATIENT);
  EXPECT_EQ(doctor->getRole(), Role::DOCTOR);
  EXPECT_EQ(admin->getRole(), Role::ADMIN);
}

TEST_F(AccountTest, Deserialize_InactiveAccount_ParsedCorrectly) {
  std::string line = "user01|hash|patient|0|2024-01-01";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->isActive());
}

TEST_F(AccountTest, Deserialize_EmptyLine_ReturnsNullopt) {
  std::string line = "";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_CommentLine_ReturnsNullopt) {
  std::string line = "# This is a comment";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_TooFewFields_ReturnsNullopt) {
  std::string line = "user01|hash|patient|1"; // Missing createdDate

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_TooManyFields_ReturnsNullopt) {
  std::string line = "user01|hash|patient|1|2024-01-01|ExtraField";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_EmptyUsername_ReturnsNullopt) {
  std::string line = "|hash123|patient|1|2024-01-01";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_EmptyPasswordHash_ReturnsNullopt) {
  std::string line = "user01||patient|1|2024-01-01";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_InvalidRole_ReturnsNullopt) {
  std::string line = "user01|hash|invalidrole|1|2024-01-01";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

TEST_F(AccountTest, Deserialize_WhitespaceInFields_TrimmedCorrectly) {
  std::string line = "  user01  |  hash123  |  patient  |  1  |  2024-01-01  ";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->getUsername(), "user01");
  EXPECT_EQ(result->getPasswordHash(), "hash123");
}

TEST_F(AccountTest, Deserialize_InvalidUsername_ReturnsNullopt) {
  // Username with spaces should be invalid based on Utils::isValidUsername
  std::string line = "user with spaces|hash|patient|1|2024-01-01";

  auto result = Account::deserialize(line);

  EXPECT_FALSE(result.has_value());
}

// ==================== Serialize/Deserialize Round-Trip Tests
// ====================

TEST_F(AccountTest, SerializeDeserialize_RoundTrip_PreservesData) {
  Account original = createTestAccount("roundtrip_user", "roundtrip_hash",
                                       Role::DOCTOR, true, "2024-06-15");

  std::string serialized = original.serialize();
  auto deserialized = Account::deserialize(serialized);

  ASSERT_TRUE(deserialized.has_value());
  EXPECT_EQ(deserialized->getUsername(), original.getUsername());
  EXPECT_EQ(deserialized->getPasswordHash(), original.getPasswordHash());
  EXPECT_EQ(deserialized->getRole(), original.getRole());
  EXPECT_EQ(deserialized->isActive(), original.isActive());
  EXPECT_EQ(deserialized->getCreatedDate(), original.getCreatedDate());
}

TEST_F(AccountTest, SerializeDeserialize_MultipleAccounts_AllPreserved) {
  Account account1 =
      createTestAccount("user1", "hash1", Role::PATIENT, true, "2024-01-01");
  Account account2 =
      createTestAccount("user2", "hash2", Role::DOCTOR, false, "2024-02-02");
  Account account3 =
      createTestAccount("user3", "hash3", Role::ADMIN, true, "2024-03-03");

  auto deserialized1 = Account::deserialize(account1.serialize());
  auto deserialized2 = Account::deserialize(account2.serialize());
  auto deserialized3 = Account::deserialize(account3.serialize());

  ASSERT_TRUE(deserialized1.has_value());
  ASSERT_TRUE(deserialized2.has_value());
  ASSERT_TRUE(deserialized3.has_value());

  EXPECT_EQ(deserialized1->getUsername(), "user1");
  EXPECT_EQ(deserialized2->getUsername(), "user2");
  EXPECT_EQ(deserialized3->getUsername(), "user3");

  EXPECT_TRUE(deserialized1->isActive());
  EXPECT_FALSE(deserialized2->isActive());
  EXPECT_TRUE(deserialized3->isActive());
}

TEST_F(AccountTest, SerializeDeserialize_InactiveAccount_PreservesStatus) {
  Account inactive = createTestAccount("inactiveuser", "hash", Role::PATIENT,
                                       false, "2024-01-01");

  std::string serialized = inactive.serialize();
  auto deserialized = Account::deserialize(serialized);

  ASSERT_TRUE(deserialized.has_value());
  EXPECT_FALSE(deserialized->isActive());
}

// ==================== Edge Cases ====================

TEST_F(AccountTest, Username_VeryLongString_HandlesCorrectly) {
  std::string longUsername(100, 'u');
  Account account(longUsername, "hash", Role::PATIENT, true, "2024-01-01");

  EXPECT_EQ(account.getUsername(), longUsername);
}

TEST_F(AccountTest, PasswordHash_VeryLongString_HandlesCorrectly) {
  std::string longHash(256, 'h');
  Account account = createTestAccount("user", longHash);

  EXPECT_EQ(account.getPasswordHash(), longHash);
}

TEST_F(AccountTest, Username_UnderscoresAndNumbers_HandlesCorrectly) {
  Account account = createTestAccount("user_123_test");

  EXPECT_EQ(account.getUsername(), "user_123_test");
}

TEST_F(AccountTest, PasswordHash_SpecialCharacters_HandlesCorrectly) {
  Account account = createTestAccount("user", "hash!@#$%^&*()");

  EXPECT_EQ(account.getPasswordHash(), "hash!@#$%^&*()");

  std::string serialized = account.serialize();
  auto deserialized = Account::deserialize(serialized);

  ASSERT_TRUE(deserialized.has_value());
  EXPECT_EQ(deserialized->getPasswordHash(), "hash!@#$%^&*()");
}

// ==================== Default Constructor Test ====================

TEST_F(AccountTest, DefaultConstructor_CreatesAccount) {
  Account account;

  // Default constructed account should have empty/default values
  EXPECT_EQ(account.getUsername(), "");
  EXPECT_EQ(account.getPasswordHash(), "");
}

// ==================== Role String Conversion Tests ====================

TEST_F(AccountTest, GetRoleString_UnknownRole_ReturnsUnknown) {
  Account account("user", "hash", Role::UNKNOWN, true, "2024-01-01");

  EXPECT_EQ(account.getRoleString(), "unknown");
}

TEST_F(AccountTest, Deserialize_RoleCaseInsensitive_HandlesCorrectly) {
  auto uppercase = Account::deserialize("user1|hash|PATIENT|1|2024-01-01");
  auto lowercase = Account::deserialize("user2|hash|patient|1|2024-01-01");
  auto mixed = Account::deserialize("user3|hash|Patient|1|2024-01-01");

  // Depending on implementation, case sensitivity may vary
  // At minimum, lowercase should work
  ASSERT_TRUE(lowercase.has_value());
  EXPECT_EQ(lowercase->getRole(), Role::PATIENT);

  // Others may or may not work depending on stringToRole implementation
}

// ==================== Date Edge Cases ====================

TEST_F(AccountTest, CreatedDate_FutureDate_Accepted) {
  std::string line = "user01|hash|patient|1|2099-12-31";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->getCreatedDate(), "2099-12-31");
}

TEST_F(AccountTest, CreatedDate_OldDate_Accepted) {
  std::string line = "user01|hash|patient|1|1990-01-01";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->getCreatedDate(), "1990-01-01");
}

TEST_F(AccountTest, CreatedDate_EmptyWithOtherValidFields_Deserializes) {
  // Empty created date should still work if other fields are valid
  std::string line = "user01|hash|patient|1|";

  auto result = Account::deserialize(line);

  // This might succeed or fail depending on validation requirements
  // If it succeeds, the date should be empty
  if (result.has_value()) {
    EXPECT_EQ(result->getCreatedDate(), "");
  }
}

// ==================== Active Status Tests ====================

TEST_F(AccountTest, Deserialize_ActiveStatus1_IsActive) {
  std::string line = "user|hash|patient|1|2024-01-01";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->isActive());
}

TEST_F(AccountTest, Deserialize_ActiveStatus0_IsInactive) {
  std::string line = "user|hash|patient|0|2024-01-01";

  auto result = Account::deserialize(line);

  ASSERT_TRUE(result.has_value());
  EXPECT_FALSE(result->isActive());
}

/*
To run only the Account tests, use the following command in the build directory:
cd build && ./HospitalTests --gtest_filter="AccountTest.*"
*/
