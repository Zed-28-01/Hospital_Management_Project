#include <gtest/gtest.h>
#include "model/Statistics.h"

namespace HMS {
namespace Model {


TEST(StatisticsTest, CalculateNormalMetrics) {
    Statistics stats;

    stats.totalAppointments = 10;
    stats.completedAppointments = 8;
    stats.cancelledAppointments = 1;

    stats.totalRevenue = 5000000.0;
    stats.paidRevenue = 4000000.0;

    stats.calculate();

    EXPECT_DOUBLE_EQ(stats.averageConsultationFee, 500000.0);

    EXPECT_DOUBLE_EQ(stats.getCompletionRate(), 80.0);
    EXPECT_DOUBLE_EQ(stats.getCancellationRate(), 10.0);
    EXPECT_DOUBLE_EQ(stats.getPaymentRate(), 80.0);
}

TEST(StatisticsTest, DivisionByZeroSafety) {
    Statistics stats;

    stats.totalAppointments = 0;
    stats.completedAppointments = 0;
    stats.cancelledAppointments = 0;
    stats.totalRevenue = 0.0;

    stats.calculate();

    EXPECT_DOUBLE_EQ(stats.averageConsultationFee, 0.0);
    EXPECT_DOUBLE_EQ(stats.getCompletionRate(), 0.0);
    EXPECT_DOUBLE_EQ(stats.getCancellationRate(), 0.0);

    stats.totalRevenue = 0.0;
    stats.paidRevenue = 0.0;

    EXPECT_DOUBLE_EQ(stats.getPaymentRate(), 0.0);
}

TEST(StatisticsTest, ResetClearsAllData) {
    Statistics stats;

    stats.totalPatients = 100;
    stats.totalRevenue = 9999.99;
    stats.doctorsBySpecialization["Cardiology"] = 5;

    stats.reset();

    EXPECT_EQ(stats.totalPatients, 0);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 0.0);

    EXPECT_TRUE(stats.doctorsBySpecialization.empty());
    EXPECT_EQ(stats.doctorsBySpecialization.size(), 0);
}

TEST(StatisticsTest, ReportGenerationDoesNotCrash) {
    Statistics stats;
    stats.totalAppointments = 5;
    stats.totalRevenue = 100000.0;
    stats.calculate();

    std::string report = stats.toReport();

    EXPECT_FALSE(report.empty());

    EXPECT_NE(report.find("THONG KE HE THONG"), std::string::npos);
    EXPECT_NE(report.find("Tong doanh thu"), std::string::npos);
}

}
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}