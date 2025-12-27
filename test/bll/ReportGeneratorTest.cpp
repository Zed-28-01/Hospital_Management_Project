#include <filesystem>
#include <gtest/gtest.h>

#include "advance/Prescription.h"
#include "advance/ReportGenerator.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "dal/AppointmentRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/MedicineRepository.h"
#include "dal/PatientRepository.h"
#include "dal/PrescriptionRepository.h"


using namespace HMS;
using namespace HMS::BLL;
using namespace HMS::DAL;
using namespace HMS::Model;

// ==================== Test Fixture ====================

class ReportGeneratorTest : public ::testing::Test {
protected:
  ReportGenerator *reportGenerator;

  // Test file paths
  std::string testPatientFile = "test_report_patients.txt";
  std::string testDoctorFile = "test_report_doctors.txt";
  std::string testAppointmentFile = "test_report_appointments.txt";
  std::string testMedicineFile = "test_report_medicines.txt";
  std::string testPrescriptionFile = "test_report_prescriptions.txt";

  void SetUp() override {
    // Reset all singletons including ReportGenerator
    ReportGenerator::resetInstance();
    PatientRepository::resetInstance();
    DoctorRepository::resetInstance();
    AppointmentRepository::resetInstance();
    MedicineRepository::resetInstance();
    PrescriptionRepository::resetInstance();

    // Set test file paths and clear data
    PatientRepository::getInstance()->setFilePath(testPatientFile);
    PatientRepository::getInstance()->clear();

    DoctorRepository::getInstance()->setFilePath(testDoctorFile);
    DoctorRepository::getInstance()->clear();

    AppointmentRepository::getInstance()->setFilePath(testAppointmentFile);
    AppointmentRepository::getInstance()->clear();

    MedicineRepository::getInstance()->setFilePath(testMedicineFile);
    MedicineRepository::getInstance()->clear();

    PrescriptionRepository::getInstance()->setFilePath(testPrescriptionFile);
    PrescriptionRepository::getInstance()->clear();

    // Get ReportGenerator instance
    reportGenerator = ReportGenerator::getInstance();

    // Seed test data
    seedTestData();
  }

  void TearDown() override {
    // Reset all singletons including ReportGenerator
    ReportGenerator::resetInstance();
    PatientRepository::resetInstance();
    DoctorRepository::resetInstance();
    AppointmentRepository::resetInstance();
    MedicineRepository::resetInstance();
    PrescriptionRepository::resetInstance();

    // Clean up test files
    cleanupTestFile(testPatientFile);
    cleanupTestFile(testDoctorFile);
    cleanupTestFile(testAppointmentFile);
    cleanupTestFile(testMedicineFile);
    cleanupTestFile(testPrescriptionFile);
  }

  void cleanupTestFile(const std::string &filePath) {
    if (std::filesystem::exists(filePath)) {
      std::filesystem::remove(filePath);
    }
    std::string backupFile = filePath + ".bak";
    if (std::filesystem::exists(backupFile)) {
      std::filesystem::remove(backupFile);
    }
  }

