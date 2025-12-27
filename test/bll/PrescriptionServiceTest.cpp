#include <gtest/gtest.h>
#include <filesystem>

#include "bll/PrescriptionService.h"
#include "advance/Prescription.h"
#include "model/Appointment.h"
#include "advance/Medicine.h"
#include "common/Types.h"

// ==================== USING ====================
using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::Model;

namespace
{
    const std::string TEST_DATA_DIR = "test_data/";
    const std::string TEST_PRESCRIPTION_FILE = "test_data/prescriptions_test.txt";
    const std::string TEST_APPOINTMENT_FILE = "test_data/appointments_test.txt";
    const std::string TEST_MEDICINE_FILE = "test_data/medicines_test.txt";
}

// ==================== TEST FIXTURE ====================
class PrescriptionServiceTest : public ::testing::Test
{
protected:
    PrescriptionService *service;

    void SetUp() override
    {
        std::filesystem::create_directories(TEST_DATA_DIR);

        PrescriptionService::resetInstance();
        service = PrescriptionService::getInstance();

        auto presRepo = DAL::PrescriptionRepository::getInstance();
        auto apptRepo = DAL::AppointmentRepository::getInstance();
        auto medRepo = DAL::MedicineRepository::getInstance();

        presRepo->setFilePath(TEST_PRESCRIPTION_FILE);
        apptRepo->setFilePath(TEST_APPOINTMENT_FILE);
        medRepo->setFilePath(TEST_MEDICINE_FILE);

        presRepo->clear();
        apptRepo->clear();
        medRepo->clear();
    }

    void TearDown() override
    {
        DAL::PrescriptionRepository::getInstance()->clear();
        DAL::AppointmentRepository::getInstance()->clear();
        DAL::MedicineRepository::getInstance()->clear();

        PrescriptionService::resetInstance();
    }

    Appointment createTestAppointment(
        const std::string &id = "A001",
        const std::string &username = "pat",
        const std::string &doctorID = "D001")
    {
        return Appointment(id, username, doctorID,
                           "2025-01-15", "09:00",
                           "Checkup", 200000, false,
                           AppointmentStatus::COMPLETED, "");
    }

    Medicine createTestMedicine(
        const std::string &id = "M001",
        const std::string &name = "Paracetamol",
        int stock = 100,
        double price = 10000)
    {
        Medicine m(id, name, "Tablet", price,stock);
        return m;
    }
};

// ==================== SINGLETON ====================
TEST_F(PrescriptionServiceTest, Singleton_ReturnsSameInstance)
{
    EXPECT_EQ(PrescriptionService::getInstance(),
              PrescriptionService::getInstance());
}

// ==================== CREATE ====================
TEST_F(PrescriptionServiceTest, CreatePrescription_Valid)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    apptRepo->add(createTestAppointment());

    auto result = service->createPrescription("A001", "Flu", "Take rest");
    EXPECT_TRUE(result.has_value());
}

TEST_F(PrescriptionServiceTest, CreatePrescription_DuplicateAppointment)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    apptRepo->add(createTestAppointment());

    service->createPrescription("A001", "Flu", "");
    EXPECT_FALSE(service->createPrescription("A001", "Again", "").has_value());
}

// ==================== VALIDATION ====================
TEST_F(PrescriptionServiceTest, CreatePrescription_InvalidAppointment)
{
    EXPECT_FALSE(service->createPrescription("INVALID", "Diag", "").has_value());
}

// ==================== QUERY ====================
TEST_F(PrescriptionServiceTest, GetPrescriptionByAppointment)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    apptRepo->add(createTestAppointment());

    auto pres = service->createPrescription("A001", "Cold", "");
    ASSERT_TRUE(pres.has_value());

    EXPECT_TRUE(service->getPrescriptionByAppointment("A001").has_value());
}

TEST_F(PrescriptionServiceTest, GetPrescriptionByID_NotExists)
{
    EXPECT_FALSE(service->getPrescriptionByID("PX").has_value());
}

