#include <gtest/gtest.h>
#include <filesystem>

#include "bll/AdminService.h"
#include "bll/PatientService.h"
#include "bll/DoctorService.h"
#include "bll/AppointmentService.h"
#include "dal/PatientRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/AppointmentRepository.h"
#include "common/Utils.h"

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Test Fixture ====================

class AdminServiceTest : public ::testing::Test
{
protected:
    AdminService *adminService;

    // Test file paths to avoid touching production data
    std::string testPatientFile = "test_admin_patients.txt";
    std::string testDoctorFile = "test_admin_doctors.txt";
    std::string testAppointmentFile = "test_admin_appointments.txt";

    void SetUp() override
    {
        // Reset all singletons to get fresh instances
        AdminService::resetInstance();
        PatientService::resetInstance();
        DoctorService::resetInstance();
        AppointmentService::resetInstance();
        PatientRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();

        // Redirect repositories to test files
        PatientRepository::getInstance()->setFilePath(testPatientFile);
        PatientRepository::getInstance()->clear();

        DoctorRepository::getInstance()->setFilePath(testDoctorFile);
        DoctorRepository::getInstance()->clear();

        AppointmentRepository::getInstance()->setFilePath(testAppointmentFile);
        AppointmentRepository::getInstance()->clear();

        // Get AdminService instance (which uses the redirected repositories)
        adminService = AdminService::getInstance();
    }

    void TearDown() override
    {
        // Reset all singletons
        AdminService::resetInstance();
        PatientService::resetInstance();
        DoctorService::resetInstance();
        AppointmentService::resetInstance();
        PatientRepository::resetInstance();
        DoctorRepository::resetInstance();
        AppointmentRepository::resetInstance();

        // Clean up test files
        cleanupTestFile(testPatientFile);
        cleanupTestFile(testDoctorFile);
        cleanupTestFile(testAppointmentFile);
    }

    void cleanupTestFile(const std::string &filePath)
    {
        if (std::filesystem::exists(filePath))
        {
            std::filesystem::remove(filePath);
        }
        std::string tempFile = filePath + ".tmp";
        if (std::filesystem::exists(tempFile))
        {
            std::filesystem::remove(tempFile);
        }
    }

    // Helper to create test doctor
    Doctor createTestDoctor(
        const std::string &doctorID = "D001",
        const std::string &username = "dr_test",
        const std::string &name = "Dr. Test",
        const std::string &specialization = "General Medicine",
        double consultationFee = 500000.0)
    {
        return Doctor(doctorID, username, name, "0123456789",
                      Gender::MALE, "1980-01-01", specialization,
                      consultationFee);
    }

    // Helper to create test patient
    Patient createTestPatient(
        const std::string &patientID = "P001",
        const std::string &username = "patient_test",
        const std::string &name = "Test Patient")
    {
        return Patient(patientID, username, name, "0987654321",
                       Gender::MALE, "1990-01-01", "123 Test St", "None");
    }

    // Helper to create test appointment
    Appointment createTestAppointment(
        const std::string &appointmentID = "APT001",
        const std::string &patientUsername = "patient_test",
        const std::string &doctorID = "D001",
        AppointmentStatus status = AppointmentStatus::SCHEDULED,
        double price = 500000.0,
        bool paid = false)
    {
        // Full constructor: id, patient, doctor, date, time, disease, price, isPaid, status, notes
        Appointment appt(appointmentID, patientUsername, doctorID,
                         "2024-12-25", "10:00", "General checkup",
                         price, paid, status, "");
        return appt;
    }
};

// ==================== Singleton Tests ====================

TEST_F(AdminServiceTest, GetInstance_ReturnsSameInstance)
{
    AdminService *instance1 = AdminService::getInstance();
    AdminService *instance2 = AdminService::getInstance();

    EXPECT_EQ(instance1, instance2);
}

TEST_F(AdminServiceTest, ResetInstance_CreatesFreshInstance)
{
    // Verify that reset doesn't leave the singleton in a bad state
    AdminService *instance1 = AdminService::getInstance();
    EXPECT_NE(instance1, nullptr);

    AdminService::resetInstance();

    // After reset, getInstance should return a valid instance
    AdminService *instance2 = AdminService::getInstance();
    EXPECT_NE(instance2, nullptr);

    // The new instance should be functional
    EXPECT_TRUE(instance2->checkSystemHealth());
}

