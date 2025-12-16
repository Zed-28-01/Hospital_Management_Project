#include <gtest/gtest.h>

#include "model/Patient.h"

using namespace HMS;
using namespace HMS::Model;

// ==================== Constructor & Getters ====================

TEST(PatientTest, ConstructorAndGetters)
{
    Patient p(
        "P001",
        "user01",
        "Nguyen Van A",
        "0123456789",
        Gender::MALE,
        "2000-01-01",
        "Ha Noi",
        "None");

    EXPECT_EQ(p.getPatientID(), "P001");
    EXPECT_EQ(p.getID(), "P001");
    EXPECT_EQ(p.getUsername(), "user01");
    EXPECT_EQ(p.getAddress(), "Ha Noi");
    EXPECT_EQ(p.getMedicalHistory(), "None");
}

// ==================== Setters ====================

TEST(PatientTest, SetAddress)
{
    Patient p(
        "P002",
        "user02",
        "Tran Thi B",
        "0987654321",
        Gender::FEMALE,
        "1999-12-31",
        "Old Address",
        "");

    p.setAddress("New Address");
    EXPECT_EQ(p.getAddress(), "New Address");
}

TEST(PatientTest, SetMedicalHistory)
{
    Patient p(
        "P003",
        "user03",
        "Le Van C",
        "0111222333",
        Gender::MALE,
        "1995-05-20",
        "Da Nang",
        "Flu");

    p.setMedicalHistory("Covid");
    EXPECT_EQ(p.getMedicalHistory(), "Covid");
}

// ==================== Append Medical History ====================

TEST(PatientTest, AppendMedicalHistory_Empty)
{
    Patient p(
        "P004",
        "user04",
        "Pham Thi D",
        "0222333444",
        Gender::FEMALE,
        "2001-08-15",
        "HCM",
        "");

    p.appendMedicalHistory("Asthma");
    EXPECT_EQ(p.getMedicalHistory(), "Asthma");
}

TEST(PatientTest, AppendMedicalHistory_NonEmpty)
{
    Patient p(
        "P005",
        "user05",
        "Vo Van E",
        "0333444555",
        Gender::MALE,
        "1988-03-10",
        "Can Tho",
        "Flu");

    p.appendMedicalHistory("Diabetes");
    EXPECT_EQ(p.getMedicalHistory(), "Flu; Diabetes");
}

// ==================== Serialize ====================

TEST(PatientTest, Serialize)
{
    Patient p(
        "P006",
        "user06",
        "Hoang Van F",
        "0444555666",
        Gender::MALE,
        "1990-10-10",
        "Hue",
        "Healthy");

    std::string serialized = p.serialize();

    EXPECT_EQ(
        serialized,
        "P006|user06|Hoang Van F|0444555666|Male|1990-10-10|Hue|Healthy");
}

// ==================== Deserialize ====================

TEST(PatientTest, DeserializeValid)
{
    std::string data =
        "P007|user07|Dang Thi G|0555666777|Female|1998-06-06|Quang Nam|Allergy";

    auto result = Patient::deserialize(data);

    ASSERT_TRUE(result.has_value());

    Patient p = result.value();
    EXPECT_EQ(p.getPatientID(), "P007");
    EXPECT_EQ(p.getUsername(), "user07");
    EXPECT_EQ(p.getAddress(), "Quang Nam");
    EXPECT_EQ(p.getMedicalHistory(), "Allergy");
}

TEST(PatientTest, DeserializeInvalidFieldCount)
{
    std::string invalidData =
        "P008|user08|Short Data";

    auto result = Patient::deserialize(invalidData);
    EXPECT_FALSE(result.has_value());
}

TEST(PatientTest, DeserializeInvalidGender)
{
    std::string invalidGender =
        "P009|user09|Test Name|0666777888|Alien|2000-01-01|Mars|Unknown";

    auto result = Patient::deserialize(invalidGender);
    EXPECT_FALSE(result.has_value());
}

// ==================== Person Base Class Getters ====================

TEST(PatientTest, PersonBaseClassGetters)
{
    Patient p(
        "P010",
        "user10",
        "Nguyen Van Test",
        "0123456789",
        Gender::MALE,
        "2000-01-01",
        "Ha Noi",
        "None");

    // Test inherited Person getters
    EXPECT_EQ(p.getName(), "Nguyen Van Test");
    EXPECT_EQ(p.getPhone(), "0123456789");
    EXPECT_EQ(p.getGender(), Gender::MALE);
    EXPECT_EQ(p.getGenderString(), "Male");
    EXPECT_EQ(p.getDateOfBirth(), "2000-01-01");
}

TEST(PatientTest, PersonBaseClassSetters)
{
    Patient p(
        "P011",
        "user11",
        "Original Name",
        "0123456789",
        Gender::MALE,
        "2000-01-01",
        "Address",
        "History");

    p.setName("New Name");
    p.setPhone("9999999999");
    p.setGender(Gender::FEMALE);
    p.setDateOfBirth("1999-12-31");

    EXPECT_EQ(p.getName(), "New Name");
    EXPECT_EQ(p.getPhone(), "9999999999");
    EXPECT_EQ(p.getGender(), Gender::FEMALE);
    EXPECT_EQ(p.getDateOfBirth(), "1999-12-31");
}

// ==================== Empty Field Edge Cases ====================

TEST(PatientTest, SerializeEmptyMedicalHistory)
{
    Patient p(
        "P012",
        "user12",
        "Test Name",
        "0123456789",
        Gender::MALE,
        "2000-01-01",
        "Address",
        "");

    std::string serialized = p.serialize();
    EXPECT_EQ(serialized,
        "P012|user12|Test Name|0123456789|Male|2000-01-01|Address|");
}

TEST(PatientTest, DeserializeEmptyMedicalHistory)
{
    std::string data = "P013|user13|Test Name|0123456789|Male|2000-01-01|Address|";

    auto result = Patient::deserialize(data);

    ASSERT_TRUE(result.has_value());

    Patient p = result.value();
    EXPECT_EQ(p.getPatientID(), "P013");
    EXPECT_EQ(p.getMedicalHistory(), "");
}

// ==================== Deserialize Full Validation ====================

TEST(PatientTest, DeserializeValidatesAllFields)
{
    std::string data =
        "P014|user14|Full Test|0777888999|Female|1995-05-15|Full Address|Complete History";

    auto result = Patient::deserialize(data);

    ASSERT_TRUE(result.has_value());

    Patient p = result.value();

    // Validate all fields are correctly parsed
    EXPECT_EQ(p.getPatientID(), "P014");
    EXPECT_EQ(p.getUsername(), "user14");
    EXPECT_EQ(p.getName(), "Full Test");
    EXPECT_EQ(p.getPhone(), "0777888999");
    EXPECT_EQ(p.getGender(), Gender::FEMALE);
    EXPECT_EQ(p.getGenderString(), "Female");
    EXPECT_EQ(p.getDateOfBirth(), "1995-05-15");
    EXPECT_EQ(p.getAddress(), "Full Address");
    EXPECT_EQ(p.getMedicalHistory(), "Complete History");
}
