#include <gtest/gtest.h>
#include "dal/PrescriptionRepository.h"
#include "advance/Prescription.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class PrescriptionRepositoryTest : public ::testing::Test
{
protected:
    HMS::DAL::PrescriptionRepository *repo;
    std::string testFilePath;

    void SetUp() override
    {
        // Create test fixtures directory
        fs::create_directories("test/fixtures");

        // Set test file path
        testFilePath = "test/fixtures/Prescription_test.txt";

        // Reset singleton and redirect to test file
        HMS::DAL::PrescriptionRepository::resetInstance();
        repo = HMS::DAL::PrescriptionRepository::getInstance();
        repo->setFilePath(testFilePath);

        // Clear any existing data
        repo->clear();
    }

    void TearDown() override
    {
        // Clean up
        if (repo)
        {
            repo->clear();
            repo->save(); // Explicit save (destructor no longer saves)
            HMS::DAL::PrescriptionRepository::resetInstance();
        }

        // Remove test file
        if (fs::exists(testFilePath))
        {
            fs::remove(testFilePath);
        }
    }

    // Helper: Create a test prescription
    HMS::Model::Prescription createTestPrescription(
        const std::string &id,
        const std::string &appointmentID,
        const std::string &patientUsername,
        const std::string &doctorID,
        const std::string &date = "2024-03-15")
    {
        HMS::Model::Prescription presc(id, appointmentID, patientUsername, doctorID, date);
        presc.setDiagnosis("Test diagnosis");
        presc.setNotes("Test notes");
        return presc;
    }

    // Helper: Create prescription with items
    HMS::Model::Prescription createPrescriptionWithItems(
        const std::string &id,
        const std::string &appointmentID,
        const std::string &patientUsername,
        const std::string &doctorID)
    {
        auto presc = createTestPrescription(id, appointmentID, patientUsername, doctorID);

        HMS::Model::PrescriptionItem item1;
        item1.medicineID = "MED001";
        item1.medicineName = "Paracetamol";
        item1.quantity = 20;
        item1.dosage = "2 tablets 3 times daily";
        item1.duration = "5 days";
        item1.instructions = "After meals";

        HMS::Model::PrescriptionItem item2;
        item2.medicineID = "MED002";
        item2.medicineName = "Amoxicillin";
        item2.quantity = 10;
        item2.dosage = "1 capsule twice daily";
        item2.duration = "7 days";
        item2.instructions = "With water";

        presc.addItem(item1);
        presc.addItem(item2);

        return presc;
    }

    // Helper: Populate test data
    void populateTestData()
    {
        // Add multiple prescriptions for different scenarios
        auto p1 = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
        auto p2 = createTestPrescription("PRE002", "APT002", "patient001", "D002", "2024-03-16");
        auto p3 = createTestPrescription("PRE003", "APT003", "patient002", "D001", "2024-03-17");
        auto p4 = createTestPrescription("PRE004", "APT004", "patient003", "D003", "2024-03-18");

        p1.setDispensed(true);
        p2.setDispensed(false);
        p3.setDispensed(false);
        p4.setDispensed(true);

        repo->add(p1);
        repo->add(p2);
        repo->add(p3);
        repo->add(p4);
    }
};

// ==================== Singleton Pattern Tests ====================

TEST_F(PrescriptionRepositoryTest, SingletonReturnsSameInstance)
{
    auto instance1 = HMS::DAL::PrescriptionRepository::getInstance();
    auto instance2 = HMS::DAL::PrescriptionRepository::getInstance();

    EXPECT_EQ(instance1, instance2);
}

TEST_F(PrescriptionRepositoryTest, ResetInstanceCreatesNewInstance)
{
    // Add data to current instance
    auto presc = createTestPrescription("PRE999", "APT999", "patient999", "D999");
    repo->add(presc);
    EXPECT_EQ(repo->count(), 1);

    // Reset instance - creates fresh instance with no data
    HMS::DAL::PrescriptionRepository::resetInstance();
    auto newInstance = HMS::DAL::PrescriptionRepository::getInstance();
    newInstance->setFilePath(testFilePath);
    newInstance->clear();  // Clear any loaded data

    // Fresh instance should have empty state after clear
    EXPECT_EQ(newInstance->count(), 0);

    // Update repo pointer for TearDown
    repo = newInstance;
}

