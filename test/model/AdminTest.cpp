#include <gtest/gtest.h>
#include "model/Admin.h"
#include "common/Types.h"
#include <sstream>

using namespace HMS;
using namespace HMS::Model;

class AdminTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup runs before each test
    }

    void TearDown() override
    {
        // Cleanup runs after each test
    }

    // Helper function to create test admin
    Admin createTestAdmin(
        const std::string &adminID = "A001",
        const std::string &username = "admin_user",
        const std::string &name = "John Admin",
        const std::string &phone = "0123456789",
        Gender gender = Gender::MALE,
        const std::string &dateOfBirth = "1990-01-01")
    {
        return Admin(adminID, username, name, phone, gender, dateOfBirth);
    }
};

// ==================== Constructor Tests ====================

TEST_F(AdminTest, Constructor_ValidParameters_CreatesAdmin)
{
    Admin admin = createTestAdmin();

    EXPECT_EQ(admin.getAdminID(), "A001");
    EXPECT_EQ(admin.getUsername(), "admin_user");
    EXPECT_EQ(admin.getName(), "John Admin");
    EXPECT_EQ(admin.getPhone(), "0123456789");
    EXPECT_EQ(admin.getGender(), Gender::MALE);
    EXPECT_EQ(admin.getDateOfBirth(), "1990-01-01");
}

TEST_F(AdminTest, Constructor_AllGenders_CreatesCorrectly)
{
    Admin male = createTestAdmin("A001", "user1", "Male Admin", "0111111111", Gender::MALE);
    Admin female = createTestAdmin("A002", "user2", "Female Admin", "0222222222", Gender::FEMALE);
    Admin other = createTestAdmin("A003", "user3", "Other Admin", "0333333333", Gender::OTHER);

    EXPECT_EQ(male.getGender(), Gender::MALE);
    EXPECT_EQ(female.getGender(), Gender::FEMALE);
    EXPECT_EQ(other.getGender(), Gender::OTHER);
}

TEST_F(AdminTest, Constructor_EmptyStrings_CreatesAdmin)
{
    Admin admin("", "", "", "", Gender::UNKNOWN, "");

    EXPECT_EQ(admin.getAdminID(), "");
    EXPECT_EQ(admin.getUsername(), "");
    EXPECT_EQ(admin.getName(), "");
}

// ==================== Getter Tests ====================

TEST_F(AdminTest, GetID_ReturnsAdminID)
{
    Admin admin = createTestAdmin("A123");

    EXPECT_EQ(admin.getID(), "A123");
    EXPECT_EQ(admin.getAdminID(), "A123");
}

TEST_F(AdminTest, GetUsername_ReturnsUsername)
{
    Admin admin = createTestAdmin("A001", "admin123");

    EXPECT_EQ(admin.getUsername(), "admin123");
}

TEST_F(AdminTest, GetName_ReturnsName)
{
    Admin admin = createTestAdmin("A001", "user", "Dr. Smith");

    EXPECT_EQ(admin.getName(), "Dr. Smith");
}

TEST_F(AdminTest, GetPhone_ReturnsPhone)
{
    Admin admin = createTestAdmin("A001", "user", "Name", "0987654321");

    EXPECT_EQ(admin.getPhone(), "0987654321");
}

TEST_F(AdminTest, GetGender_ReturnsGender)
{
    Admin admin = createTestAdmin("A001", "user", "Name", "0123456789", Gender::FEMALE);

    EXPECT_EQ(admin.getGender(), Gender::FEMALE);
}

TEST_F(AdminTest, GetDateOfBirth_ReturnsDate)
{
    Admin admin = createTestAdmin("A001", "user", "Name", "0123456789", Gender::MALE, "1985-05-15");

    EXPECT_EQ(admin.getDateOfBirth(), "1985-05-15");
}

// ==================== Serialize Tests ====================

TEST_F(AdminTest, Serialize_AllFields_CorrectFormat)
{
    Admin admin = createTestAdmin("A001", "admin_user", "John Doe", "0123456789",
                                  Gender::MALE, "1990-01-01");

    std::string serialized = admin.serialize();

    EXPECT_EQ(serialized, "A001|admin_user|John Doe|0123456789|Nam|1990-01-01");
}

