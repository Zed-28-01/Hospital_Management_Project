#include <gtest/gtest.h>
#include "dal/PatientRepository.h"
#include "model/Patient.h"
#include "common/Types.h"
#include <fstream>
#include <filesystem>

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

class PatientRepositoryTest : public ::testing::Test {
protected:
    PatientRepository* repo;
    std::string testFilePath;

    void SetUp() override {
        // Reset singleton and get fresh instance
        PatientRepository::resetInstance();
        repo = PatientRepository::getInstance();

        // Use a test file
        testFilePath = "test_patients.txt";
        repo->setFilePath(testFilePath);
        repo->clear();
    }

    void TearDown() override {
        repo->clear();
        PatientRepository::resetInstance();

        // Clean up test file
        if (std::filesystem::exists(testFilePath)) {
            std::filesystem::remove(testFilePath);
        }
    }

    Patient createTestPatient(const std::string& patientID = "P001",
                              const std::string& username = "testuser",
                              const std::string& name = "Test Patient",
                              const std::string& phone = "0123456789",
                              Gender gender = Gender::MALE,
                              const std::string& dateOfBirth = "1990-01-01",
                              const std::string& address = "123 Test St",
                              const std::string& medicalHistory = "None") {
        return Patient(patientID, username, name, phone, gender, dateOfBirth, address, medicalHistory);
    }
};

// ==================== Singleton Tests ====================

TEST_F(PatientRepositoryTest, GetInstance_ReturnsSameInstance) {
    PatientRepository* instance1 = PatientRepository::getInstance();
    PatientRepository* instance2 = PatientRepository::getInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(PatientRepositoryTest, ResetInstance_ClearsInstance) {
    PatientRepository* oldInstance = PatientRepository::getInstance();
    oldInstance->clear();

    PatientRepository::resetInstance();
    PatientRepository* newInstance = PatientRepository::getInstance();

    // New instance should be in fresh state
    EXPECT_EQ(newInstance->count(), 0u);
}

// ==================== Add Tests ====================

TEST_F(PatientRepositoryTest, Add_ValidPatient_ReturnsTrue) {
    Patient patient = createTestPatient();
    EXPECT_TRUE(repo->add(patient));
}

TEST_F(PatientRepositoryTest, Add_DuplicatePatientID_ReturnsFalse) {
    Patient patient1 = createTestPatient("P001", "user1");
    Patient patient2 = createTestPatient("P001", "user2");  // Same patient ID

    EXPECT_TRUE(repo->add(patient1));
    EXPECT_FALSE(repo->add(patient2));
}

TEST_F(PatientRepositoryTest, Add_DuplicateUsername_ReturnsFalse) {
    Patient patient1 = createTestPatient("P001", "sameuser");
    Patient patient2 = createTestPatient("P002", "sameuser");  // Same username

    EXPECT_TRUE(repo->add(patient1));
    EXPECT_FALSE(repo->add(patient2));
}

TEST_F(PatientRepositoryTest, Add_MultiplePatients_AllAdded) {
    Patient patient1 = createTestPatient("P001", "user1", "John Doe");
    Patient patient2 = createTestPatient("P002", "user2", "Jane Smith");
    Patient patient3 = createTestPatient("P003", "user3", "Bob Johnson");

    EXPECT_TRUE(repo->add(patient1));
    EXPECT_TRUE(repo->add(patient2));
    EXPECT_TRUE(repo->add(patient3));
    EXPECT_EQ(repo->count(), 3u);
}

TEST_F(PatientRepositoryTest, Add_DifferentGenders_AllAdded) {
    Patient male = createTestPatient("P001", "male_user", "Male Patient", "0111111111", Gender::MALE);
    Patient female = createTestPatient("P002", "female_user", "Female Patient", "0222222222", Gender::FEMALE);
    Patient other = createTestPatient("P003", "other_user", "Other Patient", "0333333333", Gender::OTHER);

    EXPECT_TRUE(repo->add(male));
    EXPECT_TRUE(repo->add(female));
    EXPECT_TRUE(repo->add(other));
    EXPECT_EQ(repo->count(), 3u);
}

// ==================== GetById Tests ====================

TEST_F(PatientRepositoryTest, GetById_ExistingPatient_ReturnsPatient) {
    Patient patient = createTestPatient("P001", "findme");
    repo->add(patient);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientID(), "P001");
    EXPECT_EQ(result->getUsername(), "findme");
}

