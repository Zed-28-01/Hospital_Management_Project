#include <gtest/gtest.h>

#include "dal/AppointmentRepository.h"
#include "model/Appointment.h"
#include "common/Types.h"
#include "common/Utils.h"

#include <filesystem>
#include <fstream>

using namespace HMS;
using namespace HMS::DAL;
using namespace HMS::Model;

namespace {

const std::string TEST_DATA_FILE = "test/fixtures/AppointmentTest.txt";

// Helper tạo appointment hợp lệ
Appointment makeAppointment(
    const std::string& id,
    const std::string& patient,
    const std::string& doctor,
    const std::string& date = "2030-01-01",
    const std::string& time = "09:00",
    AppointmentStatus status = AppointmentStatus::SCHEDULED,
    bool paid = false
) {
    return Appointment(
        id,
        patient,
        doctor,
        date,
        time,
        "Flu",
        100.0,
        paid,
        status,
        "Test note"
    );
}

} // anonymous namespace

// ============================================================================
// Test Fixture
// ============================================================================
class AppointmentRepositoryTest : public ::testing::Test {
protected:
    AppointmentRepository* repo;

    void SetUp() override {
        // Reset singleton
        AppointmentRepository::resetInstance();
        repo = AppointmentRepository::getInstance();

        // Redirect file path
        repo->setFilePath(TEST_DATA_FILE);

        // Clean file before each test
        std::filesystem::create_directories("test/fixtures");
        std::ofstream ofs(TEST_DATA_FILE, std::ios::trunc);
        ofs.close();

        repo->clear();
    }

    void TearDown() override {
        repo->clear();
    }
};

// ============================================================================
// BASIC CRUD
// ============================================================================

TEST_F(AppointmentRepositoryTest, AddAppointmentSuccess) {
    auto apt = makeAppointment("APT001", "patient1", "D001");
    EXPECT_TRUE(repo->add(apt));
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(AppointmentRepositoryTest, AddDuplicateAppointmentFails) {
    auto apt = makeAppointment("APT001", "patient1", "D001");
    EXPECT_TRUE(repo->add(apt));
    EXPECT_FALSE(repo->add(apt));
    EXPECT_EQ(repo->count(), 1);
}

TEST_F(AppointmentRepositoryTest, GetByIdFound) {
    auto apt = makeAppointment("APT001", "patient1", "D001");
    repo->add(apt);

    auto result = repo->getById("APT001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->getPatientUsername(), "patient1");
}

TEST_F(AppointmentRepositoryTest, GetByIdNotFound) {
    auto result = repo->getById("NOT_EXIST");
    EXPECT_FALSE(result.has_value());
}

TEST_F(AppointmentRepositoryTest, UpdateAppointmentSuccess) {
    auto apt = makeAppointment("APT001", "patient1", "D001");
    repo->add(apt);

    auto updated = makeAppointment(
        "APT001", "patient1", "D001",
        "2030-01-02", "10:00",
        AppointmentStatus::COMPLETED, true
    );

    EXPECT_TRUE(repo->update(updated));

    auto fetched = repo->getById("APT001");
    ASSERT_TRUE(fetched);
    EXPECT_TRUE(fetched->isPaid());
    EXPECT_EQ(fetched->getStatus(), AppointmentStatus::COMPLETED);
}

TEST_F(AppointmentRepositoryTest, UpdateNonExistingFails) {
    auto apt = makeAppointment("APT999", "p", "d");
    EXPECT_FALSE(repo->update(apt));
}

TEST_F(AppointmentRepositoryTest, RemoveAppointmentSuccess) {
    repo->add(makeAppointment("APT001", "p1", "D1"));
    EXPECT_TRUE(repo->remove("APT001"));
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(AppointmentRepositoryTest, RemoveNonExistingFails) {
    EXPECT_FALSE(repo->remove("APT404"));
}

// ============================================================================
// PATIENT QUERIES
// ============================================================================

TEST_F(AppointmentRepositoryTest, GetByPatient) {
    repo->add(makeAppointment("APT1", "alice", "D1"));
    repo->add(makeAppointment("APT2", "bob", "D1"));
    repo->add(makeAppointment("APT3", "alice", "D2"));

    auto result = repo->getByPatient("alice");
    EXPECT_EQ(result.size(), 2);
}

TEST_F(AppointmentRepositoryTest, GetUnpaidByPatient) {
    repo->add(makeAppointment("APT1", "alice", "D1", "2030-01-01", "09:00", AppointmentStatus::SCHEDULED, false));
    repo->add(makeAppointment("APT2", "alice", "D1", "2030-01-01", "10:00", AppointmentStatus::SCHEDULED, true));

    auto unpaid = repo->getUnpaidByPatient("alice");
    EXPECT_EQ(unpaid.size(), 1);
}

// ============================================================================
// DOCTOR QUERIES
// ============================================================================

TEST_F(AppointmentRepositoryTest, GetByDoctorAndDateSorted) {
    repo->add(makeAppointment("APT1", "p1", "D1", "2030-01-01", "10:00"));
    repo->add(makeAppointment("APT2", "p2", "D1", "2030-01-01", "09:00"));

    auto result = repo->getByDoctorAndDate("D1", "2030-01-01");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].getTime(), "09:00");
    EXPECT_EQ(result[1].getTime(), "10:00");
}

// ============================================================================
// SLOT AVAILABILITY
// ============================================================================

TEST_F(AppointmentRepositoryTest, SlotAvailableWhenEmpty) {
    EXPECT_TRUE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

TEST_F(AppointmentRepositoryTest, SlotUnavailableWhenScheduled) {
    repo->add(makeAppointment("APT1", "p1", "D1", "2030-01-01", "09:00"));
    EXPECT_FALSE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

TEST_F(AppointmentRepositoryTest, SlotAvailableWhenCancelled) {
    repo->add(makeAppointment(
        "APT1", "p1", "D1",
        "2030-01-01", "09:00",
        AppointmentStatus::CANCELLED
    ));

    EXPECT_TRUE(repo->isSlotAvailable("D1", "2030-01-01", "09:00"));
}

// ============================================================================
// FILE PERSISTENCE
// ============================================================================

TEST_F(AppointmentRepositoryTest, SaveAndLoadPersistence) {
    repo->add(makeAppointment("APT1", "alice", "D1"));
    repo->add(makeAppointment("APT2", "bob", "D2"));

    AppointmentRepository::resetInstance();
    repo = AppointmentRepository::getInstance();
    repo->setFilePath(TEST_DATA_FILE);
    repo->load();

    EXPECT_EQ(repo->count(), 2);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(AppointmentRepositoryTest, ClearRemovesAllAppointments) {
    repo->add(makeAppointment("APT1", "p", "d"));
    repo->add(makeAppointment("APT2", "p", "d"));

    EXPECT_TRUE(repo->clear());
    EXPECT_EQ(repo->count(), 0);
}

TEST_F(AppointmentRepositoryTest, EmptyRepositoryQueriesReturnEmpty) {
    EXPECT_TRUE(repo->getAll().empty());
    EXPECT_TRUE(repo->getScheduled().empty());
    EXPECT_TRUE(repo->getCompleted().empty());
}
