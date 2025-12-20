#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

// Include Service & Repositories
#include "bll/AppointmentService.h"
#include "dal/AppointmentRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/PatientRepository.h"
#include "common/Utils.h"
#include "common/Constants.h"

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Constants ====================
const std::string TEST_DIR = "test_data_service/";
const std::string TEST_APT_FILE = TEST_DIR + "AppointmentTest.txt";
const std::string TEST_DOC_FILE = TEST_DIR + "DoctorTest.txt";
const std::string TEST_PAT_FILE = TEST_DIR + "PatientTest.txt";

const std::string VALID_DOC_ID = "D001";
const std::string VALID_PAT_USER = "test_patient";
const double DOC_FEE = 500000.0;

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

        // 2. Reset Singleton
        AppointmentService::resetInstance();
        AppointmentRepository::resetInstance();
        DoctorRepository::resetInstance();
        PatientRepository::resetInstance();

        // 3. Get Instances
        service = AppointmentService::getInstance();
        aptRepo = AppointmentRepository::getInstance();
        docRepo = DoctorRepository::getInstance();
        patRepo = PatientRepository::getInstance();

        // 4. Config Paths
        aptRepo->setFilePath(TEST_APT_FILE);
        docRepo->setFilePath(TEST_DOC_FILE);
        patRepo->setFilePath(TEST_PAT_FILE);

        // 5. Clear Data
        aptRepo->clear();
        docRepo->clear();
        patRepo->clear();

        // 6. Seed Data
        seedDoctor();
        seedPatient();
    }

    void TearDown() override {
        aptRepo->clear();
        docRepo->clear();
        patRepo->clear();
        std::filesystem::remove_all(TEST_DIR);
    }

    // --- Helpers ---

    void seedDoctor() {
        // ID, Username, Name, Phone, Gender, DOB, Specialization, Schedule, Fee
        Doctor d(VALID_DOC_ID, "dr_smith", "Dr. Smith", "0900000001",
                 Gender::MALE, "1980-01-01", "Cardiology", "Mon-Fri", DOC_FEE);
        docRepo->add(d);
    }

    void seedPatient() {
        // ID, Username, Name, Phone, Gender, DOB, Address, History
        Patient p("P001", VALID_PAT_USER, "John Doe", "0900000002",
                  Gender::MALE, "1990-01-01", "123 Main St", "None");
        patRepo->add(p);
    }

    std::string getFutureDate() {
        // Trả về ngày năm 2030 để luôn là tương lai
        return "2030-01-01";
    }

    std::string getTomorrow() {
        // Logic lấy ngày mai đơn giản (cần cải thiện nếu dùng production, nhưng đủ cho test)
        // Ở đây dùng chuỗi cố định xa để an toàn, hoặc dùng Utils nếu có hàm addDays
        // Để đơn giản cho bài test này, ta giả định Utils::getCurrentDate trả về hôm nay
        // Ta sẽ dùng date logic đơn giản bằng cách cộng string năm.
        std::string today = Utils::getCurrentDate();
        int year = std::stoi(today.substr(0, 4));
        return std::to_string(year + 1) + "-01-01";
    }

    // Helper để inject trực tiếp 1 appointment vào Repo (bỏ qua validation của Service)
    // Dùng để test các case sửa/xóa lịch sử cũ
    void injectPastAppointment(const std::string& id) {
        Appointment appt(id, VALID_PAT_USER, VALID_DOC_ID, "2000-01-01", "09:00",
                         "History", DOC_FEE, false, AppointmentStatus::SCHEDULED, "");
        aptRepo->add(appt);
    }
};

// ==================== 1. BOOKING VALIDATION TESTS (Edge Cases) ====================

TEST_F(AppointmentServiceTest, Booking_Fail_InvalidDateFormat) {
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, "2030/01/01", "09:00", "Checkup");
    EXPECT_FALSE(result.has_value()) << "Should fail with date format YYYY/MM/DD";

    result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, "invalid-date", "09:00", "Checkup");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentServiceTest, Booking_Fail_InvalidTimeFormat) {
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "9:00", "Checkup");
    EXPECT_FALSE(result.has_value()) << "Should fail with H:MM (needs HH:MM)";

    result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "25:00", "Checkup");
    EXPECT_FALSE(result.has_value()) << "Should fail with invalid hour";
}

