#include <filesystem>
#include <gtest/gtest.h>

#include "bll/PrescriptionService.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/AppointmentRepository.h"
#include "dal/MedicineRepository.h"
#include "dal/PrescriptionRepository.h"

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Test Fixture ====================

class PrescriptionServiceTest : public ::testing::Test
{
protected:
    PrescriptionService *prescriptionService;

    // Test file paths to avoid touching production data
    std::string testPrescriptionFile = "test_prescription_service.txt";
    std::string testAppointmentFile = "test_prescription_appointments.txt";
    std::string testMedicineFile = "test_prescription_medicines.txt";

    void SetUp() override
    {
        // Reset singletons to get fresh instances
        PrescriptionService::resetInstance();
        PrescriptionRepository::resetInstance();
        AppointmentRepository::resetInstance();
        MedicineRepository::resetInstance();

        // Redirect repositories to test files
        PrescriptionRepository::getInstance()->setFilePath(testPrescriptionFile);
        PrescriptionRepository::getInstance()->clear();

        AppointmentRepository::getInstance()->setFilePath(testAppointmentFile);
        AppointmentRepository::getInstance()->clear();

        MedicineRepository::getInstance()->setFilePath(testMedicineFile);
        MedicineRepository::getInstance()->clear();

        // Get PrescriptionService instance
        prescriptionService = PrescriptionService::getInstance();
    }

    void TearDown() override
    {
        // Reset singletons
        PrescriptionService::resetInstance();
        PrescriptionRepository::resetInstance();
        AppointmentRepository::resetInstance();
        MedicineRepository::resetInstance();

        // Clean up test files
        cleanupTestFile(testPrescriptionFile);
        cleanupTestFile(testAppointmentFile);
        cleanupTestFile(testMedicineFile);
    }

    void cleanupTestFile(const std::string &filePath)
    {
        if (std::filesystem::exists(filePath))
        {
            std::filesystem::remove(filePath);
        }
        std::string backupFile = filePath + ".bak";
        if (std::filesystem::exists(backupFile))
        {
            std::filesystem::remove(backupFile);
        }
    }

    // Helper to create test prescription
    Prescription
    createTestPrescription(const std::string &prescriptionID = "PRE001",
                           const std::string &appointmentID = "APT001",
                           const std::string &patientUsername = "patient001",
                           const std::string &doctorID = "D001")
    {
        Prescription presc(prescriptionID, appointmentID, patientUsername, doctorID,
                           Utils::getCurrentDate());
        presc.setDiagnosis("Test Diagnosis");
        presc.setNotes("Test Notes");
        return presc;
    }

    // Helper to create test appointment
    Appointment
    createTestAppointment(const std::string &appointmentID = "APT001",
                          const std::string &patientUsername = "patient001",
                          const std::string &doctorID = "D001")
    {
        return Appointment(appointmentID, patientUsername, doctorID,
                           Utils::getCurrentDate(), "10:00", "General checkup",
                           500000.0, false, AppointmentStatus::SCHEDULED, "");
    }

    // Helper to create test medicine
    Medicine createTestMedicine(const std::string &medicineID = "MED001",
                                const std::string &name = "Paracetamol",
                                int quantity = 100)
    {
        Medicine med(medicineID, name, "Pain Relief", 50000.0, quantity);
        med.setReorderLevel(10);
        return med;
    }

    // Helper to create prescription item
    PrescriptionItem
    createTestItem(const std::string &medicineID = "MED001",
                   const std::string &medicineName = "Paracetamol",
                   int quantity = 10)
    {
        PrescriptionItem item;
        item.medicineID = medicineID;
        item.medicineName = medicineName;
        item.quantity = quantity;
        item.dosage = "2 tablets, 3 times daily";
        item.duration = "5 days";
        item.instructions = "After meals";
        return item;
    }
};

// ==================== Singleton Tests ====================

