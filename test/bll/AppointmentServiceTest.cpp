#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>

// Include Service & Repositories
#include "bll/AppointmentService.h"
#include "dal/AppointmentRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/PatientRepository.h"
#include "common/Utils.h"

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Constants for Testing ====================
const std::string TEST_DIR = "test_data/";
const std::string TEST_APT_FILE = "test_data/AppointmentTest.txt";
const std::string TEST_DOC_FILE = "test_data/DoctorTest.txt";
const std::string TEST_PAT_FILE = "test_data/PatientTest.txt";

const std::string VALID_DOC_ID = "D001";
const std::string VALID_PAT_USER = "test_patient";
const std::string INVALID_ID = "NON_EXIST";

// ==================== Test Fixture ====================
class AppointmentServiceTest : public ::testing::Test {
protected:
    AppointmentService* service;
    AppointmentRepository* aptRepo;
    DoctorRepository* docRepo;
    PatientRepository* patRepo;

    void SetUp() override {
        // 1. Tạo thư mục test
        std::filesystem::create_directories(TEST_DIR);

        // 2. Reset và lấy Instance
        AppointmentService::resetInstance();
        AppointmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        PatientRepository::resetInstance();

        service = AppointmentService::getInstance();
        aptRepo = AppointmentRepository::getInstance();
        docRepo = DoctorRepository::getInstance();
        patRepo = PatientRepository::getInstance();

        // 3. Trỏ Repository vào file test
        aptRepo->setFilePath(TEST_APT_FILE);
        docRepo->setFilePath(TEST_DOC_FILE);
        patRepo->setFilePath(TEST_PAT_FILE);

        // 4. Xóa dữ liệu cũ
        aptRepo->clear();
        docRepo->clear();
        patRepo->clear();

        // 5. SEED DATA
        seedDoctor();
        seedPatient();
    }

    void TearDown() override {
        aptRepo->clear();
        docRepo->clear();
        patRepo->clear();
        std::filesystem::remove_all(TEST_DIR);
    }

    // Helper: Tạo bác sĩ giả
    void seedDoctor() {
        // Giả định Constructor Doctor (bạn kiểm tra lại file Doctor.h nếu lỗi nhé)
        Doctor d(VALID_DOC_ID, "dr_smith", "Dr. Smith", "0900000001",
                 Gender::MALE, "1980-01-01", "Cardiology", "Mon-Fri", 500000);
        docRepo->add(d);
    }

    // Helper: Tạo bệnh nhân giả (ĐÃ SỬA KHỚP FILE HEADER)
    void seedPatient() {
        Patient p(
            "P001",             // patientID
            VALID_PAT_USER,     // username
            "John Doe",         // name
            "0900000002",       // phone
            Gender::MALE,       // gender
            "1990-01-01",       // dateOfBirth
            "123 Main St",      // address
            "No history"        // medicalHistory
        );
        patRepo->add(p);
    }

    // Helper: Lấy ngày tương lai
    std::string getFutureDate() {
        return "2030-01-01";
    }

    // Helper: Lấy ngày quá khứ
    std::string getPastDate() {
        return "2020-01-01";
    }
};

// ==================== TEST CASES ====================

TEST_F(AppointmentServiceTest, BookAppointment_Success) {
    auto result = service->bookAppointment(
        VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "Fever");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientUsername(), VALID_PAT_USER);
    EXPECT_EQ(result->getStatus(), AppointmentStatus::SCHEDULED);
}

TEST_F(AppointmentServiceTest, BookAppointment_Fail_PatientNotFound) {
    auto result = service->bookAppointment(
        "ghost_user", VALID_DOC_ID, getFutureDate(), "09:00", "Fever");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentServiceTest, BookAppointment_Fail_SlotTaken) {
    service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "Fever");
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "Cold");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentServiceTest, CancelAppointment_Success) {
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "Fever");
    EXPECT_TRUE(service->cancelAppointment(appt->getAppointmentID()));
}

TEST_F(AppointmentServiceTest, GetAvailableSlots) {
    auto slots = service->getAvailableSlots(VALID_DOC_ID, getFutureDate());
    EXPECT_FALSE(slots.empty());
    EXPECT_NE(std::find(slots.begin(), slots.end(), "08:00"), slots.end());
}