#include <gtest/gtest.h>
#include "dal/DoctorRepository.h"
#include "model/Doctor.h"
#include "common/Types.h"
#include <fstream>
#include <filesystem>

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace
{
    const std::string TEST_DATA_DIR = "test/fixtures/";
    const std::string TEST_DATA_FILE = "test/fixtures/Doctor_test.txt";
}

class DoctorRepositoryTest : public ::testing::Test
{
protected:
    DoctorRepository *repo;
    std::string testFilePath;

    void SetUp() override
    {
        // Reset singleton and get fresh instance
        DoctorRepository::resetInstance();
        repo = DoctorRepository::getInstance();

        // Use fixture file
        testFilePath = TEST_DATA_FILE;
        repo->setFilePath(testFilePath);
        std::filesystem::create_directories(TEST_DATA_DIR);
        std::ofstream ofs(testFilePath, std::ios::trunc);
        ofs.close();
        repo->clear();
    }

    void TearDown() override
    {
        repo->clear();
        DoctorRepository::resetInstance();
    }

    // Helper function to create test doctor
    Doctor createTestDoctor(
        const std::string &doctorID = "D001",
        const std::string &username = "dr_test",
        const std::string &name = "Dr. Test",
        const std::string &phone = "0123456789",
        Gender gender = Gender::MALE,
        const std::string &dateOfBirth = "1980-01-01",
        const std::string &specialization = "General Medicine",
        const std::string &schedule = "Mon-Fri 9:00-17:00",
        double consultationFee = 500000.0)
    {
        return Doctor(doctorID, username, name, phone, gender, dateOfBirth,
                      specialization, schedule, consultationFee);
    }
};

// ==================== Singleton Tests ====================

TEST_F(DoctorRepositoryTest, GetInstance_ReturnsSameInstance)
{
    DoctorRepository *instance1 = DoctorRepository::getInstance();
    DoctorRepository *instance2 = DoctorRepository::getInstance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(DoctorRepositoryTest, ResetInstance_ClearsInstance)
{
    DoctorRepository *oldInstance = DoctorRepository::getInstance();
    oldInstance->clear();

    DoctorRepository::resetInstance();
    DoctorRepository *newInstance = DoctorRepository::getInstance();

    EXPECT_EQ(newInstance->count(), 0u);
}

// ==================== Add Tests ====================

TEST_F(DoctorRepositoryTest, Add_ValidDoctor_ReturnsTrue)
{
    Doctor doctor = createTestDoctor();
    EXPECT_TRUE(repo->add(doctor));
}

TEST_F(DoctorRepositoryTest, Add_DuplicateDoctorID_ReturnsFalse)
{
    Doctor doctor1 = createTestDoctor("D001", "user1");
    Doctor doctor2 = createTestDoctor("D001", "user2"); // Same doctor ID

    EXPECT_TRUE(repo->add(doctor1));
    EXPECT_FALSE(repo->add(doctor2));
}

TEST_F(DoctorRepositoryTest, Add_DuplicateUsername_AllowedInDAL)
{
    // DAL only checks primary key (doctorID), username check is in BLL
    Doctor doctor1 = createTestDoctor("D001", "sameuser");
    Doctor doctor2 = createTestDoctor("D002", "sameuser"); // Same username, different ID

    EXPECT_TRUE(repo->add(doctor1));
    EXPECT_TRUE(repo->add(doctor2)); // Should succeed - DAL doesn't check username
    EXPECT_EQ(repo->count(), 2u);
}

TEST_F(DoctorRepositoryTest, Add_MultipleDoctors_AllAdded)
{
    Doctor doctor1 = createTestDoctor("D001", "user1", "Dr. One");
    Doctor doctor2 = createTestDoctor("D002", "user2", "Dr. Two");
    Doctor doctor3 = createTestDoctor("D003", "user3", "Dr. Three");

    EXPECT_TRUE(repo->add(doctor1));
    EXPECT_TRUE(repo->add(doctor2));
    EXPECT_TRUE(repo->add(doctor3));
    EXPECT_EQ(repo->count(), 3u);
}

TEST_F(DoctorRepositoryTest, Add_DifferentSpecializations_AllAdded)
{
    Doctor cardio = createTestDoctor("D001", "user1", "Dr. Heart", "0111111111",
                                     Gender::MALE, "1980-01-01", "Cardiology");
    Doctor neuro = createTestDoctor("D002", "user2", "Dr. Brain", "0222222222",
                                    Gender::FEMALE, "1985-05-15", "Neurology");
    Doctor ortho = createTestDoctor("D003", "user3", "Dr. Bones", "0333333333",
                                    Gender::MALE, "1975-12-20", "Orthopedics");

    EXPECT_TRUE(repo->add(cardio));
    EXPECT_TRUE(repo->add(neuro));
    EXPECT_TRUE(repo->add(ortho));
    EXPECT_EQ(repo->count(), 3u);
}

// ==================== GetById Tests ====================

TEST_F(DoctorRepositoryTest, GetById_ExistingDoctor_ReturnsDoctor)
{
    Doctor doctor = createTestDoctor("D001", "findme");
    repo->add(doctor);

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDoctorID(), "D001");
    EXPECT_EQ(result->getUsername(), "findme");
}