  void seedTestData() {
    // Add doctors
    Doctor doc1("D001", "doctor1", "Dr. John Smith", "0123456789", Gender::MALE,
                "1980-01-15", "Cardiology", "Mon-Fri", 500000.0);
    Doctor doc2("D002", "doctor2", "Dr. Jane Doe", "0987654321", Gender::FEMALE,
                "1985-05-20", "Neurology", "Mon-Sat", 600000.0);
    DoctorRepository::getInstance()->add(doc1);
    DoctorRepository::getInstance()->add(doc2);

    // Add patients
    Patient pat1("P001", "patient1", "Alice Brown", "1111111111",
                 Gender::FEMALE, "1990-03-10", "123 Main St", "");
    Patient pat2("P002", "patient2", "Bob Wilson", "2222222222", Gender::MALE,
                 "1988-07-25", "456 Oak Ave", "");
    PatientRepository::getInstance()->add(pat1);
    PatientRepository::getInstance()->add(pat2);

    // Add appointments
    std::string today = Utils::getCurrentDate();

    // Appointment 1: Completed, Paid
    Appointment apt1("APT001", "patient1", "D001", today, "09:00",
                     "Regular checkup", 500000.0, true,
                     AppointmentStatus::COMPLETED, "");

    // Appointment 2: Completed, Unpaid
    Appointment apt2("APT002", "patient2", "D001", today, "10:00", "Follow up",
                     500000.0, false, AppointmentStatus::COMPLETED, "");

    // Appointment 3: Scheduled
    Appointment apt3("APT003", "patient1", "D002", today, "14:00",
                     "Consultation", 600000.0, false,
                     AppointmentStatus::SCHEDULED, "");

    // Appointment 4: Cancelled
    Appointment apt4("APT004", "patient2", "D002", today, "15:00", "Check",
                     600000.0, false, AppointmentStatus::CANCELLED, "");

    // Appointment 5: No Show
    Appointment apt5("APT005", "patient1", "D001", today, "16:00", "Missed",
                     500000.0, false, AppointmentStatus::NO_SHOW, "");

    AppointmentRepository::getInstance()->add(apt1);
    AppointmentRepository::getInstance()->add(apt2);
    AppointmentRepository::getInstance()->add(apt3);
    AppointmentRepository::getInstance()->add(apt4);
    AppointmentRepository::getInstance()->add(apt5);

    // Add medicines
    Medicine med1("MED001", "Paracetamol", "Pain Relief", 50000.0, 100);
    med1.setReorderLevel(10);
    Medicine med2("MED002", "Amoxicillin", "Antibiotic", 75000.0, 50);
    med2.setReorderLevel(10);
    MedicineRepository::getInstance()->add(med1);
    MedicineRepository::getInstance()->add(med2);

    // Add prescriptions (for pharmacy revenue testing)
    Prescription presc1("PRE001", "APT001", "patient1", "D001", today);
    presc1.setDiagnosis("Mild fever");
    PrescriptionItem item1{"MED001", "Paracetamol", 10, "2 tablets", "5 days",
                           "After meals"};
    PrescriptionItem item2{"MED002", "Amoxicillin", 5, "1 tablet", "5 days",
                           "Before meals"};
    presc1.addItem(item1);
    presc1.addItem(item2);
    presc1.setDispensed(true); // Dispensed prescription
    PrescriptionRepository::getInstance()->add(presc1);

    Prescription presc2("PRE002", "APT002", "patient2", "D001", today);
    presc2.setDiagnosis("Headache");
    PrescriptionItem item3{"MED001", "Paracetamol", 5, "1 tablet", "3 days",
                           "As needed"};
    presc2.addItem(item3);
    presc2.setDispensed(false); // Not dispensed yet
    PrescriptionRepository::getInstance()->add(presc2);
  }
};

// ==================== Singleton Tests ====================

TEST_F(ReportGeneratorTest, GetInstance_ReturnsSameInstance) {
  ReportGenerator *instance1 = ReportGenerator::getInstance();
  ReportGenerator *instance2 = ReportGenerator::getInstance();

  EXPECT_EQ(instance1, instance2);
}

// ==================== Daily Report Tests ====================

TEST_F(ReportGeneratorTest, GenerateDailyReport_ValidDate_ReturnsReport) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateDailyReport(today);

  EXPECT_EQ(report.type, ReportType::DAILY_SUMMARY);
  EXPECT_FALSE(report.reportID.empty());
  EXPECT_EQ(report.startDate, today);
  EXPECT_EQ(report.endDate, today);
  EXPECT_FALSE(report.content.empty());
  EXPECT_FALSE(report.title.empty());
}

TEST_F(ReportGeneratorTest,
       GenerateDailyReport_InvalidDate_ReturnsErrorReport) {
  Report report = reportGenerator->generateDailyReport("invalid-date");

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GenerateDailyReport_ContainsCorrectStatistics) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateDailyReport(today);

  // Should have 5 total appointments (including no-show)
  EXPECT_EQ(report.statistics.totalAppointments, 5);
  EXPECT_EQ(report.statistics.completedAppointments, 2);
  EXPECT_EQ(report.statistics.scheduledAppointments, 1);
  EXPECT_EQ(report.statistics.cancelledAppointments, 1);
  EXPECT_EQ(report.statistics.noShowAppointments, 1);
}

