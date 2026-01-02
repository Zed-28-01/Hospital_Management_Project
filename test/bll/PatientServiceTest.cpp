#include <gtest/gtest.h>
#include <filesystem>

#include "bll/PatientService.h"
#include "model/Patient.h"
#include "model/Appointment.h"
#include "common/Types.h"

// ==================== USING ====================
using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::Model;

namespace
{
    const std::string TEST_DATA_DIR = "test_data/";
    const std::string TEST_PATIENT_FILE = "test_data/patients_test.txt";
    const std::string TEST_APPOINTMENT_FILE = "test_data/appointments_test.txt";
}

// ==================== TEST FIXTURE ====================
class PatientServiceTest : public ::testing::Test
{
protected:
    PatientService *service;

    void SetUp() override
    {
        std::filesystem::create_directories(TEST_DATA_DIR);

        PatientService::resetInstance();
        service = PatientService::getInstance();

        auto patientRepo = DAL::PatientRepository::getInstance();
        auto appointmentRepo = DAL::AppointmentRepository::getInstance();

        patientRepo->setFilePath(TEST_PATIENT_FILE);
        appointmentRepo->setFilePath(TEST_APPOINTMENT_FILE);

        patientRepo->clear();
        appointmentRepo->clear();
    }

    void TearDown() override
    {
        auto patientRepo = DAL::PatientRepository::getInstance();
        auto appointmentRepo = DAL::AppointmentRepository::getInstance();

        patientRepo->clear();
        appointmentRepo->clear();

        PatientService::resetInstance();
    }

    Patient createTestPatient(
        const std::string &id = "P001",
        const std::string &username = "user1",
        const std::string &name = "John Doe",
        const std::string &phone = "0123456789",
        Gender gender = Gender::MALE,
        const std::string &dob = "1990-01-01",
        const std::string &address = "Address",
        const std::string &history = "")
    {
        return Patient(id, username, name, phone, gender, dob, address, history);
    }

    Appointment createTestAppointment(
        const std::string &id,
        const std::string &username,
        double price = 500000,
        bool paid = false,
        AppointmentStatus status = AppointmentStatus::COMPLETED)
    {
        return Appointment(id, username, "D001", "2025-01-15", "10:00",
                           "Checkup", price, paid, status, "");
    }
};

// ==================== SINGLETON ====================
TEST_F(PatientServiceTest, Singleton_ReturnsSameInstance)
{
    EXPECT_EQ(PatientService::getInstance(), PatientService::getInstance());
}

// ==================== CREATE ====================
TEST_F(PatientServiceTest, CreatePatient_Valid)
{
    EXPECT_TRUE(service->createPatient(createTestPatient()));
}

TEST_F(PatientServiceTest, CreatePatient_DuplicateID)
{
    service->createPatient(createTestPatient("P001", "u1"));
    EXPECT_FALSE(service->createPatient(createTestPatient("P001", "u2")));
}

TEST_F(PatientServiceTest, CreatePatient_DuplicateUsername)
{
    service->createPatient(createTestPatient("P001", "same"));
    EXPECT_FALSE(service->createPatient(createTestPatient("P002", "same")));
}

// ==================== VALIDATION ====================
TEST_F(PatientServiceTest, Validation_EmptyID)
{
    EXPECT_FALSE(service->createPatient(createTestPatient("", "u1")));
}

TEST_F(PatientServiceTest, Validation_EmptyUsername)
{
    EXPECT_FALSE(service->createPatient(createTestPatient("P001", "")));
}

TEST_F(PatientServiceTest, Validation_ShortName)
{
    EXPECT_FALSE(service->createPatient(createTestPatient("P001", "u1", "A")));
}

TEST_F(PatientServiceTest, Validation_InvalidPhone)
{
    EXPECT_FALSE(service->createPatient(
        createTestPatient("P001", "u1", "John", "123")));
}

TEST_F(PatientServiceTest, Validation_InvalidDOB)
{
    EXPECT_FALSE(service->createPatient(
        createTestPatient("P001", "u1", "John", "0123456789",
                          Gender::MALE, "invalid")));
}

TEST_F(PatientServiceTest, Validation_FutureDOB)
{
    EXPECT_FALSE(service->createPatient(
        createTestPatient("P001", "u1", "John", "0123456789",
                          Gender::MALE, "2099-01-01")));
}

TEST_F(PatientServiceTest, Validation_EmptyAddress)
{
    EXPECT_FALSE(service->createPatient(
        createTestPatient("P001", "u1", "John", "0123456789",
                          Gender::MALE, "1990-01-01", "")));
}

TEST_F(PatientServiceTest, Validation_UnknownGender)
{
    EXPECT_FALSE(service->createPatient(
        createTestPatient("P001", "u1", "John", "0123456789",
                          Gender::UNKNOWN)));
}