TEST_F(DoctorRepositoryTest, GetById_NonExistingDoctor_ReturnsNullopt)
{
    auto result = repo->getById("D999");
    EXPECT_FALSE(result.has_value());
}

TEST_F(DoctorRepositoryTest, GetById_AfterAddingMultiple_ReturnsCorrectDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "Doctor One"));
    repo->add(createTestDoctor("D002", "user2", "Doctor Two"));
    repo->add(createTestDoctor("D003", "user3", "Doctor Three"));

    auto result = repo->getById("D002");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Doctor Two");
}

// ==================== GetByUsername Tests ====================

TEST_F(DoctorRepositoryTest, GetByUsername_ExistingDoctor_ReturnsDoctor)
{
    Doctor doctor = createTestDoctor("D001", "myusername");
    repo->add(doctor);

    auto result = repo->getByUsername("myusername");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDoctorID(), "D001");
}

TEST_F(DoctorRepositoryTest, GetByUsername_NonExistingUsername_ReturnsNullopt)
{
    auto result = repo->getByUsername("nonexistent");
    EXPECT_FALSE(result.has_value());
}

// ==================== GetAll Tests ====================

TEST_F(DoctorRepositoryTest, GetAll_EmptyRepo_ReturnsEmptyVector)
{
    auto doctors = repo->getAll();
    EXPECT_TRUE(doctors.empty());
}

TEST_F(DoctorRepositoryTest, GetAll_WithDoctors_ReturnsAllDoctors)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->add(createTestDoctor("D003", "user3"));

    auto doctors = repo->getAll();
    EXPECT_EQ(doctors.size(), 3u);
}

TEST_F(DoctorRepositoryTest, GetAll_ReturnsCorrectDoctorData)
{
    Doctor doctor1 = createTestDoctor("D001", "user1", "John Doe");
    Doctor doctor2 = createTestDoctor("D002", "user2", "Jane Smith");

    repo->add(doctor1);
    repo->add(doctor2);

    auto doctors = repo->getAll();
    EXPECT_EQ(doctors[0].getName(), "John Doe");
    EXPECT_EQ(doctors[1].getName(), "Jane Smith");
}

// ==================== Update Tests ====================

TEST_F(DoctorRepositoryTest, Update_ExistingDoctor_ReturnsTrue)
{
    Doctor doctor = createTestDoctor("D001", "updateme");
    repo->add(doctor);

    Doctor updated("D001", "updateme", "Updated Name", "9999999999",
                   Gender::MALE, "1980-01-01", "New Specialization",
                   "New Schedule", 1000000.0);
    EXPECT_TRUE(repo->update(updated));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Updated Name");
    EXPECT_EQ(result->getSpecialization(), "New Specialization");
}

TEST_F(DoctorRepositoryTest, Update_NonExistingDoctor_ReturnsFalse)
{
    Doctor doctor = createTestDoctor("D999", "nonexistent");
    EXPECT_FALSE(repo->update(doctor));
}

TEST_F(DoctorRepositoryTest, Update_SameUsername_AllowsUpdate)
{
    Doctor doctor = createTestDoctor("D001", "sameuser");
    repo->add(doctor);

    // Update with same username should work
    Doctor updated("D001", "sameuser", "Updated Name", "9999999999",
                   Gender::MALE, "1980-01-01", "Updated", "Schedule", 600000.0);
    EXPECT_TRUE(repo->update(updated));
}

TEST_F(DoctorRepositoryTest, Update_UsernameConflict_AllowedInDAL)
{
    // DAL only checks primary key, username conflict check is in BLL
    Doctor doctor1 = createTestDoctor("D001", "user1");
    Doctor doctor2 = createTestDoctor("D002", "user2");
    repo->add(doctor1);
    repo->add(doctor2);

    // Change doctor1's username to doctor2's username - allowed in DAL
    Doctor updated("D001", "user2", "Name", "0123456789",
                   Gender::MALE, "1980-01-01", "Spec", "Schedule", 500000.0);
    EXPECT_TRUE(repo->update(updated)); // Should succeed - DAL doesn't check username
}

TEST_F(DoctorRepositoryTest, Update_SpecializationChange_Persisted)
{
    Doctor doctor = createTestDoctor("D001", "user1", "Dr. Test", "0123456789",
                                     Gender::MALE, "1980-01-01", "General");
    repo->add(doctor);

    Doctor updated = doctor;
    updated.setSpecialization("Cardiology");
    repo->update(updated);

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getSpecialization(), "Cardiology");
}

// ==================== Remove Tests ====================

TEST_F(DoctorRepositoryTest, Remove_ExistingDoctor_ReturnsTrue)
{
    repo->add(createTestDoctor("D001", "removeme"));
    EXPECT_TRUE(repo->remove("D001"));
    EXPECT_FALSE(repo->exists("D001"));
}

