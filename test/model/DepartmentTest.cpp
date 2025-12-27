#include "advance/Department.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>


using namespace HMS::Model;
using namespace HMS;

// ==================== Constructor Tests ====================

TEST(DepartmentTest, DefaultConstructor) {
  Department dept;

  EXPECT_EQ(dept.getDepartmentID(), "");
  EXPECT_EQ(dept.getName(), "");
  EXPECT_EQ(dept.getDescription(), "");
  EXPECT_EQ(dept.getHeadDoctorID(), "");
  EXPECT_EQ(dept.getDoctorCount(), 0);
  EXPECT_EQ(dept.getLocation(), "");
  EXPECT_EQ(dept.getPhone(), "");
}

TEST(DepartmentTest, ParameterizedConstructor) {
  Department dept("DEP001", "Cardiology", "Heart and cardiovascular care",
                  "D001");

  EXPECT_EQ(dept.getDepartmentID(), "DEP001");
  EXPECT_EQ(dept.getName(), "Cardiology");
  EXPECT_EQ(dept.getDescription(), "Heart and cardiovascular care");
  EXPECT_EQ(dept.getHeadDoctorID(), "D001");
  EXPECT_EQ(dept.getDoctorCount(), 0);
  EXPECT_EQ(dept.getLocation(), "");
  EXPECT_EQ(dept.getPhone(), "");
}

// ==================== Getters/Setters Tests ====================

TEST(DepartmentTest, SettersUpdateValuesCorrectly) {
  Department dept;

  dept.setName("Neurology");
  dept.setDescription("Brain and nervous system");
  dept.setHeadDoctorID("D002");
  dept.setLocation("Building A, Floor 3");
  dept.setPhone("0281234567");

  EXPECT_EQ(dept.getName(), "Neurology");
  EXPECT_EQ(dept.getDescription(), "Brain and nervous system");
  EXPECT_EQ(dept.getHeadDoctorID(), "D002");
  EXPECT_EQ(dept.getLocation(), "Building A, Floor 3");
  EXPECT_EQ(dept.getPhone(), "0281234567");
}

// Model is a data container - setters don't validate
// Validation is done at BLL layer
TEST(DepartmentTest, SetPhoneAcceptsAnyValue) {
  Department dept("DEP001", "Test", "Desc", "D001");
  dept.setPhone("0912345678");

  // Model accepts any value - validation is at BLL
  dept.setPhone("invalid");
  EXPECT_EQ(dept.getPhone(), "invalid");
}

TEST(DepartmentTest, SetPhoneEmptyAllowed) {
  Department dept("DEP001", "Test", "Desc", "D001");
  dept.setPhone("0912345678");

  // Empty phone should be allowed
  dept.setPhone("");
  EXPECT_EQ(dept.getPhone(), "");
}

// ==================== Doctor Management Tests ====================

TEST(DepartmentTest, AddDoctorSuccess) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");
  dept.addDoctor("D002");
  dept.addDoctor("D003");

  EXPECT_EQ(dept.getDoctorCount(), 3);
  EXPECT_TRUE(dept.hasDoctor("D001"));
  EXPECT_TRUE(dept.hasDoctor("D002"));
  EXPECT_TRUE(dept.hasDoctor("D003"));
}

TEST(DepartmentTest, AddDoctorEmptyIgnored) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("");
  EXPECT_EQ(dept.getDoctorCount(), 0);
}

TEST(DepartmentTest, AddDoctorDuplicateIgnored) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");
  dept.addDoctor("D001"); // Duplicate
  dept.addDoctor("D001"); // Another duplicate

  EXPECT_EQ(dept.getDoctorCount(), 1);
}

TEST(DepartmentTest, RemoveDoctorSuccess) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");
  dept.addDoctor("D002");
  dept.addDoctor("D003");

  dept.removeDoctor("D002");

  EXPECT_EQ(dept.getDoctorCount(), 2);
  EXPECT_TRUE(dept.hasDoctor("D001"));
  EXPECT_FALSE(dept.hasDoctor("D002"));
  EXPECT_TRUE(dept.hasDoctor("D003"));
}

TEST(DepartmentTest, RemoveDoctorNotFound) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");

  // Removing non-existent doctor should not cause issues
  dept.removeDoctor("D999");

  EXPECT_EQ(dept.getDoctorCount(), 1);
  EXPECT_TRUE(dept.hasDoctor("D001"));
}

TEST(DepartmentTest, HasDoctorFalse) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");

  EXPECT_FALSE(dept.hasDoctor("D999"));
}

TEST(DepartmentTest, GetDoctorIDs) {
  Department dept("DEP001", "Test", "Desc", "D001");

  dept.addDoctor("D001");
  dept.addDoctor("D002");
  dept.addDoctor("D003");

  std::vector<std::string> doctorIDs = dept.getDoctorIDs();

  EXPECT_EQ(doctorIDs.size(), 3);
}

TEST(DepartmentTest, GetDoctorCountEmpty) {
  Department dept("DEP001", "Test", "Desc", "D001");

  EXPECT_EQ(dept.getDoctorCount(), 0);
}

// ==================== Serialization Tests ====================