// ==================== CRUD Operations Tests ====================

TEST_F(PrescriptionRepositoryTest, AddPrescriptionSuccess)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");

    EXPECT_TRUE(repo->add(presc));

    // Need to load after getInstance or use getAll()
    auto all = repo->getAll();
    EXPECT_EQ(all.size(), 1);
}

TEST_F(PrescriptionRepositoryTest, AddDuplicatePrescriptionFails)
{
    auto presc1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto presc2 = createTestPrescription("PRE001", "APT002", "patient002", "D002");

    EXPECT_TRUE(repo->add(presc1));
    EXPECT_FALSE(repo->add(presc2)); // Duplicate prescription ID should fail
    EXPECT_EQ(repo->getAll().size(), 1);
}

TEST_F(PrescriptionRepositoryTest, AddDuplicateAppointmentPrescriptionFails)
{
    auto presc1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto presc2 = createTestPrescription("PRE002", "APT001", "patient002", "D002"); // Same appointment

    EXPECT_TRUE(repo->add(presc1));
    EXPECT_FALSE(repo->add(presc2)); // Duplicate appointment should fail
    EXPECT_EQ(repo->getAll().size(), 1);
}

TEST_F(PrescriptionRepositoryTest, AddWithEmptyAppointmentAllowed)
{
    auto presc1 = createTestPrescription("PRE001", "", "patient001", "D001"); // Empty appointment
    auto presc2 = createTestPrescription("PRE002", "", "patient002", "D002"); // Empty appointment

    EXPECT_TRUE(repo->add(presc1));
    EXPECT_TRUE(repo->add(presc2)); // Both should succeed (no appointment ID)
    EXPECT_EQ(repo->getAll().size(), 2);
}

TEST_F(PrescriptionRepositoryTest, AddMultiplePrescriptionsSuccess)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto p2 = createTestPrescription("PRE002", "APT002", "patient002", "D002");
    auto p3 = createTestPrescription("PRE003", "APT003", "patient003", "D003");

    EXPECT_TRUE(repo->add(p1));
    EXPECT_TRUE(repo->add(p2));
    EXPECT_TRUE(repo->add(p3));
    EXPECT_EQ(repo->getAll().size(), 3);
}

TEST_F(PrescriptionRepositoryTest, GetByIdExisting)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto result = repo->getById("PRE001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPrescriptionID(), "PRE001");
    EXPECT_EQ(result->getAppointmentID(), "APT001");
    EXPECT_EQ(result->getPatientUsername(), "patient001");
    EXPECT_EQ(result->getDoctorID(), "D001");
}

TEST_F(PrescriptionRepositoryTest, GetByIdNonExisting)
{
    auto result = repo->getById("PRE999");

    EXPECT_FALSE(result.has_value());
}

TEST_F(PrescriptionRepositoryTest, GetByIdEmptyRepository)
{
    auto result = repo->getById("PRE001");

    EXPECT_FALSE(result.has_value());
}

TEST_F(PrescriptionRepositoryTest, GetAllEmptyRepository)
{
    auto results = repo->getAll();

    EXPECT_TRUE(results.empty());
    EXPECT_EQ(results.size(), 0);
}

TEST_F(PrescriptionRepositoryTest, GetAllMultiplePrescriptions)
{
    populateTestData();

    auto results = repo->getAll();

    EXPECT_EQ(results.size(), 4);
}

TEST_F(PrescriptionRepositoryTest, UpdateExistingPrescription)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    // Modify prescription
    presc.setDiagnosis("Updated diagnosis");
    presc.setNotes("Updated notes");
    presc.setDispensed(true);

    EXPECT_TRUE(repo->update(presc));

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getDiagnosis(), "Updated diagnosis");
    EXPECT_EQ(result->getNotes(), "Updated notes");
    EXPECT_TRUE(result->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, UpdateNonExistingPrescription)
{
    auto presc = createTestPrescription("PRE999", "APT999", "patient999", "D999");

    EXPECT_FALSE(repo->update(presc));
}

