#include <gtest/gtest.h>

#include "bll/AdminService.h"
#include "bll/PatientService.h"
#include "bll/DoctorService.h"
#include "bll/AppointmentService.h"

using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::Model;

// ==================== Test Fixture ====================

class AdminServiceTest : public ::testing::Test {
protected:
    AdminService* adminService;

    void SetUp() override {
        // Reset singleton before each test
        AdminService::resetInstance();
        adminService = AdminService::getInstance();
    }

    void TearDown() override {
        AdminService::resetInstance();
    }
};

// ==================== Singleton Tests ====================

TEST_F(AdminServiceTest, GetInstance_ReturnsSameInstance) {
    AdminService* instance1 = AdminService::getInstance();
    AdminService* instance2 = AdminService::getInstance();

    EXPECT_EQ(instance1, instance2);
}

TEST_F(AdminServiceTest, ResetInstance_CreatesFreshInstance) {
    AdminService* instance1 = AdminService::getInstance();
    EXPECT_NE(instance1, nullptr);

    AdminService::resetInstance();

    AdminService* instance2 = AdminService::getInstance();
    EXPECT_NE(instance2, nullptr);
    EXPECT_NE(instance1, instance2);
}

// ==================== Statistics Core Tests ====================

TEST_F(AdminServiceTest, GetStatistics_DoesNotCrash) {
    Statistics stats = adminService->getStatistics();

    EXPECT_GE(stats.totalPatients, 0);
    EXPECT_GE(stats.totalDoctors, 0);
    EXPECT_GE(stats.totalAppointments, 0);
}

TEST_F(AdminServiceTest, Statistics_InitialValuesAreConsistent) {
    Statistics stats = adminService->getStatistics();

    EXPECT_GE(stats.scheduledAppointments, 0);
    EXPECT_GE(stats.completedAppointments, 0);
    EXPECT_GE(stats.cancelledAppointments, 0);
    EXPECT_GE(stats.noShowAppointments, 0);
}

TEST_F(AdminServiceTest, Statistics_RevenueIsNeverNegative) {
    Statistics stats = adminService->getStatistics();

    EXPECT_GE(stats.totalRevenue, 0.0);
    EXPECT_GE(stats.paidRevenue, 0.0);
    EXPECT_GE(stats.unpaidRevenue, 0.0);
}

// ==================== Derived Metrics Safety ====================

TEST_F(AdminServiceTest, Statistics_RatesAreSafeWhenNoAppointments) {
    Statistics stats = adminService->getStatistics();

    EXPECT_DOUBLE_EQ(stats.getCompletionRate(), stats.getCompletionRate());
    EXPECT_DOUBLE_EQ(stats.getCancellationRate(), stats.getCancellationRate());
    EXPECT_DOUBLE_EQ(stats.getPaymentRate(), stats.getPaymentRate());
}

// ==================== Revenue Consistency ====================

TEST_F(AdminServiceTest, RevenueConsistency_PaidPlusUnpaidEqualsTotal) {
    Statistics stats = adminService->getStatistics();

    EXPECT_DOUBLE_EQ(
        stats.totalRevenue,
        stats.paidRevenue + stats.unpaidRevenue
    );
}

// ==================== Getter Consistency Tests ====================

TEST_F(AdminServiceTest, TotalPatients_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalPatients(), stats.totalPatients);
}

TEST_F(AdminServiceTest, TotalDoctors_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalDoctors(), stats.totalDoctors);
}

TEST_F(AdminServiceTest, TotalAppointments_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(adminService->getTotalAppointments(), stats.totalAppointments);
}

TEST_F(AdminServiceTest, TotalRevenue_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_DOUBLE_EQ(adminService->getTotalRevenue(), stats.totalRevenue);
}

// ==================== Status-Based Getter Tests ====================

TEST_F(AdminServiceTest, ScheduledAppointmentsCount_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(
        adminService->getScheduledAppointmentsCount(),
        stats.scheduledAppointments
    );
}

TEST_F(AdminServiceTest, CompletedAppointmentsCount_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(
        adminService->getCompletedAppointmentsCount(),
        stats.completedAppointments
    );
}

TEST_F(AdminServiceTest, CancelledAppointmentsCount_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(
        adminService->getCancelledAppointmentsCount(),
        stats.cancelledAppointments
    );
}

TEST_F(AdminServiceTest, NoShowAppointmentsCount_MatchesStatistics) {
    Statistics stats = adminService->getStatistics();
    EXPECT_EQ(
        adminService->getNoShowAppointmentsCount(),
        stats.noShowAppointments
    );
}

// ==================== Specialization Maps Safety ====================

TEST_F(AdminServiceTest, DoctorsBySpecialization_MapIsValid) {
    Statistics stats = adminService->getStatistics();

    for (const auto& pair : stats.doctorsBySpecialization) {
        EXPECT_FALSE(pair.first.empty());
        EXPECT_GE(pair.second, 0);
    }
}

TEST_F(AdminServiceTest, AppointmentsBySpecialization_MapIsValid) {
    Statistics stats = adminService->getStatistics();

    for (const auto& pair : stats.appointmentsBySpecialization) {
        EXPECT_FALSE(pair.first.empty());
        EXPECT_GE(pair.second, 0);
    }
}