// ==================== Statistics Core Tests ====================

TEST_F(AdminServiceTest, GetStatistics_EmptySystem_ReturnsZeros)
{
    Statistics stats = adminService->getStatistics();

    EXPECT_EQ(stats.totalPatients, 0);
    EXPECT_EQ(stats.totalDoctors, 0);
    EXPECT_EQ(stats.totalAppointments, 0);
}

TEST_F(AdminServiceTest, GetStatistics_WithData_ReturnsCorrectCounts)
{
    // Add test data
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    DoctorRepository::getInstance()->add(createTestDoctor("D002", "doc2"));
    PatientRepository::getInstance()->add(createTestPatient("P001", "pat1"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT001"));

    Statistics stats = adminService->getStatistics();

    EXPECT_EQ(stats.totalPatients, 1);
    EXPECT_EQ(stats.totalDoctors, 2);
    EXPECT_EQ(stats.totalAppointments, 1);
}

TEST_F(AdminServiceTest, Statistics_InitialValuesAreConsistent)
{
    Statistics stats = adminService->getStatistics();

    EXPECT_GE(stats.scheduledAppointments, 0);
    EXPECT_GE(stats.completedAppointments, 0);
    EXPECT_GE(stats.cancelledAppointments, 0);
    EXPECT_GE(stats.noShowAppointments, 0);
}

TEST_F(AdminServiceTest, Statistics_RevenueIsNeverNegative)
{
    Statistics stats = adminService->getStatistics();

    EXPECT_GE(stats.totalRevenue, 0.0);
    EXPECT_GE(stats.paidRevenue, 0.0);
    EXPECT_GE(stats.unpaidRevenue, 0.0);
}

// ==================== Derived Metrics Safety ====================

TEST_F(AdminServiceTest, Statistics_RatesAreSafeWhenNoAppointments)
{
    Statistics stats = adminService->getStatistics();

    // Should not crash or return NaN
    EXPECT_DOUBLE_EQ(stats.getCompletionRate(), stats.getCompletionRate());
    EXPECT_DOUBLE_EQ(stats.getCancellationRate(), stats.getCancellationRate());
    EXPECT_DOUBLE_EQ(stats.getPaymentRate(), stats.getPaymentRate());
}

// ==================== Revenue Consistency ====================

TEST_F(AdminServiceTest, RevenueConsistency_PaidPlusUnpaidEqualsTotal)
{
    // Add completed appointments with different payment statuses
    Appointment paidAppt = createTestAppointment("APT001", "pat1", "D001",
                                                 AppointmentStatus::COMPLETED, 500000.0, true);
    Appointment unpaidAppt = createTestAppointment("APT002", "pat1", "D001",
                                                   AppointmentStatus::COMPLETED, 300000.0, false);

    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    AppointmentRepository::getInstance()->add(paidAppt);
    AppointmentRepository::getInstance()->add(unpaidAppt);

    Statistics stats = adminService->getStatistics();

    EXPECT_DOUBLE_EQ(stats.totalRevenue, stats.paidRevenue + stats.unpaidRevenue);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 800000.0);
    EXPECT_DOUBLE_EQ(stats.paidRevenue, 500000.0);
    EXPECT_DOUBLE_EQ(stats.unpaidRevenue, 300000.0);
}

// ==================== Getter Consistency Tests ====================

TEST_F(AdminServiceTest, TotalPatients_MatchesStatistics)
{
    PatientRepository::getInstance()->add(createTestPatient("P001", "pat1"));
    PatientRepository::getInstance()->add(createTestPatient("P002", "pat2"));

    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalPatients(), stats.totalPatients);
    EXPECT_EQ(adminService->getTotalPatients(), 2);
}

TEST_F(AdminServiceTest, TotalDoctors_MatchesStatistics)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));

    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalDoctors(), stats.totalDoctors);
    EXPECT_EQ(adminService->getTotalDoctors(), 1);
}

TEST_F(AdminServiceTest, TotalAppointments_MatchesStatistics)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT001"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT002", "pat1", "D001"));

    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalAppointments(), stats.totalAppointments);
    EXPECT_EQ(adminService->getTotalAppointments(), 2);
}