// ==================== ITEM MANAGEMENT ====================
TEST_F(PrescriptionServiceTest, AddPrescriptionItem_Valid)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    auto medRepo = DAL::MedicineRepository::getInstance();

    apptRepo->add(createTestAppointment());
    medRepo->add(createTestMedicine());

    auto pres = service->createPrescription("A001", "Fever", "");
    ASSERT_TRUE(pres.has_value());

    EXPECT_TRUE(service->addPrescriptionItem(
        pres->getPrescriptionID(),
        "M001", 2, "2/day", "5 days", "After meal"));
}

TEST_F(PrescriptionServiceTest, RemovePrescriptionItem)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    auto medRepo = DAL::MedicineRepository::getInstance();

    apptRepo->add(createTestAppointment());
    medRepo->add(createTestMedicine());

    auto pres = service->createPrescription("A001", "Pain", "");
    service->addPrescriptionItem(
        pres->getPrescriptionID(), "M001", 1, "", "", "");

    EXPECT_TRUE(service->removePrescriptionItem(
        pres->getPrescriptionID(), "M001"));
}

// ==================== DISPENSE ====================
TEST_F(PrescriptionServiceTest, DispensePrescription_Success)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    auto medRepo = DAL::MedicineRepository::getInstance();

    apptRepo->add(createTestAppointment());
    medRepo->add(createTestMedicine("M001", "Para", 10, 5000));

    auto pres = service->createPrescription("A001", "Pain", "");
    service->addPrescriptionItem(
        pres->getPrescriptionID(), "M001", 2, "", "", "");

    auto result = service->dispensePrescription(pres->getPrescriptionID());

    EXPECT_TRUE(result.success);
    EXPECT_DOUBLE_EQ(result.totalCost, 10000);
}

TEST_F(PrescriptionServiceTest, DispensePrescription_InsufficientStock)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    auto medRepo = DAL::MedicineRepository::getInstance();

    apptRepo->add(createTestAppointment());
    medRepo->add(createTestMedicine("M001", "Para", 1, 5000));

    auto pres = service->createPrescription("A001", "Pain", "");
    service->addPrescriptionItem(
        pres->getPrescriptionID(), "M001", 5, "", "", "");

    auto result = service->dispensePrescription(pres->getPrescriptionID());
    EXPECT_FALSE(result.success);
}

// ==================== COST ====================
TEST_F(PrescriptionServiceTest, CalculatePrescriptionCost)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    auto medRepo = DAL::MedicineRepository::getInstance();

    apptRepo->add(createTestAppointment());
    medRepo->add(createTestMedicine("M001", "Para", 10, 5000));
    medRepo->add(createTestMedicine("M002", "Vitamin", 10, 2000));

    auto pres = service->createPrescription("A001", "General", "");
    service->addPrescriptionItem(pres->getPrescriptionID(), "M001", 2, "", "", "");
    service->addPrescriptionItem(pres->getPrescriptionID(), "M002", 3, "", "", "");

    EXPECT_DOUBLE_EQ(service->calculatePrescriptionCost(
                         pres->getPrescriptionID()),
                     16000);
}

// ==================== STATISTICS ====================
TEST_F(PrescriptionServiceTest, PrescriptionStatistics)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    apptRepo->add(createTestAppointment("A1"));
    apptRepo->add(createTestAppointment("A2"));

    service->createPrescription("A1", "D1", "");
    service->createPrescription("A2", "D2", "");

    auto stats = service->getPrescriptionStatistics();
    EXPECT_EQ(stats["total"], 2);
}

// ==================== PERSISTENCE ====================
TEST_F(PrescriptionServiceTest, SaveAndLoadData)
{
    auto apptRepo = DAL::AppointmentRepository::getInstance();
    apptRepo->add(createTestAppointment());

    service->createPrescription("A001", "SaveTest", "");
    ASSERT_TRUE(service->saveData());

    DAL::PrescriptionRepository::resetInstance();
    DAL::AppointmentRepository::resetInstance();
    DAL::MedicineRepository::resetInstance();
    PrescriptionService::resetInstance();

    auto presRepo = DAL::PrescriptionRepository::getInstance();
    presRepo->setFilePath(TEST_PRESCRIPTION_FILE);

    auto freshService = PrescriptionService::getInstance();
    ASSERT_TRUE(freshService->loadData());

    EXPECT_EQ(freshService->getPrescriptionCount(), 1u);
}

/*
./HospitalTests --gtest_filter="PrescriptionServiceTest.*"
*/
