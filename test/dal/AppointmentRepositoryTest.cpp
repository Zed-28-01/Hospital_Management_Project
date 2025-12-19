#include <gtest/gtest.h>

#include "dal/AppointmentRepository.h"
#include "model/Appointment.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "dal/FileHelper.h"

#include <filesystem>
#include <fstream>
#include <optional> // Bổ sung thư viện thiếu

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace
{

    const std::string TEST_DATA_DIR = "test/fixtures/";
    const std::string TEST_DATA_FILE = "test/fixtures/AppointmentTest.txt";

    // Helper tạo appointment hợp lệ
    // Giả định constructor của Appointment khớp với thứ tự tham số này
    Appointment makeAppointment(
        const std::string &id,
        const std::string &patient,
        const std::string &doctor,
        const std::string &date = "2030-01-01",
        const std::string &time = "09:00",
        AppointmentStatus status = AppointmentStatus::SCHEDULED,
        bool paid = false)
    {
        return Appointment(
            id,
            patient,
            doctor,
            date,
            time,
            "Flu",      // Diagnosis
            100.0,      // Fee
            paid,       // IsPaid
            status,     // Status
            "Test note" // Notes
        );
    }

} // anonymous namespace

// ============================================================================
// Test Fixture
// ============================================================================
class AppointmentRepositoryTest : public ::testing::Test
{
protected:
    AppointmentRepository *repo;

    void SetUp() override
    {
        // 1. Reset singleton để đảm bảo môi trường sạch
        AppointmentRepository::resetInstance();
        repo = AppointmentRepository::getInstance();

        // 2. Thiết lập đường dẫn file test
        repo->setFilePath(TEST_DATA_FILE);

        // 3. Chuẩn bị file vật lý (Xóa trắng)
        std::filesystem::create_directories(TEST_DATA_DIR);
        std::ofstream ofs(TEST_DATA_FILE, std::ios::trunc);
        ofs.close();

        // 4. Clear dữ liệu trong bộ nhớ (và đồng bộ với file rỗng)
        // Việc này cũng kích hoạt load() nội bộ nếu cần thiết
        repo->clear();
    }

    void TearDown() override
    {
        // Dọn dẹp sau khi test
        repo->clear();
        AppointmentRepository::resetInstance();
        // Tùy chọn: Xóa file test nếu muốn tiết kiệm dung lượng
        // std::filesystem::remove(TEST_DATA_FILE);
    }
};

// ============================================================================
// BASIC CRUD
// ============================================================================