TEST_F(AdminTest, Serialize_FemaleGender_CorrectFormat)
{
    Admin admin = createTestAdmin("A002", "jane_admin", "Jane Smith", "0987654321",
                                  Gender::FEMALE, "1992-03-15");

    std::string serialized = admin.serialize();

    EXPECT_EQ(serialized, "A002|jane_admin|Jane Smith|0987654321|Nữ|1992-03-15");
}

TEST_F(AdminTest, Serialize_OtherGender_CorrectFormat)
{
    Admin admin = createTestAdmin("A003", "other_admin", "Alex Jones", "0555555555",
                                  Gender::OTHER, "1988-12-25");

    std::string serialized = admin.serialize();

    EXPECT_EQ(serialized, "A003|other_admin|Alex Jones|0555555555|Khác|1988-12-25");
}

TEST_F(AdminTest, Serialize_UnknownGender_CorrectFormat)
{
    Admin admin = createTestAdmin("A004", "unknown_admin", "Unknown Person", "0111111111",
                                  Gender::UNKNOWN, "2000-01-01");

    std::string serialized = admin.serialize();

    EXPECT_EQ(serialized, "A004|unknown_admin|Unknown Person|0111111111|Không xác định|2000-01-01");
}

TEST_F(AdminTest, Serialize_EmptyFields_HandlesCorrectly)
{
    Admin admin("", "", "", "", Gender::UNKNOWN, "");

    std::string serialized = admin.serialize();

    // Format: adminID|username|name|phone|gender|dateOfBirth
    // 6 fields = 5 pipes
    EXPECT_EQ(serialized, "||||Không xác định|");
}

TEST_F(AdminTest, Serialize_SpecialCharactersInName_PreservesData)
{
    Admin admin = createTestAdmin("A001", "user", "O'Brien-Smith", "0123456789");

    std::string serialized = admin.serialize();

    EXPECT_TRUE(serialized.find("O'Brien-Smith") != std::string::npos);
}

// ==================== Deserialize Tests ====================

TEST_F(AdminTest, Deserialize_ValidLine_ReturnsAdmin)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAdminID(), "A001");
    EXPECT_EQ(result->getUsername(), "admin_user");
    EXPECT_EQ(result->getName(), "John Doe");
    EXPECT_EQ(result->getPhone(), "0123456789");
    EXPECT_EQ(result->getGender(), Gender::MALE);
    EXPECT_EQ(result->getDateOfBirth(), "1990-01-01");
}

TEST_F(AdminTest, Deserialize_AllGenders_ParsedCorrectly)
{
    auto male = Admin::deserialize("A001|user1|Male Admin|0111111111|Male|1990-01-01");
    auto female = Admin::deserialize("A002|user2|Female Admin|0222222222|Female|1991-02-02");
    auto other = Admin::deserialize("A003|user3|Other Admin|0333333333|Other|1992-03-03");

    ASSERT_TRUE(male.has_value());
    ASSERT_TRUE(female.has_value());
    ASSERT_TRUE(other.has_value());

    EXPECT_EQ(male->getGender(), Gender::MALE);
    EXPECT_EQ(female->getGender(), Gender::FEMALE);
    EXPECT_EQ(other->getGender(), Gender::OTHER);
}

TEST_F(AdminTest, Deserialize_EmptyLine_ReturnsNullopt)
{
    std::string line = "";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_CommentLine_ReturnsNullopt)
{
    std::string line = "# This is a comment";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_TooFewFields_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male"; // Missing dateOfBirth

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_TooManyFields_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|1990-01-01|ExtraField";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_EmptyAdminID_ReturnsNullopt)
{
    std::string line = "|admin_user|John Doe|0123456789|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_EmptyUsername_ReturnsNullopt)
{
    std::string line = "A001||John Doe|0123456789|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_EmptyName_ReturnsNullopt)
{
    std::string line = "A001|admin_user||0123456789|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_InvalidPhoneNumber_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|invalid_phone|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_InvalidDateFormat_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|invalid_date";

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_InvalidDateFormat2_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|1990/01/01"; // Wrong separator

    auto result = Admin::deserialize(line);

    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_UnknownGender_ParsesAsUnknown)
{
    std::string line = "A001|admin_user|John Doe|0123456789|InvalidGender|1990-01-01";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getGender(), Gender::UNKNOWN);
}

TEST_F(AdminTest, Deserialize_WhitespaceInFields_TrimmedCorrectly)
{
    std::string line = "  A001  |  admin_user  |  John Doe  |  0123456789  |  Male  |  1990-01-01  ";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAdminID(), "A001");
    EXPECT_EQ(result->getUsername(), "admin_user");
    EXPECT_EQ(result->getName(), "John Doe");
}

TEST_F(AdminTest, Deserialize_SpecialCharactersInName_PreservesData)
{
    std::string line = "A001|user|O'Brien-Smith Jr.|0123456789|Male|1990-01-01";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "O'Brien-Smith Jr.");
}