TEST_F(PrescriptionServiceTest, GetInstance_ReturnsSameInstance)
{
    PrescriptionService *instance1 = PrescriptionService::getInstance();
    PrescriptionService *instance2 = PrescriptionService::getInstance();

    EXPECT_EQ(instance1, instance2);
}

TEST_F(PrescriptionServiceTest, ResetInstance_CreatesFreshInstance)
{
    PrescriptionService *instance1 = PrescriptionService::getInstance();
    EXPECT_NE(instance1, nullptr);

    PrescriptionService::resetInstance();

    PrescriptionService *instance2 = PrescriptionService::getInstance();
    EXPECT_NE(instance2, nullptr);
}

// ==================== CRUD - Create Tests ====================

TEST_F(PrescriptionServiceTest,
       CreatePrescription_ValidPrescription_ReturnsTrue)
{
    Prescription presc = createTestPrescription("PRE001");

    bool result = prescriptionService->createPrescription(presc);

    EXPECT_TRUE(result);
    EXPECT_EQ(prescriptionService->getPrescriptionCount(), 1u);
}

TEST_F(PrescriptionServiceTest, CreatePrescription_DuplicateID_ReturnsFalse)
{
    Prescription presc1 = createTestPrescription("PRE001");
    Prescription presc2 = createTestPrescription("PRE001", "APT002");

    prescriptionService->createPrescription(presc1);
    bool result = prescriptionService->createPrescription(presc2);

    EXPECT_FALSE(result);
    EXPECT_EQ(prescriptionService->getPrescriptionCount(), 1u);
}

TEST_F(PrescriptionServiceTest,
       CreatePrescription_DuplicateAppointment_ReturnsFalse)
{
    Prescription presc1 = createTestPrescription("PRE001", "APT001");
    Prescription presc2 = createTestPrescription("PRE002", "APT001");

    prescriptionService->createPrescription(presc1);
    bool result = prescriptionService->createPrescription(presc2);

    EXPECT_FALSE(result);
}

TEST_F(PrescriptionServiceTest,
       CreatePrescription_FromAppointment_ReturnsCreatedPrescription)
{
    AppointmentRepository::getInstance()->add(createTestAppointment("APT001"));

    auto result =
        prescriptionService->createPrescription("APT001", "Fever", "Rest needed");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->getAppointmentID(), "APT001");
    EXPECT_EQ(result->getDiagnosis(), "Fever");
    EXPECT_EQ(result->getPatientUsername(), "patient001");
    EXPECT_EQ(result->getDoctorID(), "D001");
}

TEST_F(PrescriptionServiceTest,
       CreatePrescription_NonExistentAppointment_ReturnsNullopt)
{
    auto result = prescriptionService->createPrescription("NONEXISTENT",
                                                          "Diagnosis", "Notes");

    EXPECT_FALSE(result.has_value());
}

// ==================== CRUD - Read Tests ====================

TEST_F(PrescriptionServiceTest,
       GetPrescriptionByID_Exists_ReturnsPrescription)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    auto result = prescriptionService->getPrescriptionByID("PRE001");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->getPrescriptionID(), "PRE001");
}

TEST_F(PrescriptionServiceTest, GetPrescriptionByID_NotExists_ReturnsNullopt)
{
    auto result = prescriptionService->getPrescriptionByID("NONEXISTENT");

    EXPECT_FALSE(result.has_value());
}

TEST_F(PrescriptionServiceTest,
       GetPrescriptionByAppointment_Exists_ReturnsPrescription)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));

    auto result = prescriptionService->getPrescriptionByAppointment("APT001");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->getAppointmentID(), "APT001");
}

TEST_F(PrescriptionServiceTest, GetAllPrescriptions_ReturnsAllPrescriptions)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));
    prescriptionService->createPrescription(
        createTestPrescription("PRE002", "APT002"));

    auto prescriptions = prescriptionService->getAllPrescriptions();

    EXPECT_EQ(prescriptions.size(), 2u);
}