TEST_F(PrescriptionRepositoryTest, UpdatePrescriptionItems)
{
    auto presc = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    // Add another item
    HMS::Model::PrescriptionItem item3;
    item3.medicineID = "MED003";
    item3.medicineName = "Ibuprofen";
    item3.quantity = 15;
    item3.dosage = "1 tablet twice daily";
    item3.duration = "3 days";
    item3.instructions = "With food";

    presc.addItem(item3);

    EXPECT_TRUE(repo->update(presc));

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getItemCount(), 3);
}

TEST_F(PrescriptionRepositoryTest, RemoveExistingPrescription)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    EXPECT_EQ(repo->getAll().size(), 1);
    EXPECT_TRUE(repo->remove("PRE001"));
    EXPECT_EQ(repo->getAll().size(), 0);
}

TEST_F(PrescriptionRepositoryTest, RemoveNonExistingPrescription)
{
    EXPECT_FALSE(repo->remove("PRE999"));
}

TEST_F(PrescriptionRepositoryTest, RemoveFromEmptyRepository)
{
    EXPECT_FALSE(repo->remove("PRE001"));
}

// ==================== Persistence Tests ====================

TEST_F(PrescriptionRepositoryTest, SaveAndLoadPrescriptions)
{
    populateTestData();

    EXPECT_TRUE(repo->save());

    // Reset and load
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto results = repo->getAll();
    EXPECT_EQ(results.size(), 4);
}

TEST_F(PrescriptionRepositoryTest, SaveEmptyRepository)
{
    EXPECT_TRUE(repo->save());

    // File should exist but be empty (except header)
    EXPECT_TRUE(fs::exists(testFilePath));
}

TEST_F(PrescriptionRepositoryTest, LoadFromNonExistentFile)
{
    // Remove file if exists
    if (fs::exists(testFilePath))
    {
        fs::remove(testFilePath);
    }

    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto results = repo->getAll();
    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, PersistenceWithComplexItems)
{
    auto presc = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);
    repo->save();

    // Reload
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getItemCount(), 2);

    auto items = result->getItems();
    EXPECT_EQ(items[0].medicineID, "MED001");
    EXPECT_EQ(items[0].quantity, 20);
    EXPECT_EQ(items[1].medicineID, "MED002");
    EXPECT_EQ(items[1].quantity, 10);
}

TEST_F(PrescriptionRepositoryTest, ExplicitSaveRequired)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc); // add() calls save()

    // Data should be persisted
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    EXPECT_EQ(repo->getAll().size(), 1);
}

TEST_F(PrescriptionRepositoryTest, ResetInstanceSavesData)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    // resetInstance() saves before destroying
    HMS::DAL::PrescriptionRepository::resetInstance();

    // Verify data was saved
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);
    EXPECT_EQ(repo->getAll().size(), 1);
}

// ==================== Query Operations Tests ====================

TEST_F(PrescriptionRepositoryTest, CountAfterAdd)
{
    populateTestData();

    // count() auto-loads and returns correct count
    EXPECT_EQ(repo->count(), 4);
}

TEST_F(PrescriptionRepositoryTest, CountEmptyRepository)
{
    repo->clear();
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(PrescriptionRepositoryTest, CountAutoLoadsFromFile)
{
    // Add data and save
    populateTestData();
    repo->save();

    // New instance (data not loaded yet)
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    // count() auto-loads data from file (consistent with other repositories)
    EXPECT_EQ(repo->count(), 4);
}

TEST_F(PrescriptionRepositoryTest, ExistsAfterAdd)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    // exists() auto-loads and finds the prescription
    EXPECT_TRUE(repo->exists("PRE001"));
}

TEST_F(PrescriptionRepositoryTest, ExistsFalse)
{
    EXPECT_FALSE(repo->exists("PRE999"));
}