// ==================== UPDATE ====================
TEST_F(PatientServiceTest, UpdatePatient_Valid)
{
    service->createPatient(createTestPatient());
    auto p = service->getPatientByID("P001").value();
    p.setAddress("New Address");
    EXPECT_TRUE(service->updatePatient(p));
}

TEST_F(PatientServiceTest, UpdatePatient_NotExists)
{
    EXPECT_FALSE(service->updatePatient(createTestPatient("P999")));
}

TEST_F(PatientServiceTest, UpdatePatient_InvalidData)
{
    service->createPatient(createTestPatient());
    auto p = service->getPatientByID("P001").value();
    p.setAddress("");
    EXPECT_FALSE(service->updatePatient(p));
}

// ==================== DELETE ====================
TEST_F(PatientServiceTest, DeletePatient_PreservesAppointments)
{
    // Appointments should NOT be deleted when patient is deleted
    // This preserves historical records, audit trail, and referential integrity
    service->createPatient(createTestPatient("P001", "pat"));
    auto repo = DAL::AppointmentRepository::getInstance();
    repo->add(createTestAppointment("A1", "pat"));
    repo->add(createTestAppointment("A2", "pat"));
    EXPECT_TRUE(service->deletePatient("P001"));
    // Appointments remain as historical data
    EXPECT_FALSE(repo->getByPatient("pat").empty());
    EXPECT_EQ(repo->getByPatient("pat").size(), 2);
}

TEST_F(PatientServiceTest, DeletePatient_NotExists)
{
    EXPECT_FALSE(service->deletePatient("P999"));
}

// ==================== QUERY ====================
TEST_F(PatientServiceTest, GetPatientByUsername)
{
    service->createPatient(createTestPatient("P001", "findme"));
    EXPECT_TRUE(service->getPatientByUsername("findme").has_value());
}

TEST_F(PatientServiceTest, GetPatientByID_NotExists)
{
    EXPECT_FALSE(service->getPatientByID("P999").has_value());
}

// ==================== BILLING ====================
TEST_F(PatientServiceTest, CalculateTotalBill_MixedStatus)
{
    service->createPatient(createTestPatient("P001", "pat"));

    auto repo = DAL::AppointmentRepository::getInstance();
    repo->add(createTestAppointment("A1", "pat", 500000, false, AppointmentStatus::COMPLETED));
    repo->add(createTestAppointment("A2", "pat", 300000, false, AppointmentStatus::CANCELLED));

    EXPECT_DOUBLE_EQ(service->calculateTotalBill("pat"), 500000);
}

TEST_F(PatientServiceTest, CalculateTotalBill_AllPaid)
{
    service->createPatient(createTestPatient("P001", "pat"));
    auto repo = DAL::AppointmentRepository::getInstance();
    repo->add(createTestAppointment("A1", "pat", 500000, true));
    EXPECT_DOUBLE_EQ(service->calculateTotalBill("pat"), 0.0);
}

// ==================== UNPAID ====================
TEST_F(PatientServiceTest, GetUnpaidAppointments_Filtered)
{
    service->createPatient(createTestPatient("P001", "pat"));
    auto repo = DAL::AppointmentRepository::getInstance();

    repo->add(createTestAppointment("A1", "pat", 500000, false, AppointmentStatus::SCHEDULED));
    repo->add(createTestAppointment("A2", "pat", 300000, false, AppointmentStatus::CANCELLED));

    EXPECT_EQ(service->getUnpaidAppointments("pat").size(), 1u);
}

// ==================== MEDICAL HISTORY ====================
TEST_F(PatientServiceTest, AppendMedicalHistory)
{
    service->createPatient(createTestPatient("P001", "pat", "Name",
                                             "0123456789", Gender::MALE, "1990-01-01", "Addr", "Init"));

    EXPECT_TRUE(service->appendMedicalHistory("P001", "New entry"));

    auto p = service->getPatientByID("P001").value();
    EXPECT_NE(p.getMedicalHistory().find("New entry"), std::string::npos);
}

// ==================== EDGE ====================
TEST_F(PatientServiceTest, AppointmentHistory_NonExistingUser)
{
    EXPECT_TRUE(service->getAppointmentHistory("ghost").empty());
}

// ==================== STRESS ====================
TEST_F(PatientServiceTest, CreateManyPatients)
{
    for (int i = 1; i <= 10; i++)
    {
        service->createPatient(createTestPatient(
            "P" + std::to_string(i),
            "user" + std::to_string(i)));
    }
    EXPECT_EQ(service->getPatientCount(), 10u);
}