TEST_F(PrescriptionServiceTest,
       GetPatientPrescriptions_ReturnsPatientPrescriptions)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001", "patient001"));
    prescriptionService->createPrescription(
        createTestPrescription("PRE002", "APT002", "patient002"));
    prescriptionService->createPrescription(
        createTestPrescription("PRE003", "APT003", "patient001"));

    auto prescriptions =
        prescriptionService->getPatientPrescriptions("patient001");

    EXPECT_EQ(prescriptions.size(), 2u);
}

TEST_F(PrescriptionServiceTest,
       GetDoctorPrescriptions_ReturnsDoctorPrescriptions)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001", "p1", "D001"));
    prescriptionService->createPrescription(
        createTestPrescription("PRE002", "APT002", "p2", "D002"));

    auto prescriptions = prescriptionService->getDoctorPrescriptions("D001");

    EXPECT_EQ(prescriptions.size(), 1u);
}

// ==================== CRUD - Update Tests ====================

TEST_F(PrescriptionServiceTest, UpdatePrescription_Exists_ReturnsTrue)
{
    Prescription presc = createTestPrescription("PRE001");
    prescriptionService->createPrescription(presc);

    presc.setDiagnosis("Updated Diagnosis");
    presc.setNotes("Updated Notes");
    bool result = prescriptionService->updatePrescription(presc);

    EXPECT_TRUE(result);

    auto updated = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_EQ(updated->getDiagnosis(), "Updated Diagnosis");
}

TEST_F(PrescriptionServiceTest,
       UpdatePrescription_DispensedPrescription_ReturnsFalse)
{
    Prescription presc = createTestPrescription("PRE001");
    presc.setDispensed(true);
    PrescriptionRepository::getInstance()->add(
        presc); // Add directly to bypass validation

    presc.setDiagnosis("New Diagnosis");
    bool result = prescriptionService->updatePrescription(presc);

    EXPECT_FALSE(result);
}

// ==================== CRUD - Delete Tests ====================

TEST_F(PrescriptionServiceTest, DeletePrescription_Exists_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    bool result = prescriptionService->deletePrescription("PRE001");

    EXPECT_TRUE(result);
    EXPECT_EQ(prescriptionService->getPrescriptionCount(), 0u);
}

TEST_F(PrescriptionServiceTest, DeletePrescription_Dispensed_ReturnsFalse)
{
    Prescription presc = createTestPrescription("PRE001");
    presc.setDispensed(true);
    PrescriptionRepository::getInstance()->add(presc);

    bool result = prescriptionService->deletePrescription("PRE001");

    EXPECT_FALSE(result);
}

// ==================== Item Management Tests ====================

TEST_F(PrescriptionServiceTest, AddPrescriptionItem_ValidItem_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(createTestMedicine("MED001"));

    bool result = prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001"));

    EXPECT_TRUE(result);

    auto presc = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_EQ(presc->getItemCount(), 1u);
}

TEST_F(PrescriptionServiceTest, AddPrescriptionItem_WithParams_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol"));

    bool result = prescriptionService->addPrescriptionItem(
        "PRE001", "MED001", 10, "2 tablets daily", "7 days", "After meals");

    EXPECT_TRUE(result);

    auto presc = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_EQ(presc->getItemCount(), 1u);
}

TEST_F(PrescriptionServiceTest,
       AddPrescriptionItem_NonExistentMedicine_ReturnsFalse)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    bool result = prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("NONEXISTENT"));

    EXPECT_FALSE(result);
}

TEST_F(PrescriptionServiceTest,
       AddPrescriptionItem_DispensedPrescription_ReturnsFalse)
{
    Prescription presc = createTestPrescription("PRE001");
    presc.setDispensed(true);
    PrescriptionRepository::getInstance()->add(presc);
    MedicineRepository::getInstance()->add(createTestMedicine("MED001"));

    bool result = prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001"));

    EXPECT_FALSE(result);
}