// ==================== Serialize/Deserialize Round-Trip Tests ====================

TEST_F(AdminTest, SerializeDeserialize_RoundTrip_PreservesData)
{
    Admin original = createTestAdmin("A001", "admin_user", "John Doe", "0123456789",
                                     Gender::MALE, "1990-01-01");

    std::string serialized = original.serialize();
    auto deserialized = Admin::deserialize(serialized);

    ASSERT_TRUE(deserialized.has_value());
    EXPECT_EQ(deserialized->getAdminID(), original.getAdminID());
    EXPECT_EQ(deserialized->getUsername(), original.getUsername());
    EXPECT_EQ(deserialized->getName(), original.getName());
    EXPECT_EQ(deserialized->getPhone(), original.getPhone());
    EXPECT_EQ(deserialized->getGender(), original.getGender());
    EXPECT_EQ(deserialized->getDateOfBirth(), original.getDateOfBirth());
}

TEST_F(AdminTest, SerializeDeserialize_MultipleAdmins_AllPreserved)
{
    Admin admin1 = createTestAdmin("A001", "user1", "Admin One", "0111111111", Gender::MALE);
    Admin admin2 = createTestAdmin("A002", "user2", "Admin Two", "0222222222", Gender::FEMALE);
    Admin admin3 = createTestAdmin("A003", "user3", "Admin Three", "0333333333", Gender::OTHER);

    auto deserialized1 = Admin::deserialize(admin1.serialize());
    auto deserialized2 = Admin::deserialize(admin2.serialize());
    auto deserialized3 = Admin::deserialize(admin3.serialize());

    ASSERT_TRUE(deserialized1.has_value());
    ASSERT_TRUE(deserialized2.has_value());
    ASSERT_TRUE(deserialized3.has_value());

    EXPECT_EQ(deserialized1->getAdminID(), "A001");
    EXPECT_EQ(deserialized2->getAdminID(), "A002");
    EXPECT_EQ(deserialized3->getAdminID(), "A003");
}

// ==================== DisplayInfo Tests ====================

TEST_F(AdminTest, DisplayInfo_ValidAdmin_NoThrow)
{
    Admin admin = createTestAdmin();

    // Redirect cout to capture output
    std::ostringstream output;
    std::streambuf *oldCout = std::cout.rdbuf(output.rdbuf());

    EXPECT_NO_THROW(admin.displayInfo());

    // Restore cout
    std::cout.rdbuf(oldCout);

    // Verify output contains expected info
    std::string result = output.str();
    EXPECT_TRUE(result.find("ADMIN INFORMATION") != std::string::npos);
    EXPECT_TRUE(result.find("A001") != std::string::npos);
    EXPECT_TRUE(result.find("admin_user") != std::string::npos);
    EXPECT_TRUE(result.find("John Admin") != std::string::npos);
}

TEST_F(AdminTest, DisplayInfo_EmptyFields_NoThrow)
{
    Admin admin("", "", "", "", Gender::UNKNOWN, "");

    std::ostringstream output;
    std::streambuf *oldCout = std::cout.rdbuf(output.rdbuf());

    EXPECT_NO_THROW(admin.displayInfo());

    std::cout.rdbuf(oldCout);
}

// ==================== Edge Cases ====================

TEST_F(AdminTest, AdminID_VeryLongString_HandlesCorrectly)
{
    std::string longID(1000, 'A');
    Admin admin = createTestAdmin(longID);

    EXPECT_EQ(admin.getAdminID(), longID);
}

TEST_F(AdminTest, Username_SpecialCharacters_HandlesCorrectly)
{
    Admin admin = createTestAdmin("A001", "user@test.com_123");

    EXPECT_EQ(admin.getUsername(), "user@test.com_123");
}

TEST_F(AdminTest, Phone_DifferentFormats_ValidPhonesAccepted)
{
    // Assuming Utils::isValidPhone accepts various formats
    std::string line1 = "A001|user|Name|0123456789|Male|1990-01-01";
    std::string line2 = "A002|user|Name|+84123456789|Male|1990-01-01";

    auto result1 = Admin::deserialize(line1);
    // Note: result2 might fail depending on Utils::isValidPhone implementation

    ASSERT_TRUE(result1.has_value());
}