TEST_F(AppointmentServiceTest, Booking_Fail_NonStandardTimeSlot) {
    // Hệ thống chỉ cho phép đặt 08:00, 08:30... không cho đặt 08:15
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "08:15", "Checkup");
    EXPECT_FALSE(result.has_value()) << "Should fail with non-standard time slot";
}

TEST_F(AppointmentServiceTest, Booking_Fail_PastDate) {
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, "2020-01-01", "09:00", "Checkup");
    EXPECT_FALSE(result.has_value()) << "Should fail when booking in the past";
}

TEST_F(AppointmentServiceTest, Booking_Fail_PatientNotFound) {
    auto result = service->bookAppointment("non_existent_user", VALID_DOC_ID, getFutureDate(), "09:00", "Checkup");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentServiceTest, Booking_Fail_DoctorNotFound) {
    auto result = service->bookAppointment(VALID_PAT_USER, "D999", getFutureDate(), "09:00", "Checkup");
    EXPECT_FALSE(result.has_value());
}

// ==================== 2. BOOKING LOGIC TESTS ====================

TEST_F(AppointmentServiceTest, Booking_Success_Standard) {
    std::string date = getFutureDate();
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "09:00", "Fever");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientUsername(), VALID_PAT_USER);
    EXPECT_EQ(result->getDoctorID(), VALID_DOC_ID);
    EXPECT_EQ(result->getStatus(), AppointmentStatus::SCHEDULED);
    EXPECT_DOUBLE_EQ(result->getPrice(), DOC_FEE); // Verify fee fetched correctly
}

TEST_F(AppointmentServiceTest, Booking_Fail_SlotTaken) {
    std::string date = getFutureDate();
    // Book lần 1
    ASSERT_TRUE(service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "10:00", "A").has_value());

    // Book lần 2 cùng ngày giờ bác sĩ đó
    auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "10:00", "B");
    EXPECT_FALSE(result.has_value()) << "Should fail when slot is already taken";
}

TEST_F(AppointmentServiceTest, Booking_Today_Logic) {
    std::string today = Utils::getCurrentDate();
    std::string now = Utils::getCurrentTime();

    // Case 1: Book hôm nay nhưng giờ đã qua (Past Time)
    // Giả sử bây giờ là 12:00, book 08:00
    if (now > "08:00") {
        auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, today, "08:00", "Checkup");
        EXPECT_FALSE(result.has_value()) << "Should fail booking past time on current day";
    }

    // Case 2: Book hôm nay nhưng giờ tương lai (Future Time)
    // Giả sử bây giờ là 08:00, book 16:30 (ca cuối)
    if (now < "16:30") {
        auto result = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, today, "16:30", "Checkup");
        EXPECT_TRUE(result.has_value()) << "Should succeed booking future time on current day";
    }
}

// ==================== 3. EDIT & CANCEL TESTS ====================

TEST_F(AppointmentServiceTest, Cancel_Success) {
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "A");
    ASSERT_TRUE(appt.has_value());

    bool success = service->cancelAppointment(appt->getAppointmentID());
    EXPECT_TRUE(success);

    // Verify status in Repo
    auto updated = service->getAppointmentByID(appt->getAppointmentID());
    EXPECT_EQ(updated->getStatus(), AppointmentStatus::CANCELLED);
}

TEST_F(AppointmentServiceTest, Cancel_Fail_PastAppointment) {
    std::string pastID = "APT_PAST";
    injectPastAppointment(pastID); // Inject lịch năm 2000

    bool success = service->cancelAppointment(pastID);
    EXPECT_FALSE(success) << "Should not allow cancelling past appointments";
}

TEST_F(AppointmentServiceTest, Edit_Success_ChangeTime) {
    std::string date = getFutureDate();
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "09:00", "A");

    // Đổi sang 10:00
    bool success = service->editAppointment(appt->getAppointmentID(), "", "10:00");
    EXPECT_TRUE(success);

    auto updated = service->getAppointmentByID(appt->getAppointmentID());
    EXPECT_EQ(updated->getTime(), "10:00");
}