TEST_F(PrescriptionRepositoryTest, ExistsAutoLoadsFromFile)
{
    // Add data and save
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);
    repo->save();

    // New instance (data not loaded yet)
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    // exists() auto-loads data from file (consistent with other repositories)
    EXPECT_TRUE(repo->exists("PRE001"));
}

TEST_F(PrescriptionRepositoryTest, ClearRepository)
{
    populateTestData();

    EXPECT_EQ(repo->getAll().size(), 4);
    EXPECT_TRUE(repo->clear());
    EXPECT_EQ(repo->count(), 0); // count works after clear (m_isLoaded=true)
}

TEST_F(PrescriptionRepositoryTest, ClearEmptyRepository)
{
    EXPECT_TRUE(repo->clear());
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(PrescriptionRepositoryTest, ClearMaintainsLoadedState)
{
    populateTestData();
    repo->clear();

    // After clear, m_isLoaded=true, so count works without reload
    EXPECT_EQ(repo->count(), 0);
}

// ==================== Prescription-Specific Queries Tests ====================

TEST_F(PrescriptionRepositoryTest, GetByAppointmentFound)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto result = repo->getByAppointment("APT001");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPrescriptionID(), "PRE001");
}

TEST_F(PrescriptionRepositoryTest, GetByAppointmentNotFound)
{
    auto result = repo->getByAppointment("APT999");

    EXPECT_FALSE(result.has_value());
}

TEST_F(PrescriptionRepositoryTest, GetByAppointmentMultiplePrescriptions)
{
    populateTestData();

    auto result = repo->getByAppointment("APT002");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPrescriptionID(), "PRE002");
}

TEST_F(PrescriptionRepositoryTest, GetByPatientSinglePrescription)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto results = repo->getByPatient("patient001");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE001");
}

TEST_F(PrescriptionRepositoryTest, GetByPatientMultiplePrescriptions)
{
    populateTestData();

    auto results = repo->getByPatient("patient001");

    EXPECT_EQ(results.size(), 2); // PRE001 and PRE002
}

TEST_F(PrescriptionRepositoryTest, GetByPatientSortedByDate)
{
    populateTestData();

    auto results = repo->getByPatient("patient001");

    ASSERT_EQ(results.size(), 2);
    // Should be sorted by date descending (most recent first)
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE002"); // 2024-03-16
    EXPECT_EQ(results[1].getPrescriptionID(), "PRE001"); // 2024-03-15
}

TEST_F(PrescriptionRepositoryTest, GetByPatientNonExistent)
{
    populateTestData();

    auto results = repo->getByPatient("patient999");

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetByDoctorSinglePrescription)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto results = repo->getByDoctor("D001");

    EXPECT_EQ(results.size(), 1);
}

TEST_F(PrescriptionRepositoryTest, GetByDoctorMultiplePrescriptions)
{
    populateTestData();

    auto results = repo->getByDoctor("D001");

    EXPECT_EQ(results.size(), 2); // PRE001 and PRE003
}

TEST_F(PrescriptionRepositoryTest, GetByDoctorSortedByDate)
{
    populateTestData();

    auto results = repo->getByDoctor("D001");

    ASSERT_EQ(results.size(), 2);
    // Should be sorted by date descending
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE003"); // 2024-03-17
    EXPECT_EQ(results[1].getPrescriptionID(), "PRE001"); // 2024-03-15
}

TEST_F(PrescriptionRepositoryTest, GetByDoctorNonExistent)
{
    populateTestData();

    auto results = repo->getByDoctor("D999");

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetUndispensedPrescriptions)
{
    populateTestData();

    auto results = repo->getUndispensed();

    EXPECT_EQ(results.size(), 2); // PRE002 and PRE003
}

TEST_F(PrescriptionRepositoryTest, GetUndispensedSortedOldestFirst)
{
    populateTestData();

    auto results = repo->getUndispensed();

    ASSERT_EQ(results.size(), 2);
    // Should be sorted oldest first (priority)
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE002"); // 2024-03-16
    EXPECT_EQ(results[1].getPrescriptionID(), "PRE003"); // 2024-03-17
}