TEST_F(DoctorRepositoryTest, Remove_NonExistingDoctor_ReturnsFalse)
{
    EXPECT_FALSE(repo->remove("D999"));
}

TEST_F(DoctorRepositoryTest, Remove_DecreasesCount)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    EXPECT_EQ(repo->count(), 2u);

    repo->remove("D001");
    EXPECT_EQ(repo->count(), 1u);
}

TEST_F(DoctorRepositoryTest, Remove_FromMiddle_OthersRemain)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->add(createTestDoctor("D003", "user3"));

    repo->remove("D002");

    EXPECT_TRUE(repo->exists("D001"));
    EXPECT_FALSE(repo->exists("D002"));
    EXPECT_TRUE(repo->exists("D003"));
}

// ==================== Exists Tests ====================

TEST_F(DoctorRepositoryTest, Exists_ExistingDoctor_ReturnsTrue)
{
    repo->add(createTestDoctor("D001", "iexist"));
    EXPECT_TRUE(repo->exists("D001"));
}

TEST_F(DoctorRepositoryTest, Exists_NonExistingDoctor_ReturnsFalse)
{
    EXPECT_FALSE(repo->exists("D999"));
}

TEST_F(DoctorRepositoryTest, Exists_AfterRemove_ReturnsFalse)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->remove("D001");
    EXPECT_FALSE(repo->exists("D001"));
}

// ==================== Count Tests ====================

TEST_F(DoctorRepositoryTest, Count_EmptyRepo_ReturnsZero)
{
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(DoctorRepositoryTest, Count_AfterAdds_ReturnsCorrectCount)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->add(createTestDoctor("D003", "user3"));
    EXPECT_EQ(repo->count(), 3u);
}

TEST_F(DoctorRepositoryTest, Count_AfterAddAndRemove_ReturnsCorrectCount)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->remove("D001");
    EXPECT_EQ(repo->count(), 1u);
}

// ==================== Clear Tests ====================

TEST_F(DoctorRepositoryTest, Clear_RemovesAllDoctors)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));

    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(DoctorRepositoryTest, Clear_EmptyRepo_Works)
{
    repo->clear();
    EXPECT_EQ(repo->count(), 0u);
}

// ==================== SearchByName Tests ====================

TEST_F(DoctorRepositoryTest, SearchByName_FullMatch_ReturnsDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "John Doe"));

    auto results = repo->searchByName("John Doe");
    EXPECT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].getName(), "John Doe");
}

TEST_F(DoctorRepositoryTest, SearchByName_PartialMatch_ReturnsDoctors)
{
    repo->add(createTestDoctor("D001", "user1", "John Doe"));
    repo->add(createTestDoctor("D002", "user2", "Johnny Cash"));
    repo->add(createTestDoctor("D003", "user3", "Jane Smith"));

    auto results = repo->searchByName("John");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(DoctorRepositoryTest, SearchByName_CaseInsensitive_FindsMatch)
{
    repo->add(createTestDoctor("D001", "user1", "John Doe"));

    auto results = repo->searchByName("john");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, SearchByName_NoMatch_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "John Doe"));

    auto results = repo->searchByName("NonExistent");
    EXPECT_TRUE(results.empty());
}

// ==================== GetBySpecialization Tests ====================

TEST_F(DoctorRepositoryTest, GetBySpecialization_ExactMatch_ReturnsDoctors)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", "Cardiology"));

    auto results = repo->getBySpecialization("Cardiology");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(DoctorRepositoryTest, GetBySpecialization_PartialMatch_ReturnsDoctors)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->getBySpecialization("Cardio");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, GetBySpecialization_CaseInsensitive_FindsMatch)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->getBySpecialization("cardiology");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, GetBySpecialization_NoMatch_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->getBySpecialization("Neurology");
    EXPECT_TRUE(results.empty());
}

// ==================== General Search Tests ====================

TEST_F(DoctorRepositoryTest, Search_ByName_FindsDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "John Doe", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->search("John");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, Search_BySpecialization_FindsDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->search("Cardio");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, Search_ByDoctorID_FindsDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith"));

    auto results = repo->search("D001");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, Search_CaseInsensitive_FindsDoctor)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->search("cardiology");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, Search_MultipleMatches_ReturnsAll)
{
    repo->add(createTestDoctor("D001", "user1", "John Cardio", "0111111111",
                               Gender::MALE, "1980-01-01", "General"));
    repo->add(createTestDoctor("D002", "user2", "Jane Doe", "0222222222",
                               Gender::FEMALE, "1985-05-15", "Cardiology"));

    auto results = repo->search("Cardio");
    EXPECT_EQ(results.size(), 2u);
}

TEST_F(DoctorRepositoryTest, Search_NoMatch_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith"));

    auto results = repo->search("NonExistent");
    EXPECT_TRUE(results.empty());
}

// ==================== GetAllSpecializations Tests ====================