TEST_F(ReportGeneratorTest, GenerateDailyReport_CalculatesRevenueCorrectly) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateDailyReport(today);

  // 2 completed appointments at 500000 each = 1000000 total
  EXPECT_DOUBLE_EQ(report.statistics.totalRevenue, 1000000.0);
  EXPECT_DOUBLE_EQ(report.statistics.paidRevenue, 500000.0);
  EXPECT_DOUBLE_EQ(report.statistics.unpaidRevenue, 500000.0);
}

// ==================== Weekly Report Tests ====================

TEST_F(ReportGeneratorTest, GenerateWeeklyReport_ValidDate_ReturnsReport) {
  std::string startDate = Utils::getCurrentDate();

  Report report = reportGenerator->generateWeeklyReport(startDate);

  EXPECT_EQ(report.type, ReportType::WEEKLY_SUMMARY);
  EXPECT_FALSE(report.reportID.empty());
  EXPECT_EQ(report.startDate, startDate);
  EXPECT_FALSE(report.endDate.empty());
  EXPECT_FALSE(report.content.empty());
}

TEST_F(ReportGeneratorTest,
       GenerateWeeklyReport_InvalidDate_ReturnsErrorReport) {
  Report report = reportGenerator->generateWeeklyReport("not-a-date");

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GenerateWeeklyReport_EndDateIs6DaysAfterStart) {
  std::string startDate = "2024-03-01"; // Friday

  Report report = reportGenerator->generateWeeklyReport(startDate);

  EXPECT_EQ(report.startDate, "2024-03-01");
  EXPECT_EQ(report.endDate, "2024-03-07");
}

// ==================== Monthly Report Tests ====================