TEST_F(PrescriptionRepositoryTest, GetUndispensedEmpty)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    p1.setDispensed(true);
    repo->add(p1);

    auto results = repo->getUndispensed();

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetDispensedPrescriptions)
{
    populateTestData();

    auto results = repo->getDispensed();

    EXPECT_EQ(results.size(), 2); // PRE001 and PRE004
}

TEST_F(PrescriptionRepositoryTest, GetDispensedSortedMostRecentFirst)
{
    populateTestData();

    auto results = repo->getDispensed();

    ASSERT_EQ(results.size(), 2);
    // Should be sorted most recent first
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE004"); // 2024-03-18
    EXPECT_EQ(results[1].getPrescriptionID(), "PRE001"); // 2024-03-15
}

TEST_F(PrescriptionRepositoryTest, GetDispensedEmpty)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    p1.setDispensed(false);
    repo->add(p1);

    auto results = repo->getDispensed();

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetByDateFound)
{
    populateTestData();

    auto results = repo->getByDate("2024-03-16");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE002");
}

TEST_F(PrescriptionRepositoryTest, GetByDateNotFound)
{
    populateTestData();

    auto results = repo->getByDate("2024-12-31");

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetByDateMultiplePrescriptions)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001", "2024-03-15");
    auto p2 = createTestPrescription("PRE002", "APT002", "patient002", "D002", "2024-03-15");
    repo->add(p1);
    repo->add(p2);

    auto results = repo->getByDate("2024-03-15");

    EXPECT_EQ(results.size(), 2);
}

TEST_F(PrescriptionRepositoryTest, GetByDateRangeAllIncluded)
{
    populateTestData();

    auto results = repo->getByDateRange("2024-03-15", "2024-03-18");

    EXPECT_EQ(results.size(), 4);
}

TEST_F(PrescriptionRepositoryTest, GetByDateRangePartial)
{
    populateTestData();

    auto results = repo->getByDateRange("2024-03-16", "2024-03-17");

    EXPECT_EQ(results.size(), 2); // PRE002 and PRE003
}

TEST_F(PrescriptionRepositoryTest, GetByDateRangeSortedDescending)
{
    populateTestData();

    auto results = repo->getByDateRange("2024-03-16", "2024-03-17");

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE003"); // 2024-03-17
    EXPECT_EQ(results[1].getPrescriptionID(), "PRE002"); // 2024-03-16
}

TEST_F(PrescriptionRepositoryTest, GetByDateRangeNoneFound)
{
    populateTestData();

    auto results = repo->getByDateRange("2024-12-01", "2024-12-31");

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetByDateRangeSingleDay)
{
    populateTestData();

    auto results = repo->getByDateRange("2024-03-16", "2024-03-16");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE002");
}

TEST_F(PrescriptionRepositoryTest, GetByMedicineFound)
{
    auto presc = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto results = repo->getByMedicine("MED001");

    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].getPrescriptionID(), "PRE001");
}

TEST_F(PrescriptionRepositoryTest, GetByMedicineNotFound)
{
    auto presc = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    auto results = repo->getByMedicine("MED999");

    EXPECT_TRUE(results.empty());
}

TEST_F(PrescriptionRepositoryTest, GetByMedicineMultiplePrescriptions)
{
    auto p1 = createPrescriptionWithItems("PRE001", "APT001", "patient001", "D001");
    auto p2 = createPrescriptionWithItems("PRE002", "APT002", "patient002", "D002");
    repo->add(p1);
    repo->add(p2);

    auto results = repo->getByMedicine("MED001");

    EXPECT_EQ(results.size(), 2); // Both have MED001
}

TEST_F(PrescriptionRepositoryTest, GetByMedicineNoPrescriptionsWithItems)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(p1);

    auto results = repo->getByMedicine("MED001");

    EXPECT_TRUE(results.empty());
}

// ==================== ID Generation Tests ====================

TEST_F(PrescriptionRepositoryTest, GetNextIdEmptyRepository)
{
    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE001");
}

TEST_F(PrescriptionRepositoryTest, GetNextIdSequential)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(p1);

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE002");
}