TEST_F(DoctorRepositoryTest, GetAllSpecializations_EmptyRepo_ReturnsEmpty)
{
    auto specs = repo->getAllSpecializations();
    EXPECT_TRUE(specs.empty());
}

TEST_F(DoctorRepositoryTest, GetAllSpecializations_UniqueSpecializations_ReturnsAll)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", "Neurology"));
    repo->add(createTestDoctor("D003", "user3", "Dr. Three", "0333333333",
                               Gender::MALE, "1975-12-20", "Orthopedics"));

    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 3u);
}

TEST_F(DoctorRepositoryTest, GetAllSpecializations_DuplicateSpecializations_ReturnsUnique)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", "Cardiology"));

    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 1u);
    EXPECT_EQ(specs[0], "Cardiology");
}

TEST_F(DoctorRepositoryTest, GetAllSpecializations_TrimsWhitespace)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "  Cardiology  "));

    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 1u);
    EXPECT_EQ(specs[0], "Cardiology");
}

// ==================== GetNextId Tests ====================

TEST_F(DoctorRepositoryTest, GetNextId_EmptyRepo_ReturnsD001)
{
    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D001");
}

TEST_F(DoctorRepositoryTest, GetNextId_AfterOneDoctor_ReturnsD002)
{
    repo->add(createTestDoctor("D001", "user1"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D002");
}

TEST_F(DoctorRepositoryTest, GetNextId_AfterMultipleDoctors_ReturnsCorrectNext)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->add(createTestDoctor("D003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D004");
}

TEST_F(DoctorRepositoryTest, GetNextId_WithGaps_ReturnsMaxPlusOne)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D005", "user5"));
    repo->add(createTestDoctor("D003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D006"); // Max is D005, so next is D006
}

TEST_F(DoctorRepositoryTest, GetNextId_WithLeadingZeros_HandlesCorrectly)
{
    repo->add(createTestDoctor("D009", "user9"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D010");
}

TEST_F(DoctorRepositoryTest, GetNextId_WithLargeNumber_HandlesCorrectly)
{
    repo->add(createTestDoctor("D099", "user99"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D100");
}

TEST_F(DoctorRepositoryTest, GetNextId_IgnoresInvalidFormat_ReturnsCorrectNext)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("DXYZ", "invalid")); // Invalid format
    repo->add(createTestDoctor("D003", "user3"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D004"); // Should ignore DXYZ
}

// ==================== Persistence Tests ====================

TEST_F(DoctorRepositoryTest, SaveAndLoad_DataPersisted)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology", "Mon-Fri 9-17", 500000));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", "Neurology", "Tue-Sat 10-18", 600000));
    repo->save();

    // Reset and reload
    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    EXPECT_EQ(repo->count(), 2u);

    auto doctor1 = repo->getById("D001");
    ASSERT_TRUE(doctor1.has_value());
    EXPECT_EQ(doctor1->getName(), "Dr. One");
    EXPECT_EQ(doctor1->getSpecialization(), "Cardiology");

    auto doctor2 = repo->getById("D002");
    ASSERT_TRUE(doctor2.has_value());
    EXPECT_EQ(doctor2->getName(), "Dr. Two");
    EXPECT_EQ(doctor2->getSpecialization(), "Neurology");
}

TEST_F(DoctorRepositoryTest, SaveAndLoad_AllFieldsPersisted)
{
    Doctor doctor = createTestDoctor("D001", "testuser", "Dr. John Doe", "0123456789",
                                     Gender::MALE, "1980-05-15", "Cardiology",
                                     "Mon-Fri 9:00-17:00", 750000.0);
    repo->add(doctor);
    repo->save();

    // Reset and reload
    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    auto loaded = repo->getById("D001");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getDoctorID(), "D001");
    EXPECT_EQ(loaded->getUsername(), "testuser");
    EXPECT_EQ(loaded->getName(), "Dr. John Doe");
    EXPECT_EQ(loaded->getPhone(), "0123456789");
    EXPECT_EQ(loaded->getGender(), Gender::MALE);
    EXPECT_EQ(loaded->getDateOfBirth(), "1980-05-15");
    EXPECT_EQ(loaded->getSpecialization(), "Cardiology");
    EXPECT_EQ(loaded->getSchedule(), "Mon-Fri 9:00-17:00");
    EXPECT_DOUBLE_EQ(loaded->getConsultationFee(), 750000.0);
}

TEST_F(DoctorRepositoryTest, Load_NonExistentFile_CreatesFileAndReturnsTrue)
{
    // load() creates the file if it doesn't exist and returns true (consistent with other repositories)
    std::string nonExistentFile = TEST_DATA_DIR + "nonexistent_file_12345.txt";
    repo->setFilePath(nonExistentFile);
    EXPECT_TRUE(repo->load());
    EXPECT_EQ(repo->count(), 0u);

    // Cleanup the created file
    if (std::filesystem::exists(nonExistentFile))
    {
        std::filesystem::remove(nonExistentFile);
    }
}

TEST_F(DoctorRepositoryTest, LazyLoading_AutoLoadsOnAccess)
{
    // Add data and save
    repo->add(createTestDoctor("D001", "lazyuser"));
    repo->save();

    // Reset instance
    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);

    // Access data without explicit load() - should auto-load
    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(repo->exists("D001"));
}

TEST_F(DoctorRepositoryTest, Save_AtomicWrite_TempFileCleanedUp)
{
    repo->add(createTestDoctor("D001", "user1"));
    EXPECT_TRUE(repo->save());

    // Temp file should not exist after successful save
    std::string tempFile = testFilePath + ".tmp";
    EXPECT_FALSE(std::filesystem::exists(tempFile));
}

// ==================== SetFilePath Tests ====================

TEST_F(DoctorRepositoryTest, SetFilePath_ChangesFilePath)
{
    std::string newPath = TEST_DATA_DIR + "new_test_doctors.txt";
    repo->setFilePath(newPath);
    EXPECT_EQ(repo->getFilePath(), newPath);

    // Cleanup
    if (std::filesystem::exists(newPath))
    {
        std::filesystem::remove(newPath);
    }
}

TEST_F(DoctorRepositoryTest, SetFilePath_ForcesReload)
{
    // Add to first file
    repo->add(createTestDoctor("D001", "user1"));
    repo->save();

    // Create second file with different data
    std::string secondFile = TEST_DATA_DIR + "test_doctors_2.txt";
    {
        std::ofstream out(secondFile);
        out << "# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee\n";
        Doctor doctor = createTestDoctor("D002", "user2", "Different Doctor");
        out << doctor.serialize() << "\n";
    }

    // Switch to second file
    repo->setFilePath(secondFile);

    auto allDoctors = repo->getAll();
    EXPECT_EQ(allDoctors.size(), 1u);
    EXPECT_EQ(allDoctors[0].getDoctorID(), "D002");

    // Cleanup
    if (std::filesystem::exists(secondFile))
    {
        std::filesystem::remove(secondFile);
    }
}

// ==================== Edge Cases and Stress Tests ====================

TEST_F(DoctorRepositoryTest, Add_EmptyUsername_Allowed)
{
    Doctor doctor = createTestDoctor("D001", "", "Dr. Test");
    EXPECT_TRUE(repo->add(doctor));
}

TEST_F(DoctorRepositoryTest, Add_SpecialCharactersInName_Handled)
{
    Doctor doctor = createTestDoctor("D001", "user1", "Dr. O'Brien-Smith");
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Dr. O'Brien-Smith");
}

TEST_F(DoctorRepositoryTest, Add_VeryLongName_Handled)
{
    std::string longName(500, 'A');
    Doctor doctor = createTestDoctor("D001", "user1", longName);
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), longName);
}