TEST_F(ReportGeneratorTest,
       GenerateMonthlyReport_ValidMonthYear_ReturnsReport) {
  Report report = reportGenerator->generateMonthlyReport(3, 2024);

  EXPECT_EQ(report.type, ReportType::MONTHLY_SUMMARY);
  EXPECT_FALSE(report.reportID.empty());
  EXPECT_EQ(report.startDate, "2024-03-01");
  EXPECT_EQ(report.endDate, "2024-03-31");
  EXPECT_TRUE(report.title.find("March") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateMonthlyReport_InvalidMonth_ReturnsErrorReport) {
  Report report = reportGenerator->generateMonthlyReport(13, 2024);

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateMonthlyReport_InvalidYear_ReturnsErrorReport) {
  Report report = reportGenerator->generateMonthlyReport(6, 1800);

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateMonthlyReport_FebruaryLeapYear_CorrectEndDate) {
  Report report =
      reportGenerator->generateMonthlyReport(2, 2024); // 2024 is leap year

  EXPECT_EQ(report.endDate, "2024-02-29");
}

TEST_F(ReportGeneratorTest,
       GenerateMonthlyReport_FebruaryNonLeapYear_CorrectEndDate) {
  Report report = reportGenerator->generateMonthlyReport(2, 2023);

  EXPECT_EQ(report.endDate, "2023-02-28");
}

// ==================== Revenue Report Tests ====================

TEST_F(ReportGeneratorTest, GenerateRevenueReport_ValidDates_ReturnsReport) {
  std::string startDate = "2024-01-01";
  std::string endDate = "2024-12-31";

  Report report = reportGenerator->generateRevenueReport(startDate, endDate);

  EXPECT_EQ(report.type, ReportType::REVENUE_REPORT);
  EXPECT_FALSE(report.reportID.empty());
  EXPECT_EQ(report.startDate, startDate);
  EXPECT_EQ(report.endDate, endDate);
  EXPECT_TRUE(report.title.find("REVENUE") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateRevenueReport_InvalidDates_ReturnsErrorReport) {
  Report report = reportGenerator->generateRevenueReport("bad", "dates");

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GenerateRevenueReport_ContainsRevenueBreakdown) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateRevenueReport(today, today);

  EXPECT_TRUE(report.content.find("Paid") != std::string::npos);
  EXPECT_TRUE(report.content.find("Unpaid") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateRevenueReport_CalculatesPharmacyRevenueCorrectly) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateRevenueReport(today, today);

  // Pharmacy revenue from dispensed prescription (PRE001):
  // - MED001 (Paracetamol): 10 * 50000 = 500000
  // - MED002 (Amoxicillin): 5 * 75000 = 375000
  // Total pharmacy revenue = 875000
  EXPECT_TRUE(report.content.find("Pharmacy") != std::string::npos);
  EXPECT_TRUE(report.content.find("875,000") != std::string::npos ||
              report.content.find("875000") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GenerateRevenueReport_ContainsTopEarningDoctors) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateRevenueReport(today, today);

  // Should contain top earning doctors section
  EXPECT_TRUE(report.content.find("TOP EARNING") != std::string::npos ||
              report.content.find("Dr. John Smith") != std::string::npos);
}

// ==================== Patient Report Tests ====================

TEST_F(ReportGeneratorTest, GeneratePatientReport_ReturnsReport) {
  Report report = reportGenerator->generatePatientReport();

  EXPECT_EQ(report.type, ReportType::PATIENT_STATISTICS);
  EXPECT_FALSE(report.reportID.empty());
  EXPECT_TRUE(report.title.find("PATIENT") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GeneratePatientReport_ContainsPatientCount) {
  Report report = reportGenerator->generatePatientReport();

  // We added 2 patients
  EXPECT_EQ(report.statistics.totalPatients, 2);
}

// ==================== Doctor Performance Report Tests ====================

TEST_F(ReportGeneratorTest,
       GenerateDoctorPerformanceReport_AllDoctors_ReturnsReport) {
  std::string today = Utils::getCurrentDate();

  Report report =
      reportGenerator->generateDoctorPerformanceReport("", today, today);

  EXPECT_EQ(report.type, ReportType::DOCTOR_PERFORMANCE);
  EXPECT_FALSE(report.content.empty());
  EXPECT_TRUE(report.content.find("Dr. John Smith") != std::string::npos ||
              report.content.find("D001") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateDoctorPerformanceReport_SpecificDoctor_ReturnsReport) {
  std::string today = Utils::getCurrentDate();

  Report report =
      reportGenerator->generateDoctorPerformanceReport("D001", today, today);

  EXPECT_EQ(report.type, ReportType::DOCTOR_PERFORMANCE);
  // Should only contain D001's data
  EXPECT_TRUE(report.content.find("Dr. John Smith") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateDoctorPerformanceReport_NonExistentDoctor_ReturnsError) {
  std::string today = Utils::getCurrentDate();

  Report report =
      reportGenerator->generateDoctorPerformanceReport("D999", today, today);

  EXPECT_TRUE(report.content.find("Error") != std::string::npos ||
              report.content.find("not found") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateDoctorPerformanceReport_InvalidDates_ReturnsError) {
  Report report =
      reportGenerator->generateDoctorPerformanceReport("D001", "bad", "dates");

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

// ==================== Appointment Analysis Tests ====================

TEST_F(ReportGeneratorTest,
       GenerateAppointmentAnalysis_ValidDates_ReturnsReport) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateAppointmentAnalysis(today, today);

  EXPECT_EQ(report.type, ReportType::APPOINTMENT_ANALYSIS);
  EXPECT_FALSE(report.content.empty());
  EXPECT_TRUE(report.content.find("APPOINTMENT") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateAppointmentAnalysis_InvalidDates_ReturnsError) {
  Report report =
      reportGenerator->generateAppointmentAnalysis("invalid", "dates");

  EXPECT_TRUE(report.content.find("Error") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateAppointmentAnalysis_ContainsStatusBreakdown) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateAppointmentAnalysis(today, today);

  EXPECT_TRUE(report.content.find("Completed") != std::string::npos);
  EXPECT_TRUE(report.content.find("Cancelled") != std::string::npos);
  EXPECT_TRUE(report.content.find("Scheduled") != std::string::npos);
}

TEST_F(ReportGeneratorTest, GenerateAppointmentAnalysis_ContainsNoShowRate) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateAppointmentAnalysis(today, today);

  // Should contain no-show statistics
  EXPECT_TRUE(report.content.find("No Show") != std::string::npos);
  EXPECT_EQ(report.statistics.noShowAppointments, 1);

  // No-show rate should be 20% (1 out of 5 appointments)
  EXPECT_TRUE(report.content.find("No Show Rate") != std::string::npos);
}

TEST_F(ReportGeneratorTest,
       GenerateAppointmentAnalysis_CalculatesCompletionRateCorrectly) {
  std::string today = Utils::getCurrentDate();

  Report report = reportGenerator->generateAppointmentAnalysis(today, today);

  // Completion rate: 2 completed / 5 total = 40%
  double expectedRate = (2.0 / 5.0) * 100.0;
  EXPECT_NEAR(report.statistics.getCompletionRate(), expectedRate, 0.1);
}

// ==================== Custom Report Tests ====================

TEST_F(ReportGeneratorTest, GenerateCustomReport_DelegatesDailyReport) {
  std::string today = Utils::getCurrentDate();
  std::vector<std::pair<std::string, std::string>> filters;

  Report report = reportGenerator->generateCustomReport(
      ReportType::DAILY_SUMMARY, today, today, filters);

  EXPECT_EQ(report.type, ReportType::DAILY_SUMMARY);
}

TEST_F(ReportGeneratorTest, GenerateCustomReport_DelegatesRevenueReport) {
  std::string today = Utils::getCurrentDate();
  std::vector<std::pair<std::string, std::string>> filters;

  Report report = reportGenerator->generateCustomReport(
      ReportType::REVENUE_REPORT, today, today, filters);

  EXPECT_EQ(report.type, ReportType::REVENUE_REPORT);
}

TEST_F(ReportGeneratorTest, GenerateCustomReport_DoctorPerformanceWithFilter) {
  std::string today = Utils::getCurrentDate();
  std::vector<std::pair<std::string, std::string>> filters = {
      {"doctorID", "D001"}};

  Report report = reportGenerator->generateCustomReport(
      ReportType::DOCTOR_PERFORMANCE, today, today, filters);

  EXPECT_EQ(report.type, ReportType::DOCTOR_PERFORMANCE);
}

// ==================== Export Tests ====================

TEST_F(ReportGeneratorTest, ExportToText_ReturnsNonEmptyString) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);

  std::string textExport = reportGenerator->exportToText(report);

  EXPECT_FALSE(textExport.empty());
  EXPECT_TRUE(textExport.find("Report ID") != std::string::npos);
  EXPECT_TRUE(textExport.find(report.reportID) != std::string::npos);
}

TEST_F(ReportGeneratorTest, ExportToCSV_ReturnsValidCSV) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);

  std::string csvExport = reportGenerator->exportToCSV(report);

  EXPECT_FALSE(csvExport.empty());
  EXPECT_TRUE(csvExport.find(",") != std::string::npos); // Contains commas
  EXPECT_TRUE(csvExport.find("Metric,Value") != std::string::npos); // Header
}

TEST_F(ReportGeneratorTest, ExportToHTML_ReturnsValidHTML) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);

  std::string htmlExport = reportGenerator->exportToHTML(report);

  EXPECT_FALSE(htmlExport.empty());
  EXPECT_TRUE(htmlExport.find("<!DOCTYPE html>") != std::string::npos);
  EXPECT_TRUE(htmlExport.find("<html>") != std::string::npos);
  EXPECT_TRUE(htmlExport.find("</html>") != std::string::npos);
  EXPECT_TRUE(htmlExport.find("<table>") != std::string::npos);
}