TEST_F(PatientRepositoryTest, GetById_NonExistingPatient_ReturnsNullopt) {
    auto result = repo->getById("P999");
    EXPECT_FALSE(result.has_value());
}

TEST_F(PatientRepositoryTest, GetById_AfterAddingMultiple_ReturnsCorrectPatient) {
    repo->add(createTestPatient("P001", "user1", "Patient One"));
    repo->add(createTestPatient("P002", "user2", "Patient Two"));
    repo->add(createTestPatient("P003", "user3", "Patient Three"));

    auto result = repo->getById("P002");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Patient Two");
}

// ==================== GetByUsername Tests ====================

TEST_F(PatientRepositoryTest, GetByUsername_ExistingPatient_ReturnsPatient) {
    Patient patient = createTestPatient("P001", "myusername");
    repo->add(patient);

    auto result = repo->getByUsername("myusername");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientID(), "P001");
}

TEST_F(PatientRepositoryTest, GetByUsername_NonExistingUsername_ReturnsNullopt) {
    auto result = repo->getByUsername("nonexistent");
    EXPECT_FALSE(result.has_value());
}

TEST_F(PatientRepositoryTest, GetByUsername_CaseSensitive_ReturnsNullopt) {
    repo->add(createTestPatient("P001", "TestUser"));

    auto result = repo->getByUsername("testuser");  // Different case
    EXPECT_FALSE(result.has_value());  // Should not find it (case sensitive)
}

// ==================== GetAll Tests ====================

TEST_F(PatientRepositoryTest, GetAll_EmptyRepo_ReturnsEmptyVector) {
    auto patients = repo->getAll();
    EXPECT_TRUE(patients.empty());
}

TEST_F(PatientRepositoryTest, GetAll_WithPatients_ReturnsAllPatients) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    repo->add(createTestPatient("P003", "user3"));

    auto patients = repo->getAll();
    EXPECT_EQ(patients.size(), 3u);
}

TEST_F(PatientRepositoryTest, GetAll_ReturnsCorrectPatientData) {
    Patient patient1 = createTestPatient("P001", "user1", "John Doe");
    Patient patient2 = createTestPatient("P002", "user2", "Jane Smith");

    repo->add(patient1);
    repo->add(patient2);

    auto patients = repo->getAll();
    EXPECT_EQ(patients[0].getName(), "John Doe");
    EXPECT_EQ(patients[1].getName(), "Jane Smith");
}

// ==================== Update Tests ====================

TEST_F(PatientRepositoryTest, Update_ExistingPatient_ReturnsTrue) {
    Patient patient = createTestPatient("P001", "updateme");
    repo->add(patient);

    Patient updated("P001", "updateme", "Updated Name", "9999999999",
                   Gender::MALE, "1990-01-01", "New Address", "Updated history");
    EXPECT_TRUE(repo->update(updated));

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Updated Name");
    EXPECT_EQ(result->getAddress(), "New Address");
}

TEST_F(PatientRepositoryTest, Update_NonExistingPatient_ReturnsFalse) {
    Patient patient = createTestPatient("P999", "nonexistent");
    EXPECT_FALSE(repo->update(patient));
}

TEST_F(PatientRepositoryTest, Update_PhoneNumber_Persisted) {
    Patient patient = createTestPatient("P001", "phonechange", "Test", "0111111111");
    repo->add(patient);

    Patient updated("P001", "phonechange", "Test", "0999999999",
                   Gender::MALE, "1990-01-01", "123 Test St", "None");
    repo->update(updated);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPhone(), "0999999999");
}

TEST_F(PatientRepositoryTest, Update_Address_Persisted) {
    Patient patient = createTestPatient("P001", "user1");
    repo->add(patient);

    Patient updated = patient;
    updated.setAddress("456 New Street");
    repo->update(updated);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAddress(), "456 New Street");
}

TEST_F(PatientRepositoryTest, Update_MedicalHistory_Persisted) {
    Patient patient = createTestPatient("P001", "user1");
    repo->add(patient);

    Patient updated = patient;
    updated.setMedicalHistory("Diabetes, Hypertension");
    repo->update(updated);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getMedicalHistory(), "Diabetes, Hypertension");
}