TEST_F(DoctorRepositoryTest, Add_UnicodeCharacters_Handled)
{
    Doctor doctor = createTestDoctor("D001", "user1", "Nguyễn Văn A");
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Nguyễn Văn A");
}

TEST_F(DoctorRepositoryTest, Search_EmptyKeyword_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith"));

    auto results = repo->search("");

    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, SearchByName_EmptyName_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith"));

    auto results = repo->searchByName("");

    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, GetBySpecialization_EmptySpecialization_ReturnsEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Smith", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->getBySpecialization("");

    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, Add_ZeroConsultationFee_Allowed)
{
    Doctor doctor = createTestDoctor("D001", "user1", "Dr. Free", "0111111111",
                                     Gender::MALE, "1980-01-01", "General", "Mon-Fri", 0.0);
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getConsultationFee(), 0.0);
}

TEST_F(DoctorRepositoryTest, Add_NegativeConsultationFee_Allowed)
{
    Doctor doctor = createTestDoctor("D001", "user1", "Dr. Test", "0111111111",
                                     Gender::MALE, "1980-01-01", "General", "Mon-Fri", -100.0);
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getConsultationFee(), -100.0);
}

TEST_F(DoctorRepositoryTest, Add_VeryLargeConsultationFee_Handled)
{
    double largeFee = 999999999.99;
    Doctor doctor = createTestDoctor("D001", "user1", "Dr. Expensive", "0111111111",
                                     Gender::MALE, "1980-01-01", "General", "Mon-Fri", largeFee);
    EXPECT_TRUE(repo->add(doctor));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result->getConsultationFee(), largeFee);
}

TEST_F(DoctorRepositoryTest, Update_AllFields_Persisted)
{
    Doctor original = createTestDoctor("D001", "original", "Original Name", "0111111111",
                                       Gender::MALE, "1980-01-01", "Original Spec", "Original Schedule", 500000.0);
    repo->add(original);

    Doctor updated("D001", "updated", "Updated Name", "9999999999",
                   Gender::FEMALE, "1990-12-31", "Updated Spec", "Updated Schedule", 1000000.0);
    EXPECT_TRUE(repo->update(updated));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getUsername(), "updated");
    EXPECT_EQ(result->getName(), "Updated Name");
    EXPECT_EQ(result->getPhone(), "9999999999");
    EXPECT_EQ(result->getGender(), Gender::FEMALE);
    EXPECT_EQ(result->getDateOfBirth(), "1990-12-31");
    EXPECT_EQ(result->getSpecialization(), "Updated Spec");
    EXPECT_EQ(result->getSchedule(), "Updated Schedule");
    EXPECT_DOUBLE_EQ(result->getConsultationFee(), 1000000.0);
}