TEST_F(ReportGeneratorTest, ExportReport_TextFormat_CreatesFile) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);
  std::string outputPath = "test_export_output.txt";

  bool result =
      reportGenerator->exportReport(report, ExportFormat::TEXT, outputPath);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // Cleanup
  std::filesystem::remove(outputPath);
}

TEST_F(ReportGeneratorTest, ExportReport_CSVFormat_CreatesFile) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);
  std::string outputPath = "test_export_output.csv";

  bool result =
      reportGenerator->exportReport(report, ExportFormat::CSV, outputPath);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // Cleanup
  std::filesystem::remove(outputPath);
}

TEST_F(ReportGeneratorTest, ExportReport_HTMLFormat_CreatesFile) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);
  std::string outputPath = "test_export_output.html";

  bool result =
      reportGenerator->exportReport(report, ExportFormat::HTML, outputPath);

  EXPECT_TRUE(result);
  EXPECT_TRUE(std::filesystem::exists(outputPath));

  // Cleanup
  std::filesystem::remove(outputPath);
}

TEST_F(ReportGeneratorTest, ExportReport_PDFFormat_ReturnsFalse) {
  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);

  bool result =
      reportGenerator->exportReport(report, ExportFormat::PDF, "test.pdf");

  // PDF not implemented
  EXPECT_FALSE(result);
}