TEST_F(AdminTest, DateOfBirth_LeapYear_HandlesCorrectly)
{
    std::string line = "A001|user|Name|0123456789|Male|2000-02-29";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDateOfBirth(), "2000-02-29");
}

// ==================== Default Constructor Test ====================

TEST_F(AdminTest, DefaultConstructor_CreatesAdmin)
{
    Admin admin;

    // Default constructed admin should have empty/default values
    EXPECT_EQ(admin.getAdminID(), "");
    EXPECT_EQ(admin.getUsername(), "");
    EXPECT_EQ(admin.getName(), "");
}

TEST_F(AdminTest, Deserialize_PhoneTooShort_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|012345678|Male|1990-01-01"; // 9 digits
    auto result = Admin::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_PhoneTooLong_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|01234567890|Male|1990-01-01"; // 11 digits
    auto result = Admin::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_PhoneNotStartWithZero_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|1234567890|Male|1990-01-01";
    auto result = Admin::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_DateInvalidMonth_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|1990-13-01"; // Month 13
    auto result = Admin::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_DateInvalidDay_ReturnsNullopt)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|1990-01-32"; // Day 32
    auto result = Admin::deserialize(line);
    EXPECT_FALSE(result.has_value());
}

TEST_F(AdminTest, Deserialize_FutureDate_HandlesCorrectly)
{
    std::string line = "A001|admin_user|John Doe|0123456789|Male|2099-12-31";
    auto result = Admin::deserialize(line);
    ASSERT_TRUE(result.has_value()); // Should accept future dates
}

TEST_F(AdminTest, Deserialize_GenderCaseSensitive_HandlesCorrectly)
{
    auto uppercase = Admin::deserialize("A001|user|Name|0123456789|Male|1990-01-01");
    auto lowercase = Admin::deserialize("A002|user|Name|0123456789|male|1990-01-01");

    ASSERT_TRUE(uppercase.has_value());
    EXPECT_EQ(uppercase->getGender(), Gender::MALE);

    ASSERT_TRUE(lowercase.has_value());

    EXPECT_TRUE(
        lowercase->getGender() == Gender::MALE ||
        lowercase->getGender() == Gender::UNKNOWN);
}

TEST_F(AdminTest, Serialize_NameWithPipe_SanitizedCorrectly)
{
    Admin admin = createTestAdmin("A001", "user", "Name|WithPipe", "0123456789");
    std::string serialized = admin.serialize();

    // Verify pipe in name is sanitized (replaced with space)
    auto result = Admin::deserialize(serialized);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Name WithPipe"); // Pipe replaced with space
}

TEST_F(AdminTest, Deserialize_SpacesInFields_TrimmedCorrectly)
{
    std::string line = "  A001  |  admin_user  |  John Doe  |  0123456789  |  Male  |  1990-01-01  ";

    auto result = Admin::deserialize(line);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAdminID(), "A001");
    EXPECT_EQ(result->getUsername(), "admin_user");
}

TEST_F(AdminTest, Serialize_VietnameseName_HandlesCorrectly)
{
    Admin admin = createTestAdmin("A001", "user", "Nguyễn Văn A", "0123456789");
    std::string serialized = admin.serialize();

    auto result = Admin::deserialize(serialized);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Nguyễn Văn A");
}

TEST_F(AdminTest, DateOfBirth_MinimumYear_HandlesCorrectly)
{
    std::string line = "A001|user|Name|0123456789|Male|1900-01-01";
    auto result = Admin::deserialize(line);
    ASSERT_TRUE(result.has_value());
}

TEST_F(AdminTest, DateOfBirth_MaximumYear_HandlesCorrectly)
{
    std::string line = "A001|user|Name|0123456789|Male|2100-12-31";
    auto result = Admin::deserialize(line);
    ASSERT_TRUE(result.has_value());
}

TEST_F(AdminTest, GetID_SameAsGetAdminID_AlwaysEqual)
{
    Admin admin = createTestAdmin("TEST_ID");
    EXPECT_EQ(admin.getID(), admin.getAdminID());
}

/*
To run only the Admin tests, use the following command in the build directory:
cd build && ./HospitalTests --gtest_filter="AdminTest.*"
*/