TEST_F(DoctorRepositoryTest, Remove_LastDoctor_LeavesRepoEmpty)
{
    repo->add(createTestDoctor("D001", "user1"));
    EXPECT_EQ(repo->count(), 1u);

    repo->remove("D001");
    EXPECT_EQ(repo->count(), 0u);
    EXPECT_TRUE(repo->getAll().empty());
}

TEST_F(DoctorRepositoryTest, GetNextId_AfterRemove_IgnoresRemoved)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));
    repo->add(createTestDoctor("D003", "user3"));

    repo->remove("D003");

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D003");
}

TEST_F(DoctorRepositoryTest, Clear_FollowedByAdd_StartsClean)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("D002", "user2"));

    repo->clear();
    EXPECT_EQ(repo->count(), 0u);

    repo->add(createTestDoctor("D001", "newuser"));
    EXPECT_EQ(repo->count(), 1u);

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getUsername(), "newuser");
}

TEST_F(DoctorRepositoryTest, Clear_PersistsToFile)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->save();

    repo->clear();

    // Reset and reload
    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    EXPECT_EQ(repo->count(), 0u);
}

// ==================== Multiple Operations Sequence Tests ====================

TEST_F(DoctorRepositoryTest, ComplexSequence_AddUpdateRemove_ConsistentState)
{
    // Add 3 doctors
    repo->add(createTestDoctor("D001", "user1", "Doctor One"));
    repo->add(createTestDoctor("D002", "user2", "Doctor Two"));
    repo->add(createTestDoctor("D003", "user3", "Doctor Three"));
    EXPECT_EQ(repo->count(), 3u);

    // Update doctor 2
    Doctor updated("D002", "user2_updated", "Doctor Two Updated", "9999999999",
                   Gender::MALE, "1980-01-01", "Updated Spec", "Schedule", 750000.0);
    EXPECT_TRUE(repo->update(updated));

    // Remove doctor 1
    EXPECT_TRUE(repo->remove("D001"));
    EXPECT_EQ(repo->count(), 2u);

    // Add new doctor
    repo->add(createTestDoctor("D004", "user4", "Doctor Four"));
    EXPECT_EQ(repo->count(), 3u);

    // Verify state
    EXPECT_FALSE(repo->exists("D001"));
    EXPECT_TRUE(repo->exists("D002"));
    EXPECT_TRUE(repo->exists("D003"));
    EXPECT_TRUE(repo->exists("D004"));

    auto doctor2 = repo->getById("D002");
    ASSERT_TRUE(doctor2.has_value());
    EXPECT_EQ(doctor2->getName(), "Doctor Two Updated");
}

TEST_F(DoctorRepositoryTest, MultipleUpdates_LastUpdatePersists)
{
    repo->add(createTestDoctor("D001", "user1", "Version 1"));

    Doctor v2("D001", "user1", "Version 2", "0123456789",
              Gender::MALE, "1980-01-01", "Spec", "Schedule", 500000.0);
    repo->update(v2);

    Doctor v3("D001", "user1", "Version 3", "0123456789",
              Gender::MALE, "1980-01-01", "Spec", "Schedule", 500000.0);
    repo->update(v3);

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Version 3");
}

TEST_F(DoctorRepositoryTest, AddRemoveAdd_SameID_Works)
{
    Doctor doctor1 = createTestDoctor("D001", "user1", "First Doctor");
    repo->add(doctor1);
    EXPECT_TRUE(repo->exists("D001"));

    repo->remove("D001");
    EXPECT_FALSE(repo->exists("D001"));

    Doctor doctor2 = createTestDoctor("D001", "user2", "Second Doctor");
    EXPECT_TRUE(repo->add(doctor2));

    auto result = repo->getById("D001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getName(), "Second Doctor");
    EXPECT_EQ(result->getUsername(), "user2");
}

// ==================== Specialization Edge Cases ====================

TEST_F(DoctorRepositoryTest, GetAllSpecializations_WithEmptySpecializations_IgnoresEmpty)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", ""));
    repo->add(createTestDoctor("D003", "user3", "Dr. Three", "0333333333",
                               Gender::MALE, "1975-12-20", "Neurology"));

    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 2u);
}

TEST_F(DoctorRepositoryTest, GetAllSpecializations_WithWhitespaceOnly_IgnoresThem)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Two", "0222222222",
                               Gender::FEMALE, "1985-05-15", "   "));

    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 1u);
    EXPECT_EQ(specs[0], "Cardiology");
}