TEST_F(PrescriptionServiceTest, RemovePrescriptionItem_ValidItem_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(createTestMedicine("MED001"));
    prescriptionService->addPrescriptionItem("PRE001", createTestItem("MED001"));

    bool result = prescriptionService->removePrescriptionItem("PRE001", "MED001");

    EXPECT_TRUE(result);

    auto presc = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_EQ(presc->getItemCount(), 0u);
}

TEST_F(PrescriptionServiceTest, ClearPrescriptionItems_ClearsAllItems)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(createTestMedicine("MED001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED002", "Ibuprofen"));
    prescriptionService->addPrescriptionItem("PRE001", createTestItem("MED001"));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED002", "Ibuprofen"));

    bool result = prescriptionService->clearPrescriptionItems("PRE001");

    EXPECT_TRUE(result);

    auto presc = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_EQ(presc->getItemCount(), 0u);
}

// ==================== Dispensing Tests ====================

TEST_F(PrescriptionServiceTest,
       DispensePrescription_SufficientStock_SucceedsAndDeductsStock)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol", 100));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));

    auto result = prescriptionService->dispensePrescription("PRE001");

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.failedItems.empty());
    EXPECT_GT(result.totalCost, 0.0);

    // Check stock was deducted
    auto medicine = MedicineRepository::getInstance()->getById("MED001");
    EXPECT_EQ(medicine->getQuantityInStock(), 90);

    // Check prescription is marked dispensed
    auto presc = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_TRUE(presc->isDispensed());
}

TEST_F(PrescriptionServiceTest, DispensePrescription_InsufficientStock_Fails)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol", 5)); // Only 5 in stock
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10)); // Need 10

    auto result = prescriptionService->dispensePrescription("PRE001");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.failedItems.size(), 1u);
    EXPECT_EQ(result.failedItems[0], "MED001");

    // Stock should remain unchanged
    auto medicine = MedicineRepository::getInstance()->getById("MED001");
    EXPECT_EQ(medicine->getQuantityInStock(), 5);
}

TEST_F(PrescriptionServiceTest, DispensePrescription_AlreadyDispensed_Fails)
{
    Prescription presc = createTestPrescription("PRE001");
    presc.setDispensed(true);
    PrescriptionRepository::getInstance()->add(presc);

    auto result = prescriptionService->dispensePrescription("PRE001");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Prescription already dispensed");
}

TEST_F(PrescriptionServiceTest, DispensePrescription_NoItems_Fails)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    auto result = prescriptionService->dispensePrescription("PRE001");

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.message, "Prescription has no items");
}

TEST_F(PrescriptionServiceTest, CanDispense_SufficientStock_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol", 100));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));

    EXPECT_TRUE(prescriptionService->canDispense("PRE001"));
}

TEST_F(PrescriptionServiceTest, CanDispense_InsufficientStock_ReturnsFalse)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol", 5));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));

    EXPECT_FALSE(prescriptionService->canDispense("PRE001"));
}

TEST_F(PrescriptionServiceTest, GetInsufficientStockItems_ReturnsCorrectItems)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol", 100));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED002", "Ibuprofen", 5));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED002", "Ibuprofen", 20));

    auto insufficientItems =
        prescriptionService->getInsufficientStockItems("PRE001");

    EXPECT_EQ(insufficientItems.size(), 1u);
    EXPECT_EQ(insufficientItems[0], "MED002");
}

TEST_F(PrescriptionServiceTest,
       GetUndispensedPrescriptions_ReturnsUndispensed)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));

    Prescription dispensed = createTestPrescription("PRE002", "APT002");
    dispensed.setDispensed(true);
    PrescriptionRepository::getInstance()->add(dispensed);

    auto undispensed = prescriptionService->getUndispensedPrescriptions();

    EXPECT_EQ(undispensed.size(), 1u);
    EXPECT_EQ(undispensed[0].getPrescriptionID(), "PRE001");
}