TEST_F(AppointmentServiceTest, Edit_Fail_SlotTaken) {
    std::string date = getFutureDate();
    // Book slot 09:00
    auto appt1 = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "09:00", "A");
    // Book slot 10:00
    auto appt2 = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "10:00", "B");

    // Thử đổi appt2 sang 09:00 (đã bị appt1 chiếm)
    bool success = service->editAppointment(appt2->getAppointmentID(), "", "09:00");
    EXPECT_FALSE(success) << "Should fail to edit into an occupied slot";
}

// ==================== 4. WORKFLOW STATUS TESTS ====================

TEST_F(AppointmentServiceTest, Workflow_MarkCompleted) {
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "A");

    bool success = service->markAsCompleted(appt->getAppointmentID());
    EXPECT_TRUE(success);

    auto updated = service->getAppointmentByID(appt->getAppointmentID());
    EXPECT_EQ(updated->getStatus(), AppointmentStatus::COMPLETED);
}

TEST_F(AppointmentServiceTest, Workflow_Fail_CompletedToNoShow) {
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "A");
    service->markAsCompleted(appt->getAppointmentID());

    // Thử đánh dấu NoShow cho lịch đã hoàn thành
    bool success = service->markAsNoShow(appt->getAppointmentID());
    EXPECT_FALSE(success) << "Cannot mark COMPLETED appointment as NO_SHOW";
}

TEST_F(AppointmentServiceTest, Workflow_MarkPaid) {
    auto appt = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, getFutureDate(), "09:00", "A");

    EXPECT_FALSE(appt->isPaid());
    service->markAsPaid(appt->getAppointmentID());

    auto updated = service->getAppointmentByID(appt->getAppointmentID());
    EXPECT_TRUE(updated->isPaid());
}

// ==================== 5. AVAILABILITY & STATISTICS TESTS ====================

TEST_F(AppointmentServiceTest, Availability_Check) {
    std::string date = getFutureDate();
    auto slotsInitial = service->getAvailableSlots(VALID_DOC_ID, date);

    // Book 1 slot lúc 09:00
    service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "09:00", "A");

    auto slotsAfter = service->getAvailableSlots(VALID_DOC_ID, date);

    // Kích thước phải giảm đi 1
    EXPECT_EQ(slotsAfter.size(), slotsInitial.size() - 1);

    // Slot 09:00 không còn tồn tại
    auto it = std::find(slotsAfter.begin(), slotsAfter.end(), "09:00");
    EXPECT_EQ(it, slotsAfter.end());
}

TEST_F(AppointmentServiceTest, Statistics_Revenue) {
    std::string date = getFutureDate();

    // 1. Appt Paid (Completed) -> Doanh thu 500k
    auto a1 = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "08:00", "A");
    service->markAsCompleted(a1->getAppointmentID());
    service->markAsPaid(a1->getAppointmentID());

    // 2. Appt Unpaid (Scheduled) -> Doanh thu 500k (tùy logic getTotalRevenue đếm cả chưa thanh toán hay không)
    // Theo logic trong Service bạn viết: getTotalRevenue đếm tất cả ngoại trừ Cancelled.
    auto a2 = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "09:00", "B");

    // 3. Appt Cancelled -> Doanh thu 0
    auto a3 = service->bookAppointment(VALID_PAT_USER, VALID_DOC_ID, date, "10:00", "C");
    service->cancelAppointment(a3->getAppointmentID());

    double expectedTotal = DOC_FEE * 2; // a1 + a2
    double expectedPaid = DOC_FEE;      // a1
    double expectedUnpaid = DOC_FEE;    // a2

    EXPECT_DOUBLE_EQ(service->getTotalRevenue(), expectedTotal);
    EXPECT_DOUBLE_EQ(service->getPaidRevenue(), expectedPaid);
    EXPECT_DOUBLE_EQ(service->getUnpaidRevenue(), expectedUnpaid);
}

// Main function cho GTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}