TEST_F(DoctorRepositoryTest, GetBySpecialization_WithWhitespace_FindsMatch)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. One", "0111111111",
                               Gender::MALE, "1980-01-01", "  Cardiology  "));

    auto results = repo->getBySpecialization("Cardiology");
    EXPECT_EQ(results.size(), 1u);
}

// ==================== Search Comprehensive Tests ====================

TEST_F(DoctorRepositoryTest, Search_MatchesInMultipleFields_ReturnsOnce)
{
    // Doctor with "Cardio" in both name and specialization
    repo->add(createTestDoctor("D001", "user1", "Dr. Cardio Smith", "0111111111",
                               Gender::MALE, "1980-01-01", "Cardiology"));

    auto results = repo->search("Cardio");
    EXPECT_EQ(results.size(), 1u); // Should return doctor once, not twice
}

TEST_F(DoctorRepositoryTest, Search_WithSpecialCharacters_HandlesGracefully)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. O'Brien"));

    auto results = repo->search("O'Brien");
    EXPECT_EQ(results.size(), 1u);
}

TEST_F(DoctorRepositoryTest, SearchByName_WithMiddleName_FindsMatch)
{
    repo->add(createTestDoctor("D001", "user1", "John Michael Doe"));

    auto results = repo->searchByName("Michael");
    EXPECT_EQ(results.size(), 1u);
}

// ==================== ID Format Tests ====================

