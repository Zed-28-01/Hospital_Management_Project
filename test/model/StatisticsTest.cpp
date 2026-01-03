#include <gtest/gtest.h>
#include "model/Statistics.h"

using HMS::Model::Statistics;

// ==================== Core Statistics Tests ====================

TEST(StatisticsTest, CalculateNormalMetrics)
{
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

TEST(StatisticsTest, CalculatePendingPrescriptions)
{
    Statistics stats;

    stats.totalPrescriptions = 100;
    stats.dispensedPrescriptions = 75;

    stats.calculate();

    EXPECT_EQ(stats.pendingPrescriptions, 25);
}

TEST(StatisticsTest, CalculatePendingPrescriptions_NegativeClampedToZero)
{
    Statistics stats;

    // Edge case: dispensed > total (shouldn't happen, but handle gracefully)
    stats.totalPrescriptions = 50;
    stats.dispensedPrescriptions = 60;

    stats.calculate();

    EXPECT_EQ(stats.pendingPrescriptions, 0);
}

// ==================== Division By Zero Safety Tests ====================

TEST(StatisticsTest, DivisionByZeroSafety_CoreMetrics)
{
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

TEST(StatisticsTest, DivisionByZeroSafety_MedicineMetrics)
{
    Statistics stats;

    stats.totalMedicines = 0;
    stats.lowStockMedicines = 0;
    stats.expiredMedicines = 0;

    EXPECT_DOUBLE_EQ(stats.getLowStockRate(), 0.0);
    EXPECT_DOUBLE_EQ(stats.getExpiredRate(), 0.0);
}

TEST(StatisticsTest, DivisionByZeroSafety_PrescriptionMetrics)
{
    Statistics stats;

    stats.totalPrescriptions = 0;
    stats.dispensedPrescriptions = 0;

    EXPECT_DOUBLE_EQ(stats.getDispenseRate(), 0.0);
}

// ==================== Reset Tests ====================

TEST(StatisticsTest, ResetClearsAllData_CoreMetrics)
{
    Statistics stats;

    stats.totalPatients = 100;
    stats.totalDoctors = 50;
    stats.totalAppointments = 200;
    stats.totalRevenue = 9999.99;
    stats.paidRevenue = 8000.0;
    stats.unpaidRevenue = 1999.99;
    stats.doctorsBySpecialization["Cardiology"] = 5;

    stats.reset();

    EXPECT_EQ(stats.totalPatients, 0);
    EXPECT_EQ(stats.totalDoctors, 0);
    EXPECT_EQ(stats.totalAppointments, 0);
    EXPECT_DOUBLE_EQ(stats.totalRevenue, 0.0);
    EXPECT_DOUBLE_EQ(stats.paidRevenue, 0.0);
    EXPECT_DOUBLE_EQ(stats.unpaidRevenue, 0.0);
    EXPECT_TRUE(stats.doctorsBySpecialization.empty());
}

TEST(StatisticsTest, ResetClearsAllData_MedicineMetrics)
{
    Statistics stats;

    stats.totalMedicines = 500;
    stats.lowStockMedicines = 20;
    stats.expiredMedicines = 5;
    stats.expiringSoonMedicines = 15;
    stats.totalInventoryValue = 1000000.0;
    stats.medicinesByCategory["Antibiotics"] = 100;
    stats.inventoryValueByCategory["Antibiotics"] = 500000.0;

    stats.reset();

    EXPECT_EQ(stats.totalMedicines, 0);
    EXPECT_EQ(stats.lowStockMedicines, 0);
    EXPECT_EQ(stats.expiredMedicines, 0);
    EXPECT_EQ(stats.expiringSoonMedicines, 0);
    EXPECT_DOUBLE_EQ(stats.totalInventoryValue, 0.0);
    EXPECT_TRUE(stats.medicinesByCategory.empty());
    EXPECT_TRUE(stats.inventoryValueByCategory.empty());
}

TEST(StatisticsTest, ResetClearsAllData_DepartmentMetrics)
{
    Statistics stats;

    stats.totalDepartments = 10;
    stats.doctorsByDepartment["Cardiology"] = 8;
    stats.revenueByDepartment["Cardiology"] = 500000.0;
    stats.appointmentsByDepartment["Cardiology"] = 150;

    stats.reset();

    EXPECT_EQ(stats.totalDepartments, 0);
    EXPECT_TRUE(stats.doctorsByDepartment.empty());
    EXPECT_TRUE(stats.revenueByDepartment.empty());
    EXPECT_TRUE(stats.appointmentsByDepartment.empty());
}

TEST(StatisticsTest, ResetClearsAllData_PrescriptionMetrics)
{
    Statistics stats;

    stats.totalPrescriptions = 300;
    stats.dispensedPrescriptions = 250;
    stats.pendingPrescriptions = 50;
    stats.totalPrescriptionItems = 1200;

    stats.reset();

    EXPECT_EQ(stats.totalPrescriptions, 0);
    EXPECT_EQ(stats.dispensedPrescriptions, 0);
    EXPECT_EQ(stats.pendingPrescriptions, 0);
    EXPECT_EQ(stats.totalPrescriptionItems, 0);
}

// ==================== Rate Calculation Tests ====================

TEST(StatisticsTest, GetLowStockRate_NormalCalculation)
{
    Statistics stats;

    stats.totalMedicines = 100;
    stats.lowStockMedicines = 15;

    EXPECT_DOUBLE_EQ(stats.getLowStockRate(), 15.0);
}

TEST(StatisticsTest, GetExpiredRate_NormalCalculation)
{
    Statistics stats;

    stats.totalMedicines = 200;
    stats.expiredMedicines = 10;

    EXPECT_DOUBLE_EQ(stats.getExpiredRate(), 5.0);
}

TEST(StatisticsTest, GetDispenseRate_NormalCalculation)
{
    Statistics stats;

    stats.totalPrescriptions = 50;
    stats.dispensedPrescriptions = 40;

    EXPECT_DOUBLE_EQ(stats.getDispenseRate(), 80.0);
}

TEST(StatisticsTest, GetDispenseRate_AllDispensed)
{
    Statistics stats;

    stats.totalPrescriptions = 100;
    stats.dispensedPrescriptions = 100;

    EXPECT_DOUBLE_EQ(stats.getDispenseRate(), 100.0);
}

TEST(StatisticsTest, GetDispenseRate_NoneDispensed)
{
    Statistics stats;

    stats.totalPrescriptions = 100;
    stats.dispensedPrescriptions = 0;

    EXPECT_DOUBLE_EQ(stats.getDispenseRate(), 0.0);
}

// ==================== Report Generation Tests ====================

TEST(StatisticsTest, ReportGenerationDoesNotCrash_CoreMetrics)
{
    Statistics stats;
    stats.totalAppointments = 5;
    stats.totalRevenue = 100000.0;
    stats.calculate();

    std::string report = stats.toReport();

    EXPECT_FALSE(report.empty());

    // Check for Vietnamese text with diacritics
    EXPECT_NE(report.find("THỐNG KÊ HỆ THỐNG"), std::string::npos);
    EXPECT_NE(report.find("Tổng doanh thu"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_IncludesMedicineStats)
{
    Statistics stats;
    stats.totalMedicines = 100;
    stats.lowStockMedicines = 10;
    stats.expiredMedicines = 5;
    stats.expiringSoonMedicines = 8;
    stats.totalInventoryValue = 5000000.0;
    stats.medicinesByCategory["Antibiotics"] = 30;
    stats.medicinesByCategory["Painkillers"] = 25;

    std::string report = stats.toReport();

    EXPECT_NE(report.find("THỐNG KÊ THUỐC"), std::string::npos);
    EXPECT_NE(report.find("Tổng số thuốc"), std::string::npos);
    EXPECT_NE(report.find("Sắp hết hàng"), std::string::npos);
    EXPECT_NE(report.find("Đã hết hạn"), std::string::npos);
    EXPECT_NE(report.find("Giá trị tồn kho"), std::string::npos);
    EXPECT_NE(report.find("Theo danh mục"), std::string::npos);
    EXPECT_NE(report.find("Antibiotics"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_IncludesDepartmentStats)
{
    Statistics stats;
    stats.totalDepartments = 5;
    stats.doctorsByDepartment["Cardiology"] = 10;
    stats.doctorsByDepartment["Neurology"] = 8;
    stats.revenueByDepartment["Cardiology"] = 1000000.0;

    std::string report = stats.toReport();

    EXPECT_NE(report.find("THỐNG KÊ KHOA/PHÒNG"), std::string::npos);
    EXPECT_NE(report.find("Tổng số khoa"), std::string::npos);
    EXPECT_NE(report.find("Bác sĩ theo khoa"), std::string::npos);
    EXPECT_NE(report.find("Cardiology"), std::string::npos);
    EXPECT_NE(report.find("Doanh thu theo khoa"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_IncludesPrescriptionStats)
{
    Statistics stats;
    stats.totalPrescriptions = 200;
    stats.dispensedPrescriptions = 180;
    stats.pendingPrescriptions = 20;
    stats.totalPrescriptionItems = 800;

    std::string report = stats.toReport();

    EXPECT_NE(report.find("THỐNG KÊ ĐƠN THUỐC"), std::string::npos);
    EXPECT_NE(report.find("Tổng số đơn"), std::string::npos);
    EXPECT_NE(report.find("Đã phát thuốc"), std::string::npos);
    EXPECT_NE(report.find("Chờ phát thuốc"), std::string::npos);
    EXPECT_NE(report.find("Tổng số mục thuốc"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_SkipsMedicineStatsWhenEmpty)
{
    Statistics stats;
    stats.totalMedicines = 0;

    std::string report = stats.toReport();

    EXPECT_EQ(report.find("THỐNG KÊ THUỐC"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_SkipsDepartmentStatsWhenEmpty)
{
    Statistics stats;
    stats.totalDepartments = 0;

    std::string report = stats.toReport();

    EXPECT_EQ(report.find("THỐNG KÊ KHOA/PHÒNG"), std::string::npos);
}

TEST(StatisticsTest, ReportGeneration_SkipsPrescriptionStatsWhenEmpty)
{
    Statistics stats;
    stats.totalPrescriptions = 0;

    std::string report = stats.toReport();

    EXPECT_EQ(report.find("THỐNG KÊ ĐƠN THUỐC"), std::string::npos);
}

// ==================== Map Field Tests ====================

TEST(StatisticsTest, MedicinesByCategory_StoresMultipleCategories)
{
    Statistics stats;

    stats.medicinesByCategory["Antibiotics"] = 50;
    stats.medicinesByCategory["Painkillers"] = 30;
    stats.medicinesByCategory["Vitamins"] = 20;

    EXPECT_EQ(stats.medicinesByCategory.size(), 3);
    EXPECT_EQ(stats.medicinesByCategory["Antibiotics"], 50);
    EXPECT_EQ(stats.medicinesByCategory["Painkillers"], 30);
    EXPECT_EQ(stats.medicinesByCategory["Vitamins"], 20);
}

TEST(StatisticsTest, InventoryValueByCategory_StoresMultipleCategories)
{
    Statistics stats;

    stats.inventoryValueByCategory["Antibiotics"] = 1000000.0;
    stats.inventoryValueByCategory["Painkillers"] = 500000.0;

    EXPECT_EQ(stats.inventoryValueByCategory.size(), 2);
    EXPECT_DOUBLE_EQ(stats.inventoryValueByCategory["Antibiotics"], 1000000.0);
    EXPECT_DOUBLE_EQ(stats.inventoryValueByCategory["Painkillers"], 500000.0);
}

TEST(StatisticsTest, DoctorsByDepartment_StoresMultipleDepartments)
{
    Statistics stats;

    stats.doctorsByDepartment["Cardiology"] = 10;
    stats.doctorsByDepartment["Neurology"] = 8;
    stats.doctorsByDepartment["Orthopedics"] = 6;

    EXPECT_EQ(stats.doctorsByDepartment.size(), 3);
    EXPECT_EQ(stats.doctorsByDepartment["Cardiology"], 10);
    EXPECT_EQ(stats.doctorsByDepartment["Neurology"], 8);
    EXPECT_EQ(stats.doctorsByDepartment["Orthopedics"], 6);
}

TEST(StatisticsTest, RevenueByDepartment_StoresMultipleDepartments)
{
    Statistics stats;

    stats.revenueByDepartment["Cardiology"] = 5000000.0;
    stats.revenueByDepartment["Neurology"] = 3000000.0;

    EXPECT_EQ(stats.revenueByDepartment.size(), 2);
    EXPECT_DOUBLE_EQ(stats.revenueByDepartment["Cardiology"], 5000000.0);
    EXPECT_DOUBLE_EQ(stats.revenueByDepartment["Neurology"], 3000000.0);
}

TEST(StatisticsTest, AppointmentsByDepartment_StoresMultipleDepartments)
{
    Statistics stats;

    stats.appointmentsByDepartment["Cardiology"] = 150;
    stats.appointmentsByDepartment["Neurology"] = 100;

    EXPECT_EQ(stats.appointmentsByDepartment.size(), 2);
    EXPECT_EQ(stats.appointmentsByDepartment["Cardiology"], 150);
    EXPECT_EQ(stats.appointmentsByDepartment["Neurology"], 100);
}

// ==================== Full Statistics Integration Test ====================

TEST(StatisticsTest, FullStatistics_AllFieldsSet)
{
    Statistics stats;

    // Core counts
    stats.totalPatients = 1000;
    stats.totalDoctors = 50;
    stats.totalAppointments = 5000;

    // Appointment status
    stats.scheduledAppointments = 500;
    stats.completedAppointments = 4000;
    stats.cancelledAppointments = 300;
    stats.noShowAppointments = 200;

    // Financial
    stats.totalRevenue = 50000000.0;
    stats.paidRevenue = 45000000.0;
    stats.unpaidRevenue = 5000000.0;

    // Time-based
    stats.appointmentsToday = 50;
    stats.appointmentsThisWeek = 300;
    stats.appointmentsThisMonth = 1200;

    // Medicine
    stats.totalMedicines = 500;
    stats.lowStockMedicines = 25;
    stats.expiredMedicines = 10;
    stats.expiringSoonMedicines = 15;
    stats.totalInventoryValue = 100000000.0;
    stats.medicinesByCategory["Antibiotics"] = 100;

    // Department
    stats.totalDepartments = 10;
    stats.doctorsByDepartment["Cardiology"] = 8;
    stats.revenueByDepartment["Cardiology"] = 10000000.0;
    stats.appointmentsByDepartment["Cardiology"] = 800;

    // Prescription
    stats.totalPrescriptions = 3000;
    stats.dispensedPrescriptions = 2800;
    stats.totalPrescriptionItems = 12000;

    stats.calculate();

    // Verify calculated fields
    EXPECT_DOUBLE_EQ(stats.averageConsultationFee, 10000.0);
    EXPECT_EQ(stats.pendingPrescriptions, 200);

    // Verify rates
    EXPECT_DOUBLE_EQ(stats.getCompletionRate(), 80.0);
    EXPECT_DOUBLE_EQ(stats.getCancellationRate(), 6.0);
    EXPECT_DOUBLE_EQ(stats.getPaymentRate(), 90.0);
    EXPECT_DOUBLE_EQ(stats.getLowStockRate(), 5.0);
    EXPECT_DOUBLE_EQ(stats.getExpiredRate(), 2.0);

    // Verify report generation doesn't crash
    std::string report = stats.toReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("THỐNG KÊ HỆ THỐNG"), std::string::npos);
    EXPECT_NE(report.find("THỐNG KÊ DOANH THU"), std::string::npos);
    EXPECT_NE(report.find("THỐNG KÊ THUỐC"), std::string::npos);
    EXPECT_NE(report.find("THỐNG KÊ KHOA/PHÒNG"), std::string::npos);
    EXPECT_NE(report.find("THỐNG KÊ ĐƠN THUỐC"), std::string::npos);
}

/*
cd build && ./HospitalTests --gtest_filter="StatisticsTest.*"
*/