// ==================== Remove Tests ====================

TEST_F(PatientRepositoryTest, Remove_ExistingPatient_ReturnsTrue) {
    repo->add(createTestPatient("P001", "removeme"));
    EXPECT_TRUE(repo->remove("P001"));
    EXPECT_FALSE(repo->exists("P001"));
}

TEST_F(PatientRepositoryTest, Remove_NonExistingPatient_ReturnsFalse) {
    EXPECT_FALSE(repo->remove("P999"));
}

TEST_F(PatientRepositoryTest, Remove_DecreasesCount) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    EXPECT_EQ(repo->count(), 2u);

    repo->remove("P001");
    EXPECT_EQ(repo->count(), 1u);
}

TEST_F(PatientRepositoryTest, Remove_FromMiddle_OthersRemain) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    repo->add(createTestPatient("P003", "user3"));

    repo->remove("P002");

    EXPECT_TRUE(repo->exists("P001"));
    EXPECT_FALSE(repo->exists("P002"));
    EXPECT_TRUE(repo->exists("P003"));
}

// ==================== Exists Tests ====================

TEST_F(PatientRepositoryTest, Exists_ExistingPatient_ReturnsTrue) {
    repo->add(createTestPatient("P001", "iexist"));
    EXPECT_TRUE(repo->exists("P001"));
}

TEST_F(PatientRepositoryTest, Exists_NonExistingPatient_ReturnsFalse) {
    EXPECT_FALSE(repo->exists("P999"));
}

TEST_F(PatientRepositoryTest, Exists_AfterRemove_ReturnsFalse) {
    repo->add(createTestPatient("P001", "user1"));
    repo->remove("P001");
    EXPECT_FALSE(repo->exists("P001"));
}

// ==================== Count Tests ====================

TEST_F(PatientRepositoryTest, Count_EmptyRepo_ReturnsZero) {
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(PatientRepositoryTest, Count_AfterAdds_ReturnsCorrectCount) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    repo->add(createTestPatient("P003", "user3"));
    EXPECT_EQ(repo->count(), 3u);
}

TEST_F(PatientRepositoryTest, Count_AfterAddAndRemove_ReturnsCorrectCount) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    repo->remove("P001");
    EXPECT_EQ(repo->count(), 1u);
}

// ==================== Clear Tests ====================

TEST_F(PatientRepositoryTest, Clear_RemovesAllPatients) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));

    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(PatientRepositoryTest, Clear_EmptyRepo_Works) {
    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

// ==================== SearchByName Tests ====================

TEST_F(PatientRepositoryTest, SearchByName_FullMatch_ReturnsPatient) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));

    auto results = repo->searchByName("John Doe");
    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].getName(), "John Doe");
}

TEST_F(PatientRepositoryTest, SearchByName_PartialMatch_ReturnsPatients) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));
    repo->add(createTestPatient("P002", "user2", "Johnny Cash"));
    repo->add(createTestPatient("P003", "user3", "Jane Smith"));

    auto results = repo->searchByName("John");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(PatientRepositoryTest, SearchByName_CaseInsensitive_FindsMatch) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));

    auto results = repo->searchByName("john");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, SearchByName_NoMatch_ReturnsEmpty) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));

    auto results = repo->searchByName("NonExistent");
    EXPECT_TRUE(results.empty());
}

TEST_F(PatientRepositoryTest, SearchByName_EmptyString_ReturnsAll) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));
    repo->add(createTestPatient("P002", "user2", "Jane Smith"));

    auto results = repo->searchByName("");
    EXPECT_EQ(results.size(), 2u);
}

// ==================== SearchByPhone Tests ====================