TEST_F(PrescriptionServiceTest, GetDispensedPrescriptions_ReturnsDispensed)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));

    Prescription dispensed = createTestPrescription("PRE002", "APT002");
    dispensed.setDispensed(true);
    PrescriptionRepository::getInstance()->add(dispensed);

    auto dispensedList = prescriptionService->getDispensedPrescriptions();

    EXPECT_EQ(dispensedList.size(), 1u);
    EXPECT_EQ(dispensedList[0].getPrescriptionID(), "PRE002");
}

// ==================== Cost Calculation Tests ====================

TEST_F(PrescriptionServiceTest, CalculatePrescriptionCost_ReturnsCorrectCost)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    // Medicine with price 50000
    Medicine med("MED001", "Paracetamol", "Pain Relief", 50000.0, 100);
    MedicineRepository::getInstance()->add(med);

    // Add item with quantity 10
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));

    double cost = prescriptionService->calculatePrescriptionCost("PRE001");

    // 50000 * 10 = 500000
    EXPECT_DOUBLE_EQ(cost, 500000.0);
}

TEST_F(PrescriptionServiceTest,
       CalculatePrescriptionCost_MultipleItems_ReturnsTotal)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    Medicine med1("MED001", "Paracetamol", "Pain Relief", 50000.0, 100);
    Medicine med2("MED002", "Ibuprofen", "Pain Relief", 75000.0, 100);
    MedicineRepository::getInstance()->add(med1);
    MedicineRepository::getInstance()->add(med2);

    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED001", "Paracetamol", 10));
    prescriptionService->addPrescriptionItem(
        "PRE001", createTestItem("MED002", "Ibuprofen", 5));

    double cost = prescriptionService->calculatePrescriptionCost("PRE001");

    // (50000 * 10) + (75000 * 5) = 500000 + 375000 = 875000
    EXPECT_DOUBLE_EQ(cost, 875000.0);
}

TEST_F(PrescriptionServiceTest,
       CalculatePrescriptionCost_NonExistent_ReturnsNegativeOne)
{
    double cost = prescriptionService->calculatePrescriptionCost("NONEXISTENT");

    EXPECT_DOUBLE_EQ(cost, -1.0);
}

// ==================== Statistics Tests ====================

TEST_F(PrescriptionServiceTest, GetPrescriptionStatistics_ReturnsCorrectStats)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));
    prescriptionService->createPrescription(
        createTestPrescription("PRE002", "APT002"));

    Prescription dispensed = createTestPrescription("PRE003", "APT003");
    dispensed.setDispensed(true);
    PrescriptionRepository::getInstance()->add(dispensed);

    auto stats = prescriptionService->getPrescriptionStatistics();

    EXPECT_EQ(stats["total"], 3);
    EXPECT_EQ(stats["dispensed"], 1);
    EXPECT_EQ(stats["undispensed"], 2);
}

TEST_F(PrescriptionServiceTest,
       GetMostPrescribedMedicines_ReturnsCorrectOrder)
{
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED001", "Paracetamol"));
    MedicineRepository::getInstance()->add(
        createTestMedicine("MED002", "Ibuprofen"));

    // Create prescriptions with items
    Prescription p1 = createTestPrescription("PRE001", "APT001");
    p1.addItem(createTestItem("MED001", "Paracetamol", 10));
    p1.addItem(createTestItem("MED002", "Ibuprofen", 5));
    PrescriptionRepository::getInstance()->add(p1);

    Prescription p2 = createTestPrescription("PRE002", "APT002");
    p2.addItem(createTestItem("MED001", "Paracetamol", 20));
    PrescriptionRepository::getInstance()->add(p2);

    auto mostPrescribed = prescriptionService->getMostPrescribedMedicines(10);

    EXPECT_GE(mostPrescribed.size(), 2u);
    // MED001 should have total 30, MED002 should have 5
    EXPECT_EQ(mostPrescribed[0].first, "MED001");
    EXPECT_EQ(mostPrescribed[0].second, 30);
    EXPECT_EQ(mostPrescribed[1].first, "MED002");
    EXPECT_EQ(mostPrescribed[1].second, 5);
}

