#include <gtest/gtest.h>
#include "model/Doctor.h"
#include "common/Types.h"
#include "common/Utils.h"
#include <string>

using namespace HMS::Model;
using namespace HMS;

TEST(DoctorTest, ConstructorAndGetters) {

    Doctor doc(
        "D001",                 // ID
        "doc_john",             // Username
        "John Doe",             // Name
        "0912345678",           // Phone
        Gender::MALE,           // Gender
        "1980-01-01",           // DOB
        "Cardiology",           // Specialization
        "Mon-Fri 08:00-17:00",  // Schedule (String)
        500000.0                // Fee
    );

    EXPECT_EQ(doc.getID(), "D001");
    EXPECT_EQ(doc.getUsername(), "doc_john");
    EXPECT_EQ(doc.getName(), "John Doe");
    EXPECT_EQ(doc.getPhone(), "0912345678");
    EXPECT_EQ(doc.getGender(), Gender::MALE);
    EXPECT_EQ(doc.getDateOfBirth(), "1980-01-01");
    EXPECT_EQ(doc.getSpecialization(), "Cardiology");
    EXPECT_EQ(doc.getSchedule(), "Mon-Fri 08:00-17:00");
    EXPECT_DOUBLE_EQ(doc.getConsultationFee(), 500000.0);
}


TEST(DoctorTest, SettersUpdateValuesCorrectly) {
    Doctor doc;

    doc.setSpecialization("Neurology");
    doc.setSchedule("Sat-Sun 09:00-12:00");
    doc.setConsultationFee(1000000.0);

    // Kiểm tra lại
    EXPECT_EQ(doc.getSpecialization(), "Neurology");
    EXPECT_EQ(doc.getSchedule(), "Sat-Sun 09:00-12:00");
    EXPECT_DOUBLE_EQ(doc.getConsultationFee(), 1000000.0);
}

TEST(DoctorTest, SerializeReturnsCorrectFormat) {
    Doctor doc(
        "D002", "jane_doe", "Jane Doe", "0987654321",
        Gender::FEMALE, "1985-05-05", "Dermatology",
        "Tue-Thu", 300000.0
    );

    std::string serialized = doc.serialize();


    EXPECT_NE(serialized.find("D002"), std::string::npos);
    EXPECT_NE(serialized.find("Jane Doe"), std::string::npos);
    EXPECT_NE(serialized.find("Dermatology"), std::string::npos);
    EXPECT_NE(serialized.find("300000"), std::string::npos);


    int pipes = 0;
    for (char c : serialized) {
        if (c == '|') pipes++;
    }
    EXPECT_EQ(pipes, 8);
}

TEST(DoctorTest, DeserializeValidString) {

    std::string data = "D003|doc_test|Dr. Test|0123456789|Male|1990-01-01|General|Mon-Wed|200000";

    auto result = Doctor::deserialize(data);

    ASSERT_TRUE(result.has_value());

    Doctor doc = result.value();
    EXPECT_EQ(doc.getID(), "D003");
    EXPECT_EQ(doc.getName(), "Dr. Test");
    EXPECT_EQ(doc.getSpecialization(), "General");
    EXPECT_EQ(doc.getSchedule(), "Mon-Wed"); // String
    EXPECT_DOUBLE_EQ(doc.getConsultationFee(), 200000.0);
}

TEST(DoctorTest, DeserializeEmptyStringReturnsNullopt) {
    auto result = Doctor::deserialize("");
    EXPECT_FALSE(result.has_value());
}

TEST(DoctorTest, DeserializeInvalidStringReturnsNullopt) {
    std::string invalidData = "D003|doc_test|Dr. Test";

    auto result = Doctor::deserialize(invalidData);
    EXPECT_FALSE(result.has_value());
}