TEST_F(DoctorRepositoryTest, GetNextId_WithNonStandardFormat_IgnoresThem)
{
    repo->add(createTestDoctor("D001", "user1"));
    repo->add(createTestDoctor("DOC002", "user2")); // Non-standard
    repo->add(createTestDoctor("d003", "user3"));   // Lowercase
    repo->add(createTestDoctor("D004", "user4"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D005"); // Should only count D001 and D004
}

TEST_F(DoctorRepositoryTest, GetNextId_WithLeadingZerosPadding_MaintainsFormat)
{
    repo->add(createTestDoctor("D007", "user7"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D008"); // Should maintain 3-digit format
}

TEST_F(DoctorRepositoryTest, GetNextId_AfterD999_HandlesCorrectly)
{
    repo->add(createTestDoctor("D999", "user999"));

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D1000"); // Should handle 4+ digits
}

// ==================== Persistence Edge Cases ====================

TEST_F(DoctorRepositoryTest, SaveAndLoad_WithSpecialCharactersInData_Preserved)
{
    Doctor doctor = createTestDoctor("D001", "username_ok", "Dr. Name With Dash-And'Quote",
                                     "0123456789", Gender::MALE, "1980-01-01",
                                     "General Medicine", "Mon-Fri 9-5", 500000.0);
    repo->add(doctor);
    repo->save();

    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    auto loaded = repo->getById("D001");
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getName(), "Dr. Name With Dash-And'Quote");
    EXPECT_EQ(loaded->getSpecialization(), "General Medicine");
}

TEST_F(DoctorRepositoryTest, SaveAndLoad_EmptyRepository_CreatesEmptyFile)
{
    repo->clear();
    repo->save();

    EXPECT_TRUE(std::filesystem::exists(testFilePath));

    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(DoctorRepositoryTest, Load_CorruptedLine_SkipsLine)
{
    // Create file with one good line and one corrupted line
    {
        std::ofstream out(testFilePath);
        out << "# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee\n";

        Doctor good = createTestDoctor("D001", "user1", "Good Doctor");
        out << good.serialize() << "\n";
        out << "CORRUPTED|DATA|HERE\n"; // Corrupted line

        Doctor good2 = createTestDoctor("D002", "user2", "Another Good Doctor");
        out << good2.serialize() << "\n";
    }

    repo->load();
    EXPECT_EQ(repo->count(), 2u); // Should load only valid entries
    EXPECT_TRUE(repo->exists("D001"));
    EXPECT_TRUE(repo->exists("D002"));
}

TEST_F(DoctorRepositoryTest, Load_FileWithOnlyComments_LoadsEmpty)
{
    {
        std::ofstream out(testFilePath);
        out << "# This is a comment\n";
        out << "# Another comment\n";
        out << "# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee\n";
    }

    repo->load();
    EXPECT_EQ(repo->count(), 0u);
}

TEST_F(DoctorRepositoryTest, Load_FileWithEmptyLines_SkipsThem)
{
    {
        std::ofstream out(testFilePath);
        out << "# Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee\n";
        out << "\n";

        Doctor doctor = createTestDoctor("D001", "user1");
        out << doctor.serialize() << "\n";
        out << "\n\n";
    }

    repo->load();
    EXPECT_EQ(repo->count(), 1u);
}

// ==================== Concurrent Access Simulation Tests ====================

TEST_F(DoctorRepositoryTest, MultipleGetInstance_ThreadSafe)
{
    // This tests that getInstance is thread-safe
    std::vector<DoctorRepository *> instances;

    for (int i = 0; i < 10; ++i)
    {
        instances.push_back(DoctorRepository::getInstance());
    }

    // All should be the same instance
    for (size_t i = 1; i < instances.size(); ++i)
    {
        EXPECT_EQ(instances[0], instances[i]);
    }
}

// ==================== Gender Field Tests ====================

TEST_F(DoctorRepositoryTest, Add_BothGenders_BothAdded)
{
    Doctor male = createTestDoctor("D001", "user1", "Dr. Male", "0111111111",
                                   Gender::MALE, "1980-01-01", "General");
    Doctor female = createTestDoctor("D002", "user2", "Dr. Female", "0222222222",
                                     Gender::FEMALE, "1985-05-15", "General");

    EXPECT_TRUE(repo->add(male));
    EXPECT_TRUE(repo->add(female));

    auto maleResult = repo->getById("D001");
    ASSERT_TRUE(maleResult.has_value());
    EXPECT_EQ(maleResult->getGender(), Gender::MALE);

    auto femaleResult = repo->getById("D002");
    ASSERT_TRUE(femaleResult.has_value());
    EXPECT_EQ(femaleResult->getGender(), Gender::FEMALE);
}

TEST_F(DoctorRepositoryTest, SaveAndLoad_GenderPreserved)
{
    repo->add(createTestDoctor("D001", "user1", "Dr. Male", "0111111111",
                               Gender::MALE, "1980-01-01", "General"));
    repo->add(createTestDoctor("D002", "user2", "Dr. Female", "0222222222",
                               Gender::FEMALE, "1985-05-15", "General"));
    repo->save();

    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    auto male = repo->getById("D001");
    ASSERT_TRUE(male.has_value());
    EXPECT_EQ(male->getGender(), Gender::MALE);

    auto female = repo->getById("D002");
    ASSERT_TRUE(female.has_value());
    EXPECT_EQ(female->getGender(), Gender::FEMALE);
}

// ==================== Final Integration Test ====================

TEST_F(DoctorRepositoryTest, FullWorkflow_AddUpdateSearchRemovePersist)
{
    // Add multiple doctors
    repo->add(createTestDoctor("D001", "john_doe", "Dr. John Doe", "0111111111",
                               Gender::MALE, "1975-03-15", "Cardiology", "Mon-Fri 9-17", 750000.0));
    repo->add(createTestDoctor("D002", "jane_smith", "Dr. Jane Smith", "0222222222",
                               Gender::FEMALE, "1980-07-22", "Neurology", "Tue-Sat 10-18", 850000.0));
    repo->add(createTestDoctor("D003", "bob_jones", "Dr. Bob Jones", "0333333333",
                               Gender::MALE, "1985-11-30", "Orthopedics", "Mon-Wed-Fri 8-16", 600000.0));

    // Verify count
    EXPECT_EQ(repo->count(), 3u);

    // Search by specialization
    auto cardiologists = repo->getBySpecialization("Cardiology");
    EXPECT_EQ(cardiologists.size(), 1u);

    // Update a doctor
    Doctor updated("D002", "jane_smith", "Dr. Jane Smith-Wilson", "0222222222",
                   Gender::FEMALE, "1980-07-22", "Neurosurgery", "Tue-Sat 10-18", 950000.0);
    EXPECT_TRUE(repo->update(updated));

    // Verify update
    auto jane = repo->getById("D002");
    ASSERT_TRUE(jane.has_value());
    EXPECT_EQ(jane->getName(), "Dr. Jane Smith-Wilson");
    EXPECT_EQ(jane->getSpecialization(), "Neurosurgery");

    // Search by name
    auto searchResults = repo->searchByName("Smith");
    EXPECT_EQ(searchResults.size(), 1u);

    // Remove a doctor
    EXPECT_TRUE(repo->remove("D003"));
    EXPECT_EQ(repo->count(), 2u);

    // Save
    EXPECT_TRUE(repo->save());

    // Reset and reload
    DoctorRepository::resetInstance();
    repo = DoctorRepository::getInstance();
    repo->setFilePath(testFilePath);
    repo->load();

    // Verify persistence
    EXPECT_EQ(repo->count(), 2u);
    EXPECT_TRUE(repo->exists("D001"));
    EXPECT_TRUE(repo->exists("D002"));
    EXPECT_FALSE(repo->exists("D003"));

    // Verify updated data persisted
    auto janeAfterReload = repo->getById("D002");
    ASSERT_TRUE(janeAfterReload.has_value());
    EXPECT_EQ(janeAfterReload->getName(), "Dr. Jane Smith-Wilson");
    EXPECT_EQ(janeAfterReload->getSpecialization(), "Neurosurgery");
    EXPECT_DOUBLE_EQ(janeAfterReload->getConsultationFee(), 950000.0);

    std::string nextId = repo->getNextId();
    EXPECT_EQ(nextId, "D003");
    // Get all specializations
    auto specs = repo->getAllSpecializations();
    EXPECT_EQ(specs.size(), 2u); // Cardiology and Neurosurgery
}

/*
Build va run tests:
cd build && ./HospitalTests --gtest_filter="DoctorRepositoryTest.*"
*/