TEST_F(AdminServiceTest, TotalRevenue_MatchesStatistics)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT001", "pat1", "D001", AppointmentStatus::COMPLETED, 500000.0));

    Statistics stats = adminService->getStatistics();
    EXPECT_DOUBLE_EQ(adminService->getTotalRevenue(), stats.totalRevenue);
}

// ==================== Status-Based Getter Tests ====================

TEST_F(AdminServiceTest, StatusCounts_MatchStatistics)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));

    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT001", "pat1", "D001", AppointmentStatus::SCHEDULED));
    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT002", "pat1", "D001", AppointmentStatus::SCHEDULED));
    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT003", "pat1", "D001", AppointmentStatus::COMPLETED));
    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT004", "pat1", "D001", AppointmentStatus::CANCELLED));
    AppointmentRepository::getInstance()->add(
        createTestAppointment("APT005", "pat1", "D001", AppointmentStatus::NO_SHOW));

    Statistics stats = adminService->getStatistics();

    EXPECT_EQ(adminService->getScheduledAppointmentsCount(), stats.scheduledAppointments);
    EXPECT_EQ(adminService->getScheduledAppointmentsCount(), 2);

    EXPECT_EQ(adminService->getCompletedAppointmentsCount(), stats.completedAppointments);
    EXPECT_EQ(adminService->getCompletedAppointmentsCount(), 1);

    EXPECT_EQ(adminService->getCancelledAppointmentsCount(), stats.cancelledAppointments);
    EXPECT_EQ(adminService->getCancelledAppointmentsCount(), 1);

    EXPECT_EQ(adminService->getNoShowAppointmentsCount(), stats.noShowAppointments);
    EXPECT_EQ(adminService->getNoShowAppointmentsCount(), 1);
}

// ==================== Specialization Maps Safety ====================

TEST_F(AdminServiceTest, DoctorsBySpecialization_MapIsValid)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1", "Dr. One", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D002", "doc2", "Dr. Two", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D003", "doc3", "Dr. Three", "Neurology"));

    Statistics stats = adminService->getStatistics();

    EXPECT_EQ(stats.doctorsBySpecialization.size(), 2u);
    EXPECT_EQ(stats.doctorsBySpecialization["Cardiology"], 2);
    EXPECT_EQ(stats.doctorsBySpecialization["Neurology"], 1);
}

TEST_F(AdminServiceTest, AppointmentsBySpecialization_MapIsValid)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1", "Dr. Heart", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D002", "doc2", "Dr. Brain", "Neurology"));

    AppointmentRepository::getInstance()->add(createTestAppointment("APT001", "pat1", "D001"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT002", "pat1", "D001"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT003", "pat1", "D002"));

    Statistics stats = adminService->getStatistics();

    EXPECT_EQ(stats.appointmentsBySpecialization.size(), 2u);
    EXPECT_EQ(stats.appointmentsBySpecialization["Cardiology"], 2);
    EXPECT_EQ(stats.appointmentsBySpecialization["Neurology"], 1);
}

// ==================== Doctor Statistics Tests ====================

TEST_F(AdminServiceTest, GetDoctorCountBySpecialization_ReturnsCorrectCounts)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1", "Dr. One", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D002", "doc2", "Dr. Two", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D003", "doc3", "Dr. Three", "Neurology"));

    auto counts = adminService->getDoctorCountBySpecialization();

    EXPECT_EQ(counts["Cardiology"], 2);
    EXPECT_EQ(counts["Neurology"], 1);
}

TEST_F(AdminServiceTest, GetAppointmentCountBySpecialization_ReturnsCorrectCounts)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1", "Dr. Heart", "Cardiology"));
    DoctorRepository::getInstance()->add(createTestDoctor("D002", "doc2", "Dr. Brain", "Neurology"));

    AppointmentRepository::getInstance()->add(createTestAppointment("APT001", "pat1", "D001"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT002", "pat1", "D001"));
    AppointmentRepository::getInstance()->add(createTestAppointment("APT003", "pat1", "D002"));

    auto counts = adminService->getAppointmentCountBySpecialization();

    EXPECT_EQ(counts["Cardiology"], 2);
    EXPECT_EQ(counts["Neurology"], 1);
}

// ==================== Report Generation Tests ====================

TEST_F(AdminServiceTest, GenerateSummaryReport_DoesNotCrash)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    PatientRepository::getInstance()->add(createTestPatient("P001", "pat1"));

    std::string report = adminService->generateSummaryReport();

    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("THỐNG KÊ"), std::string::npos);
}