TEST_F(PatientRepositoryTest, SearchByPhone_ExactMatch_ReturnsPatient) {
    repo->add(createTestPatient("P001", "user1", "John", "0123456789"));

    auto results = repo->searchByPhone("0123456789");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, SearchByPhone_PartialMatch_ReturnsPatients) {
    repo->add(createTestPatient("P001", "user1", "John", "0123456789"));
    repo->add(createTestPatient("P002", "user2", "Jane", "0123999999"));
    repo->add(createTestPatient("P003", "user3", "Bob", "0987654321"));

    auto results = repo->searchByPhone("0123");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(PatientRepositoryTest, SearchByPhone_NoMatch_ReturnsEmpty) {
    repo->add(createTestPatient("P001", "user1", "John", "0123456789"));

    auto results = repo->searchByPhone("9999");
    EXPECT_TRUE(results.empty());
}

TEST_F(PatientRepositoryTest, SearchByPhone_MiddleDigits_FindsMatch) {
    repo->add(createTestPatient("P001", "user1", "John", "0123456789"));

    auto results = repo->searchByPhone("345");
    EXPECT_EQ(results.size(), 1u);
}

// ==================== General Search Tests ====================

TEST_F(PatientRepositoryTest, Search_ByName_FindsPatient) {
    repo->add(createTestPatient("P001", "user1", "John Doe", "0111111111",
                                Gender::MALE, "1990-01-01", "123 Main St"));

    auto results = repo->search("John");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, Search_ByPhone_FindsPatient) {
    repo->add(createTestPatient("P001", "user1", "John Doe", "0123456789",
                                Gender::MALE, "1990-01-01", "123 Main St"));

    auto results = repo->search("0123");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, Search_ByAddress_FindsPatient) {
    repo->add(createTestPatient("P001", "user1", "John Doe", "0111111111",
                                Gender::MALE, "1990-01-01", "123 Main Street"));

    auto results = repo->search("Main");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, Search_CaseInsensitive_FindsPatient) {
    repo->add(createTestPatient("P001", "user1", "John Doe", "0111111111",
                                Gender::MALE, "1990-01-01", "123 Main Street"));

    auto results = repo->search("main");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(PatientRepositoryTest, Search_MultipleMatches_ReturnsAll) {
    repo->add(createTestPatient("P001", "user1", "John Main", "0111111111",
                                Gender::MALE, "1990-01-01", "456 Elm St"));
    repo->add(createTestPatient("P002", "user2", "Jane Doe", "0222222222",
                                Gender::FEMALE, "1992-02-02", "789 Main St"));

    auto results = repo->search("Main");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(PatientRepositoryTest, Search_NoMatch_ReturnsEmpty) {
    repo->add(createTestPatient("P001", "user1", "John Doe"));

    auto results = repo->search("NonExistent");
    EXPECT_TRUE(results.empty());
}

// ==================== GetNextId Tests ====================

TEST_F(PatientRepositoryTest, GetNextId_EmptyRepo_ReturnsP001) {
    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P001");
}

TEST_F(PatientRepositoryTest, GetNextId_AfterOnePatient_ReturnsP002) {
    repo->add(createTestPatient("P001", "user1"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P002");
}

TEST_F(PatientRepositoryTest, GetNextId_AfterMultiplePatients_ReturnsCorrectNext) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P002", "user2"));
    repo->add(createTestPatient("P003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P004");
}

TEST_F(PatientRepositoryTest, GetNextId_WithGaps_ReturnsMaxPlusOne) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("P005", "user5"));
    repo->add(createTestPatient("P003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P006");  // Max is P005, so next is P006
}

TEST_F(PatientRepositoryTest, GetNextId_WithLeadingZeros_HandlesCorrectly) {
    repo->add(createTestPatient("P009", "user9"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P010");
}

TEST_F(PatientRepositoryTest, GetNextId_WithLargeNumber_HandlesCorrectly) {
    repo->add(createTestPatient("P099", "user99"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P100");
}

TEST_F(PatientRepositoryTest, GetNextId_IgnoresInvalidFormat_ReturnsCorrectNext) {
    repo->add(createTestPatient("P001", "user1"));
    repo->add(createTestPatient("PXYZ", "invalid"));  // Invalid format
    repo->add(createTestPatient("P003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "P004");  // Should ignore PXYZ
}

// ==================== Persistence Tests ====================

TEST_F(PatientRepositoryTest, SaveAndLoad_DataPersisted) {
    repo->add(createTestPatient("P001", "persist1", "John Doe", "0111111111",
                                Gender::MALE, "1990-01-01", "123 Main St", "Diabetes"));
    repo->add(createTestPatient("P002", "persist2", "Jane Smith", "0222222222",
                                Gender::FEMALE, "1992-02-02", "456 Elm St", "None"));
    repo->save();

    // Reset and reload
    PatientRepository::resetInstance();
    repo = PatientRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    EXPECT_EQ(repo->count(), 2u);

    auto patient1 = repo->getById("P001");
    ASSERT_TRUE(patient1.has_value());
    EXPECT_EQ(patient1->getName(), "John Doe");
    EXPECT_EQ(patient1->getGender(), Gender::MALE);

    auto patient2 = repo->getById("P002");
    ASSERT_TRUE(patient2.has_value());
    EXPECT_EQ(patient2->getName(), "Jane Smith");
    EXPECT_EQ(patient2->getGender(), Gender::FEMALE);
}

TEST_F(PatientRepositoryTest, SaveAndLoad_AllFieldsPersisted) {
    Patient patient = createTestPatient("P001", "testuser", "John Doe", "0123456789",
                                       Gender::MALE, "1990-05-15", "789 Oak Ave",
                                       "Hypertension; Diabetes");
    repo->add(patient);
    repo->save();

    // Reset and reload
    PatientRepository::resetInstance();
    repo = PatientRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    auto loaded = repo->getById("P001");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getPatientID(), "P001");
    EXPECT_EQ(loaded->getUsername(), "testuser");
    EXPECT_EQ(loaded->getName(), "John Doe");
    EXPECT_EQ(loaded->getPhone(), "0123456789");
    EXPECT_EQ(loaded->getGender(), Gender::MALE);
    EXPECT_EQ(loaded->getDateOfBirth(), "1990-05-15");
    EXPECT_EQ(loaded->getAddress(), "789 Oak Ave");
    EXPECT_EQ(loaded->getMedicalHistory(), "Hypertension; Diabetes");
}

TEST_F(PatientRepositoryTest, Load_NonExistentFile_ReturnsFalse) {
    // Reset để có clean state
    PatientRepository::resetInstance();
    repo = PatientRepository::getInstance();

    std::string nonExistentFile = "nonexistent_file_12345.txt";

    // Đảm bảo file không tồn tại
    if (std::filesystem::exists(nonExistentFile)) {
        std::filesystem::remove(nonExistentFile);
    }

    repo->setFilePath(nonExistentFile);

    bool loaded = repo->load();
    EXPECT_FALSE(loaded);
    EXPECT_EQ(repo->count(), 0u);
}


TEST_F(PatientRepositoryTest, LazyLoading_AutoLoadsOnAccess) {
    repo->add(createTestPatient("P001", "lazyuser"));
    repo->save();

    PatientRepository::resetInstance();
    repo = PatientRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());

    EXPECT_TRUE(repo->exists("P001"));
}

TEST_F(PatientRepositoryTest, Save_EmptyRepo_CreatesEmptyFile) {
    repo->clear();
    EXPECT_TRUE(repo->save());

    // File should exist
    EXPECT_TRUE(std::filesystem::exists(testFilePath));
}

// ==================== SetFilePath Tests ====================

TEST_F(PatientRepositoryTest, SetFilePath_ChangesFilePath) {
    std::string newPath = "new_test_patients.txt";
    repo->setFilePath(newPath);
    EXPECT_EQ(repo->getFilePath(), newPath);

    // Cleanup
    if (std::filesystem::exists(newPath)) {
        std::filesystem::remove(newPath);
    }
}

TEST_F(PatientRepositoryTest, SetFilePath_ForcesReload) {
    // Add to first file
    repo->add(createTestPatient("P001", "user1"));
    repo->save();

    // Create second file with different data
    std::string secondFile = "test_patients_2.txt";
    {
        std::ofstream out(secondFile);
        out << "# Format: patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory\n";
        Patient patient = createTestPatient("P002", "user2", "Different Patient");
        out << patient.serialize() << "\n";
    }

    // Switch to second file
    repo->setFilePath(secondFile);

    auto allPatients = repo->getAll();

    EXPECT_EQ(allPatients.size(), 1u);
    EXPECT_EQ(allPatients[0].getPatientID(), "P002");

    // Cleanup
    if (std::filesystem::exists(secondFile)) {
        std::filesystem::remove(secondFile);
    }
}

TEST_F(PatientRepositoryTest, GetFilePath_ReturnsCorrectPath) {
    EXPECT_EQ(repo->getFilePath(), testFilePath);
}

// ==================== Edge Cases ====================

TEST_F(PatientRepositoryTest, Add_AfterClear_Works) {
    repo->add(createTestPatient("P001", "user1"));
    repo->clear();
    repo->add(createTestPatient("P002", "user2"));

    EXPECT_EQ(repo->count(), 1u);
    EXPECT_TRUE(repo->exists("P002"));
}

TEST_F(PatientRepositoryTest, Update_AfterRemove_ReturnsFalse) {
    Patient patient = createTestPatient("P001", "user1");
    repo->add(patient);
    repo->remove("P001");

    Patient updated = createTestPatient("P001", "user1", "Updated Name");
    EXPECT_FALSE(repo->update(updated));
}

TEST_F(PatientRepositoryTest, MultipleOperations_CorrectState) {
    // Add patients
    repo->add(createTestPatient("P001", "user1", "John"));
    repo->add(createTestPatient("P002", "user2", "Jane"));
    repo->add(createTestPatient("P003", "user3", "Bob"));
    EXPECT_EQ(repo->count(), 3u);

    // Remove one
    repo->remove("P002");
    EXPECT_EQ(repo->count(), 2u);

    // Update one
    Patient updated = createTestPatient("P001", "user1", "John Updated");
    repo->update(updated);

    // Verify final state
    EXPECT_FALSE(repo->exists("P002"));
    auto patient1 = repo->getById("P001");
    ASSERT_TRUE(patient1.has_value());
    EXPECT_EQ(patient1->getName(), "John Updated");
}

TEST_F(PatientRepositoryTest, SpecialCharactersInFields_HandledCorrectly) {
    Patient patient = createTestPatient("P001", "user@test", "O'Brien, John", "0123-456-789",
                                       Gender::MALE, "1990-01-01", "123 Main St, Apt #5",
                                       "Allergies: Penicillin; Conditions: None");
    repo->add(patient);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "O'Brien, John");
    EXPECT_EQ(result->getMedicalHistory(), "Allergies: Penicillin; Conditions: None");
}

TEST_F(PatientRepositoryTest, EmptyFields_HandledCorrectly) {
    Patient patient = createTestPatient("P001", "user1", "John", "0123456789",
                                       Gender::MALE, "1990-01-01", "", "");  // Empty address and history
    repo->add(patient);

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAddress(), "");
    EXPECT_EQ(result->getMedicalHistory(), "");
}

TEST_F(PatientRepositoryTest, LongMedicalHistory_HandledCorrectly) {
    std::string longHistory = "Diabetes Type 2 diagnosed 2010; Hypertension since 2015; "
                             "Allergic to Penicillin; Previous surgery appendectomy 2005; "
                             "Family history of heart disease; Currently on Metformin 500mg twice daily";

    Patient patient = createTestPatient("P001", "user1", "John", "0123456789",
                                       Gender::MALE, "1990-01-01", "123 Main St", longHistory);
    repo->add(patient);
    repo->save();

    // Reload
    PatientRepository::resetInstance();
    repo = PatientRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    auto result = repo->getById("P001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getMedicalHistory(), longHistory);
}

TEST_F(PatientRepositoryTest, GetAll_PreservesOrder) {
    repo->add(createTestPatient("P001", "user1", "First"));
    repo->add(createTestPatient("P002", "user2", "Second"));
    repo->add(createTestPatient("P003", "user3", "Third"));

    auto patients = repo->getAll();
    ASSERT_EQ(patients.size(), 3u);
    EXPECT_EQ(patients[0].getName(), "First");
    EXPECT_EQ(patients[1].getName(), "Second");
    EXPECT_EQ(patients[2].getName(), "Third");
}

TEST_F(PatientRepositoryTest, ConcurrentAccess_SingletonWorks) {
    PatientRepository* repo1 = PatientRepository::getInstance();
    PatientRepository* repo2 = PatientRepository::getInstance();

    repo1->add(createTestPatient("P001", "user1"));

    repo2->getAll();

    EXPECT_EQ(repo2->count(), 1u);
    EXPECT_TRUE(repo2->exists("P001"));
}
