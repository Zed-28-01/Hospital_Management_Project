#include <gtest/gtest.h>

#include "../../include/model/Patient.h"

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