// ==================== CREATE (OVERLOAD) ====================
TEST_F(PatientServiceTest, CreatePatient_AutoGenerateID)
{
    // Test hàm createPatient nhận tham số rời và tự sinh ID
    auto patientOpt = service->createPatient(
        "newuser", "New Patient", "0987654321",
        Gender::FEMALE, "1995-05-05", "456 Street", "No history");

    ASSERT_TRUE(patientOpt.has_value());
    EXPECT_FALSE(patientOpt->getPatientID().empty());
    EXPECT_EQ(patientOpt->getUsername(), "newuser");
    EXPECT_TRUE(service->patientExists(patientOpt->getPatientID()));
}

// ==================== SEARCH ====================
TEST_F(PatientServiceTest, SearchPatients_ByKeyword)
{
    service->createPatient(createTestPatient("P001", "user1", "Alice Pham"));
    service->createPatient(createTestPatient("P002", "user2", "Bob Tran"));
    service->createPatient(createTestPatient("P003", "user3", "Charlie Pham"));

    // Tìm kiếm từ khóa "Pham"
    auto results = service->searchPatients("Pham");
    EXPECT_EQ(results.size(), 2u);

    // Tìm kiếm trống (phải trả về tất cả)
    auto allResults = service->searchPatients("");
    EXPECT_EQ(allResults.size(), 3u);
}

// ==================== SORTING LOGIC ====================
TEST_F(PatientServiceTest, GetUnpaidAppointments_SortingOrder)
{
    service->createPatient(createTestPatient("P001", "pat"));
    auto repo = DAL::AppointmentRepository::getInstance();

    // Thêm các lịch hẹn sai thứ tự thời gian
    // A1: Ngày 20, A2: Ngày 10 (cùng tháng), A3: Ngày 10 nhưng giờ muộn hơn
    repo->add(Appointment("A1", "pat", "D1", "2025-01-20", "10:00", "", 100, false, AppointmentStatus::SCHEDULED, ""));
    repo->add(Appointment("A2", "pat", "D1", "2025-01-10", "08:00", "", 100, false, AppointmentStatus::SCHEDULED, ""));
    repo->add(Appointment("A3", "pat", "D1", "2025-01-10", "09:00", "", 100, false, AppointmentStatus::SCHEDULED, ""));

    auto results = service->getUnpaidAppointments("pat");

    ASSERT_EQ(results.size(), 3u);
    // Kiểm tra thứ tự: A2 (8:00) -> A3 (9:00) -> A1 (Ngày 20)
    EXPECT_EQ(results[0].getAppointmentID(), "A2");
    EXPECT_EQ(results[1].getAppointmentID(), "A3");
    EXPECT_EQ(results[2].getAppointmentID(), "A1");
}

// ==================== PERSISTENCE (IO) ====================
TEST_F(PatientServiceTest, SaveAndLoadData)
{
    // 1. Create data and save
    service->createPatient(createTestPatient("P001", "save_test", "Save Test"));
    ASSERT_TRUE(service->saveData());

    // 2. Reset both repository AND service to simulate app restart
    // IMPORTANT: Must reset service since it holds pointers to repositories.
    // Resetting only repository causes dangling pointer in service -> crash
    DAL::PatientRepository::resetInstance();
    DAL::AppointmentRepository::resetInstance();
    PatientService::resetInstance();

    // 3. Get fresh instances (simulating app restart)
    auto freshRepo = DAL::PatientRepository::getInstance();
    freshRepo->setFilePath(TEST_PATIENT_FILE);

    auto freshApptRepo = DAL::AppointmentRepository::getInstance();
    freshApptRepo->setFilePath(TEST_APPOINTMENT_FILE);

    service = PatientService::getInstance();

    // 4. Load data from file
    ASSERT_TRUE(service->loadData());
    EXPECT_EQ(service->getPatientCount(), 1u);
    EXPECT_TRUE(service->getPatientByUsername("save_test").has_value());
}

// ==================== EXTRA BOUNDARY ====================
TEST_F(PatientServiceTest, UpdateMedicalHistory_NonExistingPatient)
{
    EXPECT_FALSE(service->updateMedicalHistory("P999", "Some history"));
    EXPECT_FALSE(service->appendMedicalHistory("P999", "Some entry"));
}

TEST_F(PatientServiceTest, CalculateTotalPaid_Mixed)
{
    service->createPatient(createTestPatient("P001", "pat"));
    auto repo = DAL::AppointmentRepository::getInstance();

    repo->add(createTestAppointment("A1", "pat", 500000, true));  // Đã trả
    repo->add(createTestAppointment("A2", "pat", 200000, false)); // Chưa trả

    EXPECT_DOUBLE_EQ(service->calculateTotalPaid("pat"), 500000.0);
}

/*
./HospitalTests --gtest_filter="PatientServiceTest.*"
*/