TEST_F(PrescriptionRepositoryTest, GetNextIdGaps)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto p5 = createTestPrescription("PRE005", "APT005", "patient005", "D005");
    repo->add(p1);
    repo->add(p5);

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE006"); // Should be max + 1
}

TEST_F(PrescriptionRepositoryTest, GetNextIdThreeDigitFormat)
{
    auto p99 = createTestPrescription("PRE099", "APT099", "patient099", "D099");
    repo->add(p99);

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE100");
}

TEST_F(PrescriptionRepositoryTest, GetNextIdWithInvalidIDs)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto pInvalid = createTestPrescription("INVALID", "APT002", "patient002", "D002");
    repo->add(p1);
    repo->add(pInvalid);

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE002"); // Should skip invalid IDs
}

TEST_F(PrescriptionRepositoryTest, GetNextIdAfterRemoval)
{
    auto p1 = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    auto p2 = createTestPrescription("PRE002", "APT002", "patient002", "D002");
    auto p3 = createTestPrescription("PRE003", "APT003", "patient003", "D003");

    repo->add(p1);
    repo->add(p2);
    repo->add(p3);
    repo->remove("PRE002");

    std::string nextId = repo->getNextId();

    EXPECT_EQ(nextId, "PRE004"); // Should be max + 1, not fill gap
}

// ==================== Dispensing Operations Tests ====================

TEST_F(PrescriptionRepositoryTest, MarkAsDispensedSuccess)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    presc.setDispensed(false);
    repo->add(presc);

    EXPECT_TRUE(repo->markAsDispensed("PRE001"));

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, MarkAsDispensedNonExistent)
{
    EXPECT_FALSE(repo->markAsDispensed("PRE999"));
}

TEST_F(PrescriptionRepositoryTest, MarkAsDispensedAlreadyDispensed)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    presc.setDispensed(true);
    repo->add(presc);

    EXPECT_TRUE(repo->markAsDispensed("PRE001")); // Should still succeed

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, MarkAsUndispensedSuccess)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    presc.setDispensed(true);
    repo->add(presc);

    EXPECT_TRUE(repo->markAsUndispensed("PRE001"));

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, MarkAsUndispensedNonExistent)
{
    EXPECT_FALSE(repo->markAsUndispensed("PRE999"));
}

TEST_F(PrescriptionRepositoryTest, MarkAsUndispensedAlreadyUndispensed)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    presc.setDispensed(false);
    repo->add(presc);

    EXPECT_TRUE(repo->markAsUndispensed("PRE001")); // Should still succeed

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, ToggleDispensedStatus)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    presc.setDispensed(false);
    repo->add(presc);

    // Dispense
    repo->markAsDispensed("PRE001");
    auto result1 = repo->getById("PRE001");
    EXPECT_TRUE(result1->isDispensed());

    // Undispense
    repo->markAsUndispensed("PRE001");
    auto result2 = repo->getById("PRE001");
    EXPECT_FALSE(result2->isDispensed());

    // Dispense again
    repo->markAsDispensed("PRE001");
    auto result3 = repo->getById("PRE001");
    EXPECT_TRUE(result3->isDispensed());
}

// ==================== File Path Management Tests ====================

TEST_F(PrescriptionRepositoryTest, SetFilePathChangesStorage)
{
    std::string altPath = "test/fixtures/Prescription_alt.txt";

    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    repo->add(presc);

    repo->setFilePath(altPath);
    EXPECT_EQ(repo->getFilePath(), altPath);

    // Original file should still have data
    EXPECT_TRUE(fs::exists(testFilePath));

    // Cleanup
    if (fs::exists(altPath))
    {
        fs::remove(altPath);
    }
}

TEST_F(PrescriptionRepositoryTest, GetFilePathDefault)
{
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();

    EXPECT_EQ(repo->getFilePath(), HMS::Constants::PRESCRIPTION_FILE);
}