// ==================== Edge Case Tests ====================

TEST_F(ReportGeneratorTest,
       GenerateDailyReport_NoAppointments_HandlesGracefully) {
  // Clear all appointments
  AppointmentRepository::getInstance()->clear();

  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateDailyReport(today);

  EXPECT_EQ(report.statistics.totalAppointments, 0);
  EXPECT_DOUBLE_EQ(report.statistics.totalRevenue, 0.0);
  // Should not crash or have divide-by-zero errors
  EXPECT_FALSE(report.content.empty());
}

TEST_F(ReportGeneratorTest,
       GenerateRevenueReport_NoCompletedAppointments_HandlesGracefully) {
  // Clear and add only scheduled appointments
  AppointmentRepository::getInstance()->clear();
  Appointment apt("APT100", "patient1", "D001", Utils::getCurrentDate(),
                  "09:00", "Test", 500000.0, false,
                  AppointmentStatus::SCHEDULED, "");
  AppointmentRepository::getInstance()->add(apt);

  std::string today = Utils::getCurrentDate();
  Report report = reportGenerator->generateRevenueReport(today, today);

  EXPECT_DOUBLE_EQ(report.statistics.totalRevenue, 0.0);
  // Collection rate should be 0%, not NaN or crash
  EXPECT_FALSE(report.content.empty());
}

TEST_F(ReportGeneratorTest,
       GenerateDoctorPerformanceReport_NoDoctors_HandlesGracefully) {
  DoctorRepository::getInstance()->clear();

  std::string today = Utils::getCurrentDate();
  Report report =
      reportGenerator->generateDoctorPerformanceReport("", today, today);

  // Should not crash
  EXPECT_FALSE(report.content.empty());
}

// ==================== Report ID Uniqueness Tests ====================

TEST_F(ReportGeneratorTest, ReportIDs_AreUnique) {
  std::string today = Utils::getCurrentDate();

  Report report1 = reportGenerator->generateDailyReport(today);
  Report report2 = reportGenerator->generateDailyReport(today);
  Report report3 = reportGenerator->generateWeeklyReport(today);

  EXPECT_NE(report1.reportID, report2.reportID);
  EXPECT_NE(report1.reportID, report3.reportID);
  EXPECT_NE(report2.reportID, report3.reportID);
}

TEST_F(ReportGeneratorTest, ReportIDs_HaveCorrectPrefix) {
  std::string today = Utils::getCurrentDate();

  Report daily = reportGenerator->generateDailyReport(today);
  Report weekly = reportGenerator->generateWeeklyReport(today);
  Report monthly = reportGenerator->generateMonthlyReport(3, 2024);
  Report revenue = reportGenerator->generateRevenueReport(today, today);
  Report patient = reportGenerator->generatePatientReport();
  Report doctor =
      reportGenerator->generateDoctorPerformanceReport("", today, today);
  Report analysis = reportGenerator->generateAppointmentAnalysis(today, today);

  EXPECT_TRUE(daily.reportID.find("RPT-D") == 0);
  EXPECT_TRUE(weekly.reportID.find("RPT-W") == 0);
  EXPECT_TRUE(monthly.reportID.find("RPT-M") == 0);
  EXPECT_TRUE(revenue.reportID.find("RPT-R") == 0);
  EXPECT_TRUE(patient.reportID.find("RPT-P") == 0);
  EXPECT_TRUE(doctor.reportID.find("RPT-DP") == 0);
  EXPECT_TRUE(analysis.reportID.find("RPT-A") == 0);
}

/*
Build and run tests:
cd build && ./HospitalTests --gtest_filter="ReportGeneratorTest.*"
*/