TEST_F(AppointmentRepositoryTest, AddAppointmentSuccess)
{
    auto apt = makeAppointment("APT001", "patient1", "D001");
    EXPECT_TRUE(repo->add(apt));
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(AppointmentRepositoryTest, AddDuplicateAppointmentFails)
{
    auto apt = makeAppointment("APT001", "patient1", "D001");
    EXPECT_TRUE(repo->add(apt));

    // Thêm lại cùng ID sẽ thất bại
    EXPECT_FALSE(repo->add(apt));
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(AppointmentRepositoryTest, GetByIdFound)
{
    auto apt = makeAppointment("APT001", "patient1", "D001");
    repo->add(apt);

    auto result = repo->getById("APT001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientUsername(), "patient1");
}

TEST_F(AppointmentRepositoryTest, GetByIdNotFound)
{
    auto result = repo->getById("NOT_EXIST");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentRepositoryTest, UpdateAppointmentSuccess)
{
    auto apt = makeAppointment("APT001", "patient1", "D001");
    repo->add(apt);

    auto updated = makeAppointment(
        "APT001", "patient1", "D001",
        "2030-01-02", "10:00",
        AppointmentStatus::COMPLETED, true);

    EXPECT_TRUE(repo->update(updated));

    auto fetched = repo->getById("APT001");
    ASSERT_TRUE(fetched.has_value());
    EXPECT_TRUE(fetched->isPaid());
    EXPECT_EQ(fetched->getStatus(), AppointmentStatus::COMPLETED);
    EXPECT_EQ(fetched->getDate(), "2030-01-02");
}

TEST_F(AppointmentRepositoryTest, UpdateNonExistingFails)
{
    auto apt = makeAppointment("APT999", "p", "d");
    EXPECT_FALSE(repo->update(apt));
}

TEST_F(AppointmentRepositoryTest, RemoveAppointmentSuccess)
{
    repo->add(makeAppointment("APT001", "p1", "D1"));
    EXPECT_TRUE(repo->remove("APT001"));
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(AppointmentRepositoryTest, RemoveNonExistingFails)
{
    EXPECT_FALSE(repo->remove("APT404"));
}

// ============================================================================
// PATIENT QUERIES
// ============================================================================

TEST_F(AppointmentRepositoryTest, GetByPatient)
{
    repo->add(makeAppointment("APT1", "alice", "D1"));
    repo->add(makeAppointment("APT2", "bob", "D1"));
    repo->add(makeAppointment("APT3", "alice", "D2"));

    auto result = repo->getByPatient("alice");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(AppointmentRepositoryTest, GetUnpaidByPatient)
{
    repo->add(makeAppointment("APT1", "alice", "D1", "2030-01-01", "09:00", AppointmentStatus::SCHEDULED, false));
    repo->add(makeAppointment("APT2", "alice", "D1", "2030-01-01", "10:00", AppointmentStatus::SCHEDULED, true));

    auto unpaid = repo->getUnpaidByPatient("alice");
    EXPECT_EQ(unpaid.size(), 1);
    EXPECT_EQ(unpaid[0].getAppointmentID(), "APT1");
}

// ============================================================================
// DOCTOR QUERIES
// ============================================================================

TEST_F(AppointmentRepositoryTest, GetByDoctorAndDateSorted)
{
    // Add không theo thứ tự thời gian
    repo->add(makeAppointment("APT1", "p1", "D1", "2030-01-01", "10:00"));
    repo->add(makeAppointment("APT2", "p2", "D1", "2030-01-01", "09:00"));

    auto result = repo->getByDoctorAndDate("D1", "2030-01-01");
    ASSERT_EQ(result.size(), 2);
    // Kỳ vọng sắp xếp: 09:00 trước, 10:00 sau
    EXPECT_EQ(result[0].getTime(), "09:00");
    EXPECT_EQ(result[1].getTime(), "10:00");
}

// ============================================================================
// SLOT AVAILABILITY
// ============================================================================

TEST_F(AppointmentRepositoryTest, SlotAvailableWhenEmpty)
{
    EXPECT_TRUE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

TEST_F(AppointmentRepositoryTest, SlotUnavailableWhenScheduled)
{
    repo->add(makeAppointment("APT1", "p1", "D1", "2030-01-01", "09:00"));
    EXPECT_FALSE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

TEST_F(AppointmentRepositoryTest, SlotAvailableWhenCancelled)
{
    repo->add(makeAppointment(
        "APT1", "p1", "D1",
        "2030-01-01", "09:00",
        AppointmentStatus::CANCELLED));

    EXPECT_TRUE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

// ============================================================================
// FILE PERSISTENCE
// ============================================================================

TEST_F(AppointmentRepositoryTest, SaveAndLoadPersistence)
{
    // 1. Thêm dữ liệu vào repo hiện tại
    repo->add(makeAppointment("APT1", "alice", "D1"));
    repo->add(makeAppointment("APT2", "bob", "D2"));
    // Dữ liệu đã được save() vào file test nhờ hàm add()

    // 2. Reset Singleton để mô phỏng việc tắt/bật ứng dụng
    AppointmentRepository::resetInstance();

    // 3. Lấy instance mới
    repo = AppointmentRepository::getInstance();

    // QUAN TRỌNG: Instance mới sẽ trỏ về file mặc định (data/Appointment.txt).
    // Phải trỏ lại về file test (test/fixtures/AppointmentTest.txt).
    repo->setFilePath(TEST_DATA_FILE);

    // 4. Load dữ liệu từ file test
    repo->load();

    // 5. Kiểm tra
    EXPECT_EQ(repo->count(), 2);
    EXPECT_TRUE(repo->exists("APT1"));
    EXPECT_TRUE(repo->exists("APT2"));
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(AppointmentRepositoryTest, ClearRemovesAllAppointments)
{
    repo->add(makeAppointment("APT1", "p", "d"));
    repo->add(makeAppointment("APT2", "p", "d"));

    EXPECT_TRUE(repo->clear());
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(AppointmentRepositoryTest, EmptyRepositoryQueriesReturnEmpty)
{
    EXPECT_TRUE(repo->getAll().empty());
    EXPECT_TRUE(repo->getScheduled().empty());
    EXPECT_TRUE(repo->getCompleted().empty());
}

/* Build va run tests:
cd build && ./HospitalTests --gtest_filter="PatientRepositoryTest.*"
*/