TEST_F(PrescriptionRepositoryTest, GetFilePathDoesNotLoad)
{
    // Create new instance without loading
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    // getFilePath should not trigger load
    std::string path = repo->getFilePath();
    EXPECT_EQ(path, testFilePath);

    // count() would be 0 since no data loaded
    EXPECT_EQ(repo->count(), 0);
}

// ==================== Edge Cases & Stress Tests ====================

TEST_F(PrescriptionRepositoryTest, LargePrescriptionItem)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");

    HMS::Model::PrescriptionItem item;
    item.medicineID = "MED001";
    item.medicineName = std::string(1000, 'A'); // Very long name
    item.quantity = 999999;
    item.dosage = std::string(500, 'B');
    item.duration = std::string(200, 'C');
    item.instructions = std::string(1000, 'D');

    presc.addItem(item);
    repo->add(presc);
    repo->save();

    // Reload and verify
    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getItemCount(), 1);
}

TEST_F(PrescriptionRepositoryTest, ManyItems)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");

    // Add 100 items
    for (int i = 1; i <= 100; ++i)
    {
        HMS::Model::PrescriptionItem item;
        item.medicineID = std::format("MED{:03d}", i);
        item.medicineName = std::format("Medicine {}", i);
        item.quantity = i;
        item.dosage = "Test dosage";
        item.duration = "Test duration";
        item.instructions = "Test instructions";
        presc.addItem(item);
    }

    repo->add(presc);
    EXPECT_EQ(presc.getItemCount(), 100);
}

TEST_F(PrescriptionRepositoryTest, SpecialCharactersInFields)
{
    auto presc = createTestPrescription("PRE001", "APT001", "patient001", "D001");
    // Note: Pipe '|' is the field delimiter and cannot be used in data fields
    presc.setDiagnosis("Special: ;:,<>?/");
    presc.setNotes("Unicode: café naïve résumé");

    repo->add(presc);
    repo->save();

    HMS::DAL::PrescriptionRepository::resetInstance();
    repo = HMS::DAL::PrescriptionRepository::getInstance();
    repo->setFilePath(testFilePath);

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
}

TEST_F(PrescriptionRepositoryTest, ConcurrentOperations)
{
    // Add multiple prescriptions
    for (int i = 1; i <= 10; ++i)
    {
        auto presc = createTestPrescription(
            std::format("PRE{:03d}", i),
            std::format("APT{:03d}", i),
            std::format("patient{:03d}", i),
            std::format("D{:03d}", i % 5 + 1));
        repo->add(presc);
    }

    EXPECT_EQ(repo->getAll().size(), 10);

    // Remove some
    repo->remove("PRE003");
    repo->remove("PRE007");

    EXPECT_EQ(repo->getAll().size(), 8);

    // Update some
    auto presc5 = repo->getById("PRE005");
    ASSERT_TRUE(presc5.has_value());
    presc5->setDispensed(true);
    repo->update(*presc5);

    // Verify
    auto updated = repo->getById("PRE005");
    EXPECT_TRUE(updated->isDispensed());
}

TEST_F(PrescriptionRepositoryTest, EmptyStringFields)
{
    HMS::Model::Prescription presc("PRE001", "", "", "", "");

    repo->add(presc);

    auto result = repo->getById("PRE001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getAppointmentID(), "");
    EXPECT_EQ(result->getPatientUsername(), "");
}

TEST_F(PrescriptionRepositoryTest, GetByPatientWithManyPrescriptions)
{
    // Add 50 prescriptions for same patient
    for (int i = 1; i <= 50; ++i)
    {
        auto presc = createTestPrescription(
            std::format("PRE{:03d}", i),
            std::format("APT{:03d}", i),
            "patient001",
            "D001",
            std::format("2024-03-{:02d}", (i % 30) + 1));
        repo->add(presc);
    }

    auto results = repo->getByPatient("patient001");

    EXPECT_EQ(results.size(), 50);
    // Verify sorted by date (descending)
    for (size_t i = 1; i < results.size(); ++i)
    {
        EXPECT_GE(HMS::Utils::compareDates(
                      results[i - 1].getPrescriptionDate(),
                      results[i].getPrescriptionDate()),
                  0);
    }
}