// ==================== Validation Tests ====================

TEST_F(PrescriptionServiceTest,
       ValidatePrescription_ValidPrescription_ReturnsTrue)
{
    Prescription presc = createTestPrescription("PRE001");

    EXPECT_TRUE(prescriptionService->validatePrescription(presc));
}

TEST_F(PrescriptionServiceTest, ValidatePrescription_EmptyID_ReturnsFalse)
{
    Prescription presc("", "APT001", "patient001", "D001",
                       Utils::getCurrentDate());

    EXPECT_FALSE(prescriptionService->validatePrescription(presc));
}

TEST_F(PrescriptionServiceTest,
       ValidatePrescription_EmptyPatient_ReturnsFalse)
{
    Prescription presc("PRE001", "APT001", "", "D001", Utils::getCurrentDate());

    EXPECT_FALSE(prescriptionService->validatePrescription(presc));
}

TEST_F(PrescriptionServiceTest, ValidatePrescription_EmptyDoctor_ReturnsFalse)
{
    Prescription presc("PRE001", "APT001", "patient001", "",
                       Utils::getCurrentDate());

    EXPECT_FALSE(prescriptionService->validatePrescription(presc));
}

TEST_F(PrescriptionServiceTest,
       ValidatePrescription_InvalidItemQuantity_ReturnsFalse)
{
    Prescription presc = createTestPrescription("PRE001");
    PrescriptionItem invalidItem = createTestItem("MED001", "Paracetamol", 0);
    presc.addItem(invalidItem);

    EXPECT_FALSE(prescriptionService->validatePrescription(presc));
}

TEST_F(PrescriptionServiceTest, PrescriptionExists_Exists_ReturnsTrue)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    EXPECT_TRUE(prescriptionService->prescriptionExists("PRE001"));
}

TEST_F(PrescriptionServiceTest, PrescriptionExists_NotExists_ReturnsFalse)
{
    EXPECT_FALSE(prescriptionService->prescriptionExists("NONEXISTENT"));
}

TEST_F(PrescriptionServiceTest,
       AppointmentHasPrescription_HasPrescription_ReturnsTrue)
{
    prescriptionService->createPrescription(
        createTestPrescription("PRE001", "APT001"));

    EXPECT_TRUE(prescriptionService->appointmentHasPrescription("APT001"));
}

TEST_F(PrescriptionServiceTest,
       AppointmentHasPrescription_NoPrescription_ReturnsFalse)
{
    EXPECT_FALSE(prescriptionService->appointmentHasPrescription("APT001"));
}

// ==================== Data Persistence Tests ====================

TEST_F(PrescriptionServiceTest, SaveData_DoesNotCrash)
{
    prescriptionService->createPrescription(createTestPrescription("PRE001"));

    bool result = prescriptionService->saveData();

    EXPECT_TRUE(result);
}

TEST_F(PrescriptionServiceTest, LoadData_DoesNotCrash)
{
    bool result = prescriptionService->loadData();

    EXPECT_TRUE(result);
}

TEST_F(PrescriptionServiceTest, SaveAndLoad_DataPersists)
{
    Prescription presc = createTestPrescription("PRE001");
    presc.setDiagnosis("Persistent Diagnosis");
    prescriptionService->createPrescription(presc);
    prescriptionService->saveData();

    // Reset and reload
    PrescriptionService::resetInstance();
    PrescriptionRepository::resetInstance();
    PrescriptionRepository::getInstance()->setFilePath(testPrescriptionFile);

    prescriptionService = PrescriptionService::getInstance();
    prescriptionService->loadData();

    auto loaded = prescriptionService->getPrescriptionByID("PRE001");
    EXPECT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->getDiagnosis(), "Persistent Diagnosis");
}

/*
Build and run tests:
cd build && ./HospitalTests --gtest_filter="PrescriptionServiceTest.*"
*/