TEST(DepartmentTest, SerializeReturnsCorrectFormat) {
  Department dept("DEP001", "Cardiology", "Heart care", "D001");
  dept.addDoctor("D001");
  dept.addDoctor("D005");
  dept.addDoctor("D012");
  dept.setLocation("Building A, Floor 2");
  dept.setPhone("0281234567");

  std::string serialized = dept.serialize();

  EXPECT_NE(serialized.find("DEP001"), std::string::npos);
  EXPECT_NE(serialized.find("Cardiology"), std::string::npos);
  EXPECT_NE(serialized.find("D001,D005,D012"), std::string::npos);
  EXPECT_NE(serialized.find("Building A, Floor 2"), std::string::npos);
  EXPECT_NE(serialized.find("0281234567"), std::string::npos);

  // Count pipes (should be 6 for 7 fields)
  int pipes = 0;
  for (char c : serialized) {
    if (c == '|')
      pipes++;
  }
  EXPECT_EQ(pipes, 6);
}

TEST(DepartmentTest, SerializeEmptyDoctors) {
  Department dept("DEP001", "Cardiology", "Heart care", "D001");
  dept.setLocation("Building A");
  dept.setPhone("0281234567");

  std::string serialized = dept.serialize();

  // Should have proper format even with empty doctor list
  EXPECT_NE(serialized.find("DEP001"), std::string::npos);
}

// ==================== Deserialization Tests ====================

TEST(DepartmentTest, DeserializeValid) {
  std::string line = "DEP001|Cardiology|Heart and cardiovascular "
                     "care|D001|D001,D005,D012|Building A Floor 2|0281234567";

  auto result = Department::deserialize(line);

  ASSERT_TRUE(result.has_value());

  Department dept = result.value();
  EXPECT_EQ(dept.getDepartmentID(), "DEP001");
  EXPECT_EQ(dept.getName(), "Cardiology");
  EXPECT_EQ(dept.getDescription(), "Heart and cardiovascular care");
  EXPECT_EQ(dept.getHeadDoctorID(), "D001");
  EXPECT_EQ(dept.getDoctorCount(), 3);
  EXPECT_TRUE(dept.hasDoctor("D001"));
  EXPECT_TRUE(dept.hasDoctor("D005"));
  EXPECT_TRUE(dept.hasDoctor("D012"));
  EXPECT_EQ(dept.getLocation(), "Building A Floor 2");
  EXPECT_EQ(dept.getPhone(), "0281234567");
}

TEST(DepartmentTest, DeserializeEmptyDoctorList) {
  std::string line = "DEP001|Cardiology|Heart care|D001||Building A|0281234567";

  auto result = Department::deserialize(line);

  ASSERT_TRUE(result.has_value());

  Department dept = result.value();
  EXPECT_EQ(dept.getDoctorCount(), 0);
}

TEST(DepartmentTest, DeserializeEmptyReturnsNullopt) {
  auto result = Department::deserialize("");
  EXPECT_FALSE(result.has_value());
}

TEST(DepartmentTest, DeserializeCommentReturnsNullopt) {
  auto result = Department::deserialize("#DEP001|Cardiology|...");
  EXPECT_FALSE(result.has_value());
}

TEST(DepartmentTest, DeserializeInvalidFieldCountReturnsNullopt) {
  std::string line = "DEP001|Cardiology|Heart care"; // Not enough fields

  auto result = Department::deserialize(line);
  EXPECT_FALSE(result.has_value());
}

TEST(DepartmentTest, DeserializeInvalidPhoneReturnsNullopt) {
  std::string line =
      "DEP001|Cardiology|Heart care|D001|D001,D002|Building A|invalid-phone";

  auto result = Department::deserialize(line);
  EXPECT_FALSE(result.has_value());
}

TEST(DepartmentTest, DeserializeEmptyPhoneAllowed) {
  std::string line = "DEP001|Cardiology|Heart care|D001|D001,D002|Building A|";

  auto result = Department::deserialize(line);

  ASSERT_TRUE(result.has_value());

  Department dept = result.value();
  EXPECT_EQ(dept.getPhone(), "");
}

// ==================== Round-trip Serialization ====================

TEST(DepartmentTest, RoundTripSerialization) {
  Department original("DEP001", "Cardiology", "Heart care", "D001");
  original.addDoctor("D001");
  original.addDoctor("D005");
  original.addDoctor("D012");
  original.setLocation("Building A Floor 2");
  original.setPhone("0281234567");

  std::string serialized = original.serialize();
  auto result = Department::deserialize(serialized);

  ASSERT_TRUE(result.has_value());

  Department restored = result.value();
  EXPECT_EQ(restored.getDepartmentID(), original.getDepartmentID());
  EXPECT_EQ(restored.getName(), original.getName());
  EXPECT_EQ(restored.getDescription(), original.getDescription());
  EXPECT_EQ(restored.getHeadDoctorID(), original.getHeadDoctorID());
  EXPECT_EQ(restored.getDoctorCount(), original.getDoctorCount());
  EXPECT_EQ(restored.getLocation(), original.getLocation());
  EXPECT_EQ(restored.getPhone(), original.getPhone());
}

TEST(DepartmentTest, RoundTripSerializationEmptyDoctors) {
  Department original("DEP001", "Cardiology", "Heart care", "D001");
  original.setLocation("Building A");
  original.setPhone("0281234567");

  std::string serialized = original.serialize();
  auto result = Department::deserialize(serialized);

  ASSERT_TRUE(result.has_value());

  Department restored = result.value();
  EXPECT_EQ(restored.getDoctorCount(), 0);
}

/*
cd build && ./HospitalTests --gtest_filter="DepartmentTest.*"
*/