TEST_F(AdminServiceTest, GenerateDailyReport_DoesNotCrash)
{
    std::string report = adminService->generateDailyReport("2024-12-25");

    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("BAO CAO NGAY"), std::string::npos);
}

TEST_F(AdminServiceTest, GenerateMonthlyReport_DoesNotCrash)
{
    std::string report = adminService->generateMonthlyReport(12, 2024);

    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("BAO CAO"), std::string::npos);
}

// ==================== System Health Tests ====================

TEST_F(AdminServiceTest, CheckSystemHealth_ReturnsTrue)
{
    EXPECT_TRUE(adminService->checkSystemHealth());
}

TEST_F(AdminServiceTest, GetSystemStatus_ReturnsValidReport)
{
    std::string status = adminService->getSystemStatus();

    EXPECT_FALSE(status.empty());
    EXPECT_NE(status.find("TRANG THAI HE THONG"), std::string::npos);
    EXPECT_NE(status.find("OK"), std::string::npos);
}

// ==================== Data Management Tests ====================

TEST_F(AdminServiceTest, SaveAllData_DoesNotCrash)
{
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));
    PatientRepository::getInstance()->add(createTestPatient("P001", "pat1"));

    // Should not throw
    bool result = adminService->saveAllData();
    EXPECT_TRUE(result);
}

TEST_F(AdminServiceTest, LoadAllData_DoesNotCrash)
{
    // Should not throw
    bool result = adminService->loadAllData();
    EXPECT_TRUE(result);
}

// ==================== Validation Tests ====================

TEST_F(AdminServiceTest, GenerateMonthlyReport_InvalidMonth_ReturnsError)
{
    std::string report = adminService->generateMonthlyReport(0, 2024);
    EXPECT_NE(report.find("Error"), std::string::npos);

    report = adminService->generateMonthlyReport(13, 2024);
    EXPECT_NE(report.find("Error"), std::string::npos);
}

TEST_F(AdminServiceTest, GenerateMonthlyReport_InvalidYear_ReturnsError)
{
    std::string report = adminService->generateMonthlyReport(6, 1800);
    EXPECT_NE(report.find("Error"), std::string::npos);
}

TEST_F(AdminServiceTest, GenerateDailyReport_InvalidDate_ReturnsError)
{
    std::string report = adminService->generateDailyReport("invalid-date");
    EXPECT_NE(report.find("Error"), std::string::npos);

    report = adminService->generateDailyReport("2024-13-01");
    EXPECT_NE(report.find("Error"), std::string::npos);
}

TEST_F(AdminServiceTest, GenerateWeeklyReport_InvalidDate_ReturnsError)
{
    std::string report = adminService->generateWeeklyReport("not-a-date");
    EXPECT_NE(report.find("Error"), std::string::npos);
}

// ==================== Week Range Tests ====================

TEST_F(AdminServiceTest, GetAppointmentsThisWeek_ReturnsValidResult)
{
    // Should not crash and return empty or valid list
    auto appointments = adminService->getAppointmentsThisWeek();
    EXPECT_GE(appointments.size(), 0u);
}

TEST_F(AdminServiceTest, Statistics_AppointmentsThisWeek_IsPopulated)
{
    // Add appointment for today (which is within this week)
    DoctorRepository::getInstance()->add(createTestDoctor("D001", "doc1"));

    // Create appointment with today's date
    std::string today = HMS::Utils::getCurrentDate();
    Appointment todayAppt("APT001", "pat1", "D001", today, "10:00",
                          "Checkup", 500000.0, false, AppointmentStatus::SCHEDULED, "");
    AppointmentRepository::getInstance()->add(todayAppt);

    Statistics stats = adminService->getStatistics();

    // Today's appointment should be counted in this week
    EXPECT_GE(stats.appointmentsThisWeek, 1);
}

/*
Build and run tests:
cd build && ./HospitalTests --gtest_filter="AdminServiceTest.*"
*/
