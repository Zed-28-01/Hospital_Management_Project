#include "advance/ReportGenerator.h"
#include "common/Constants.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "dal/AppointmentRepository.h"
#include "dal/DoctorRepository.h"
#include "dal/MedicineRepository.h"
#include "dal/PatientRepository.h"
#include "dal/PrescriptionRepository.h"

#include <algorithm>
#include <atomic>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace HMS
{
  namespace BLL
  {

    // ==================== Static Members ====================
    std::unique_ptr<ReportGenerator> ReportGenerator::s_instance = nullptr;
    std::mutex ReportGenerator::s_mutex;

    // ==================== Singleton Access ====================
    ReportGenerator *ReportGenerator::getInstance()
    {
      std::lock_guard<std::mutex> lock(s_mutex);
      if (!s_instance)
      {
        s_instance = std::unique_ptr<ReportGenerator>(new ReportGenerator());
      }
      return s_instance.get();
    }

    void ReportGenerator::resetInstance()
    {
      std::lock_guard<std::mutex> lock(s_mutex);
      s_instance.reset();
    }

    // ==================== Private Helpers ====================

    namespace
    {
      // Helper to get repositories (avoid repeated getInstance calls)
      struct Repositories
      {
        DAL::PatientRepository *patients;
        DAL::DoctorRepository *doctors;
        DAL::AppointmentRepository *appointments;
        DAL::MedicineRepository *medicines;
        DAL::PrescriptionRepository *prescriptions;

        Repositories()
            : patients(DAL::PatientRepository::getInstance()),
              doctors(DAL::DoctorRepository::getInstance()),
              appointments(DAL::AppointmentRepository::getInstance()),
              medicines(DAL::MedicineRepository::getInstance()),
              prescriptions(DAL::PrescriptionRepository::getInstance()) {}
      };

      // Helper to generate unique report ID
      std::string generateReportID(ReportType type)
      {
        static std::atomic<int> counter{0};
        std::string prefix;
        switch (type)
        {
        case ReportType::DAILY_SUMMARY:
          prefix = "RPT-D";
          break;
        case ReportType::WEEKLY_SUMMARY:
          prefix = "RPT-W";
          break;
        case ReportType::MONTHLY_SUMMARY:
          prefix = "RPT-M";
          break;
        case ReportType::REVENUE_REPORT:
          prefix = "RPT-R";
          break;
        case ReportType::PATIENT_STATISTICS:
          prefix = "RPT-P";
          break;
        case ReportType::DOCTOR_PERFORMANCE:
          prefix = "RPT-DP";
          break;
        case ReportType::APPOINTMENT_ANALYSIS:
          prefix = "RPT-A";
          break;
        default:
          prefix = "RPT-C";
          break;
        }

        std::ostringstream oss;
        oss << prefix << std::setfill('0') << std::setw(4) << (++counter);
        return oss.str();
      }

      // Helper to calculate end of week (6 days after start)
      std::string calculateWeekEnd(const std::string &startDate)
      {
        if (!Utils::isValidDateInternal(startDate))
        {
          return startDate;
        }

        int year = std::stoi(startDate.substr(0, 4));
        int month = std::stoi(startDate.substr(5, 2));
        int day = std::stoi(startDate.substr(8, 2));

        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day + 6;
        std::mktime(&tm);

        char buffer[11];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm);
        return std::string(buffer);
      }

      // Helper to calculate month date range
      std::pair<std::string, std::string> getMonthRange(int month, int year)
      {
        std::ostringstream startOss;
        startOss << year << "-" << std::setfill('0') << std::setw(2) << month
                 << "-01";

        int daysInMonth = Utils::getDaysInMonth(month, year);

        std::ostringstream endOss;
        endOss << year << "-" << std::setfill('0') << std::setw(2) << month << "-"
               << std::setfill('0') << std::setw(2) << daysInMonth;

        return {startOss.str(), endOss.str()};
      }

      // Safe division helper
      double safeDivide(double numerator, double denominator)
      {
        if (denominator == 0.0)
        {
          return 0.0;
        }
        return numerator / denominator;
      }

      // Helper to aggregate appointment statistics
      Model::Statistics
      aggregateAppointmentStats(const std::vector<Model::Appointment> &appointments)
      {
        Model::Statistics stats;
        stats.reset();

        stats.totalAppointments = static_cast<int>(appointments.size());

        for (const auto &appt : appointments)
        {
          switch (appt.getStatus())
          {
          case AppointmentStatus::SCHEDULED:
            stats.scheduledAppointments++;
            break;
          case AppointmentStatus::COMPLETED:
            stats.completedAppointments++;
            stats.totalRevenue += appt.getPrice();
            if (appt.isPaid())
            {
              stats.paidRevenue += appt.getPrice();
            }
            else
            {
              stats.unpaidRevenue += appt.getPrice();
            }
            break;
          case AppointmentStatus::CANCELLED:
            stats.cancelledAppointments++;
            break;
          case AppointmentStatus::NO_SHOW:
            stats.noShowAppointments++;
            break;
          default:
            break;
          }
        }

        stats.calculate();
        return stats;
      }

      // Helper to format report section header
      std::string formatSectionHeader(const std::string &title)
      {
        std::ostringstream oss;
        oss << "\n"
            << title << "\n";
        oss << std::string(title.length(), '-') << "\n";
        return oss.str();
      }

      // Helper to format a stat line
      std::string formatStatLine(const std::string &label, int value,
                                 int width = 25)
      {
        std::ostringstream oss;
        oss << "  " << Utils::padString(label + ":", width) << value << "\n";
        return oss.str();
      }

      std::string formatStatLine(const std::string &label, double value,
                                 int width = 25)
      {
        std::ostringstream oss;
        oss << "  " << Utils::padString(label + ":", width)
            << Utils::formatMoney(value) << "\n";
        return oss.str();
      }

      std::string formatStatLine(const std::string &label, const std::string &value,
                                 int width = 25)
      {
        std::ostringstream oss;
        oss << "  " << Utils::padString(label + ":", width) << value << "\n";
        return oss.str();
      }

    } // anonymous namespace

    // ==================== Report Generation ====================

    Report ReportGenerator::generateDailyReport(const std::string &date)
    {
      Report report;
      report.type = ReportType::DAILY_SUMMARY;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.startDate = date;
      report.endDate = date;

      // Validate date
      if (!Utils::isValidDateInternal(date))
      {
        report.title = "Daily Report - Invalid Date";
        report.content = "Error: Invalid date format. Expected YYYY-MM-DD.";
        return report;
      }

      report.title = "DAILY REPORT - " + Utils::formatDateDisplay(date);

      Repositories repos;
      auto appointments = repos.appointments->getByDate(date);

      // Aggregate statistics
      report.statistics = aggregateAppointmentStats(appointments);

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("OVERVIEW");
      content << formatStatLine("Total Appointments",
                                report.statistics.totalAppointments);
      content << formatStatLine("Scheduled",
                                report.statistics.scheduledAppointments);
      content << formatStatLine("Completed",
                                report.statistics.completedAppointments);
      content << formatStatLine("Cancelled",
                                report.statistics.cancelledAppointments);
      content << formatStatLine("No Show", report.statistics.noShowAppointments);

      content << formatSectionHeader("REVENUE");
      content << formatStatLine("Total Revenue", report.statistics.totalRevenue);
      content << formatStatLine("Paid", report.statistics.paidRevenue);
      content << formatStatLine("Unpaid", report.statistics.unpaidRevenue);

      report.content = content.str();
      return report;
    }

    Report ReportGenerator::generateWeeklyReport(const std::string &startDate)
    {
      Report report;
      report.type = ReportType::WEEKLY_SUMMARY;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.startDate = startDate;

      // Validate date
      if (!Utils::isValidDateInternal(startDate))
      {
        report.title = "Weekly Report - Invalid Date";
        report.content = "Error: Invalid date format. Expected YYYY-MM-DD.";
        return report;
      }

      report.endDate = calculateWeekEnd(startDate);
      report.title = "WEEKLY REPORT";

      Repositories repos;
      auto appointments =
          repos.appointments->getByDateRange(startDate, report.endDate);

      // Aggregate statistics
      report.statistics = aggregateAppointmentStats(appointments);

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << "From " << Utils::formatDateDisplay(startDate) << " to "
              << Utils::formatDateDisplay(report.endDate) << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("OVERVIEW");
      content << formatStatLine("Total Appointments",
                                report.statistics.totalAppointments);
      content << formatStatLine("Scheduled",
                                report.statistics.scheduledAppointments);
      content << formatStatLine("Completed",
                                report.statistics.completedAppointments);
      content << formatStatLine("Cancelled",
                                report.statistics.cancelledAppointments);
      content << formatStatLine("No Show", report.statistics.noShowAppointments);

      content << formatStatLine(
          "Completion Rate",
          std::to_string(static_cast<int>(report.statistics.getCompletionRate())) +
              "%");

      content << formatSectionHeader("REVENUE");
      content << formatStatLine("Total Revenue", report.statistics.totalRevenue);
      content << formatStatLine("Paid", report.statistics.paidRevenue);
      content << formatStatLine("Unpaid", report.statistics.unpaidRevenue);

      content << formatStatLine(
          "Payment Rate",
          std::to_string(static_cast<int>(report.statistics.getPaymentRate())) +
              "%");

      report.content = content.str();
      return report;
    }

    Report ReportGenerator::generateMonthlyReport(int month, int year)
    {
      Report report;
      report.type = ReportType::MONTHLY_SUMMARY;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();

      // Validate month and year
      if (month < 1 || month > 12)
      {
        report.title = "Monthly Report - Invalid Month";
        report.content = "Error: Invalid month. Expected 1-12.";
        return report;
      }

      if (year < 1900 || year > 2100)
      {
        report.title = "Monthly Report - Invalid Year";
        report.content = "Error: Invalid year.";
        return report;
      }

      auto [startDate, endDate] = getMonthRange(month, year);
      report.startDate = startDate;
      report.endDate = endDate;

      static const char *monthNames[] = {
          "January", "February", "March", "April", "May", "June",
          "July", "August", "September", "October", "November", "December"};

      report.title = "MONTHLY REPORT - " + std::string(monthNames[month - 1]) +
                     " " + std::to_string(year);

      Repositories repos;
      auto appointments = repos.appointments->getByDateRange(startDate, endDate);
      auto allDoctors = repos.doctors->getAll();

      // Aggregate statistics
      report.statistics = aggregateAppointmentStats(appointments);

      // Build doctor lookup map to avoid N+1
      std::unordered_map<std::string, std::string> doctorSpecMap;
      for (const auto &doc : allDoctors)
      {
        doctorSpecMap[doc.getID()] = doc.getSpecialization();
        report.statistics.doctorsBySpecialization[doc.getSpecialization()]++;
      }

      // Count appointments by specialization
      for (const auto &appt : appointments)
      {
        auto it = doctorSpecMap.find(appt.getDoctorID());
        if (it != doctorSpecMap.end())
        {
          report.statistics.appointmentsBySpecialization[it->second]++;
        }
      }

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("OVERVIEW");
      content << formatStatLine("Total Appointments",
                                report.statistics.totalAppointments);
      content << formatStatLine("Scheduled",
                                report.statistics.scheduledAppointments);
      content << formatStatLine("Completed",
                                report.statistics.completedAppointments);
      content << formatStatLine("Cancelled",
                                report.statistics.cancelledAppointments);
      content << formatStatLine("No Show", report.statistics.noShowAppointments);

      content << formatStatLine(
          "Completion Rate",
          std::to_string(static_cast<int>(report.statistics.getCompletionRate())) +
              "%");

      content << formatSectionHeader("REVENUE");
      content << formatStatLine("Total Revenue", report.statistics.totalRevenue);
      content << formatStatLine("Paid", report.statistics.paidRevenue);
      content << formatStatLine("Unpaid", report.statistics.unpaidRevenue);

      content << formatStatLine(
          "Payment Rate",
          std::to_string(static_cast<int>(report.statistics.getPaymentRate())) +
              "%");

      // Appointments by Specialization
      if (!report.statistics.appointmentsBySpecialization.empty())
      {
        content << formatSectionHeader("BY SPECIALIZATION");
        for (const auto &[spec, count] :
             report.statistics.appointmentsBySpecialization)
        {
          content << formatStatLine(spec, count);
        }
      }

      report.content = content.str();
      return report;
    }

    Report ReportGenerator::generateRevenueReport(const std::string &startDate,
                                                  const std::string &endDate)
    {
      Report report;
      report.type = ReportType::REVENUE_REPORT;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.startDate = startDate;
      report.endDate = endDate;

      // Validate dates
      if (!Utils::isValidDateInternal(startDate) || !Utils::isValidDateInternal(endDate))
      {
        report.title = "Revenue Report - Invalid Dates";
        report.content = "Error: Invalid date format. Expected YYYY-MM-DD.";
        return report;
      }

      report.title = "REVENUE REPORT";

      Repositories repos;
      auto appointments = repos.appointments->getByDateRange(startDate, endDate);
      auto allDoctors = repos.doctors->getAll();
      auto prescriptions = repos.prescriptions->getByDateRange(startDate, endDate);
      auto dispensedPrescriptions = repos.prescriptions->getDispensed();

      // Build doctor lookup
      std::unordered_map<std::string, std::string> doctorNameMap;
      std::unordered_map<std::string, double> doctorRevenueMap;
      for (const auto &doc : allDoctors)
      {
        doctorNameMap[doc.getID()] = doc.getName();
        doctorRevenueMap[doc.getID()] = 0.0;
      }

      // Calculate revenue per doctor
      double totalRevenue = 0.0;
      double paidRevenue = 0.0;
      int completedCount = 0;

      for (const auto &appt : appointments)
      {
        if (appt.getStatus() == AppointmentStatus::COMPLETED)
        {
          completedCount++;
          totalRevenue += appt.getPrice();
          doctorRevenueMap[appt.getDoctorID()] += appt.getPrice();

          if (appt.isPaid())
          {
            paidRevenue += appt.getPrice();
          }
        }
      }

      // Calculate pharmacy revenue from dispensed prescriptions in date range
      // Pre-load all medicines into a lookup map to avoid N+1 queries
      auto allMedicines = repos.medicines->getAll();
      std::unordered_map<std::string, double> medicinePriceMap;
      for (const auto &med : allMedicines)
      {
        medicinePriceMap[med.getMedicineID()] = med.getUnitPrice();
      }

      double pharmacyRevenue = 0.0;
      for (const auto &presc : dispensedPrescriptions)
      {
        // Check if prescription is in date range
        const std::string &prescDate = presc.getPrescriptionDate();
        if (Utils::compareDates(prescDate, startDate) >= 0 &&
            Utils::compareDates(prescDate, endDate) <= 0)
        {
          // Sum up medicine costs using pre-loaded price map
          for (const auto &item : presc.getItems())
          {
            auto it = medicinePriceMap.find(item.medicineID);
            if (it != medicinePriceMap.end())
            {
              pharmacyRevenue += it->second * item.quantity;
            }
          }
        }
      }

      report.statistics.totalRevenue = totalRevenue;
      report.statistics.paidRevenue = paidRevenue;
      report.statistics.unpaidRevenue = totalRevenue - paidRevenue;

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << "From " << Utils::formatDateDisplay(startDate) << " to "
              << Utils::formatDateDisplay(endDate) << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("CONSULTATION REVENUE");
      content << formatStatLine("Completed Appointments", completedCount);
      content << formatStatLine("Total Revenue", totalRevenue);
      content << formatStatLine("Paid Revenue", paidRevenue);
      content << formatStatLine("Unpaid Revenue", totalRevenue - paidRevenue);

      double collectionRate = safeDivide(paidRevenue, totalRevenue) * 100.0;
      content << formatStatLine("Collection Rate",
                                std::to_string(static_cast<int>(collectionRate)) +
                                    "%");

      content << formatSectionHeader("PHARMACY REVENUE");
      content << formatStatLine("Dispensed Prescriptions",
                                static_cast<int>(dispensedPrescriptions.size()));
      content << formatStatLine("Pharmacy Revenue", pharmacyRevenue);

      content << formatSectionHeader("TOTAL");
      content << formatStatLine("Combined Revenue", totalRevenue + pharmacyRevenue);

      // Top Earning Doctors
      std::vector<std::pair<std::string, double>> sortedDoctors(
          doctorRevenueMap.begin(), doctorRevenueMap.end());
      std::sort(sortedDoctors.begin(), sortedDoctors.end(),
                [](const auto &a, const auto &b)
                { return a.second > b.second; });

      content << formatSectionHeader("TOP EARNING DOCTORS");
      int rank = 1;
      for (const auto &[doctorID, revenue] : sortedDoctors)
      {
        if (revenue > 0 && rank <= 5)
        {
          std::string doctorName =
              doctorNameMap.count(doctorID) ? doctorNameMap[doctorID] : doctorID;
          content << formatStatLine(std::to_string(rank) + ". " + doctorName,
                                    revenue);
          rank++;
        }
      }

      report.content = content.str();
      return report;
    }

    Report ReportGenerator::generatePatientReport()
    {
      Report report;
      report.type = ReportType::PATIENT_STATISTICS;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.title = "PATIENT STATISTICS REPORT";

      Repositories repos;
      auto patients = repos.patients->getAll();
      auto appointments = repos.appointments->getAll();

      // Count unique patients with appointments
      std::set<std::string> patientsWithAppointments;
      std::unordered_map<std::string, int> appointmentsPerPatient;

      for (const auto &appt : appointments)
      {
        patientsWithAppointments.insert(appt.getPatientUsername());
        appointmentsPerPatient[appt.getPatientUsername()]++;
      }

      // Calculate statistics
      report.statistics.totalPatients = static_cast<int>(patients.size());
      int activePatients = static_cast<int>(patientsWithAppointments.size());

      double avgAppointmentsPerPatient =
          safeDivide(static_cast<double>(appointments.size()),
                     static_cast<double>(activePatients));

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("PATIENT OVERVIEW");
      content << formatStatLine("Total Registered",
                                report.statistics.totalPatients);
      content << formatStatLine("Active (with appointments)", activePatients);

      double activeRate =
          safeDivide(static_cast<double>(activePatients),
                     static_cast<double>(report.statistics.totalPatients)) *
          100.0;
      content << formatStatLine("Activity Rate",
                                std::to_string(static_cast<int>(activeRate)) + "%");

      content << formatSectionHeader("APPOINTMENT STATISTICS");
      content << formatStatLine("Total Appointments",
                                static_cast<int>(appointments.size()));
      std::ostringstream avgOss;
      avgOss << std::fixed << std::setprecision(1) << avgAppointmentsPerPatient;
      content << formatStatLine("Avg per Active Patient", avgOss.str());

      report.content = content.str();
      return report;
    }

    Report
    ReportGenerator::generateDoctorPerformanceReport(const std::string &doctorID,
                                                     const std::string &startDate,
                                                     const std::string &endDate)
    {
      Report report;
      report.type = ReportType::DOCTOR_PERFORMANCE;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.startDate = startDate;
      report.endDate = endDate;

      // Validate dates
      if (!Utils::isValidDateInternal(startDate) || !Utils::isValidDateInternal(endDate))
      {
        report.title = "Doctor Performance Report - Invalid Dates";
        report.content = "Error: Invalid date format. Expected YYYY-MM-DD.";
        return report;
      }

      report.title = "DOCTOR PERFORMANCE REPORT";

      Repositories repos;
      auto allDoctors = repos.doctors->getAll();
      auto appointments = repos.appointments->getByDateRange(startDate, endDate);

      // Filter by doctor if specified
      std::vector<Model::Doctor> targetDoctors;
      if (doctorID.empty())
      {
        targetDoctors = allDoctors;
      }
      else
      {
        auto doc = repos.doctors->getById(doctorID);
        if (doc)
        {
          targetDoctors.push_back(*doc);
        }
        else
        {
          report.title = "Doctor Performance Report - Doctor Not Found";
          report.content = "Error: Doctor with ID " + doctorID + " not found.";
          return report;
        }
      }

      // Aggregate per-doctor statistics
      struct DoctorStats
      {
        int total = 0;
        int completed = 0;
        int cancelled = 0;
        int noShow = 0;
        double revenue = 0.0;
        std::set<std::string> uniquePatients;
      };

      std::map<std::string, DoctorStats> doctorStatsMap;

      for (const auto &doc : targetDoctors)
      {
        doctorStatsMap[doc.getID()] = DoctorStats{};
      }

      for (const auto &appt : appointments)
      {
        if (doctorStatsMap.count(appt.getDoctorID()) == 0)
        {
          continue;
        }

        auto &stats = doctorStatsMap[appt.getDoctorID()];
        stats.total++;

        switch (appt.getStatus())
        {
        case AppointmentStatus::COMPLETED:
          stats.completed++;
          stats.revenue += appt.getPrice();
          stats.uniquePatients.insert(appt.getPatientUsername());
          break;
        case AppointmentStatus::CANCELLED:
          stats.cancelled++;
          break;
        case AppointmentStatus::NO_SHOW:
          stats.noShow++;
          break;
        default:
          break;
        }
      }

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << "From " << Utils::formatDateDisplay(startDate) << " to "
              << Utils::formatDateDisplay(endDate) << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      for (const auto &doc : targetDoctors)
      {
        const auto &stats = doctorStatsMap[doc.getID()];

        content << formatSectionHeader(doc.getName() + " (" +
                                       doc.getSpecialization() + ")");
        content << formatStatLine("Total Appointments", stats.total);
        content << formatStatLine("Completed", stats.completed);
        content << formatStatLine("Cancelled", stats.cancelled);
        content << formatStatLine("No Show", stats.noShow);

        double completionRate = safeDivide(static_cast<double>(stats.completed),
                                           static_cast<double>(stats.total)) *
                                100.0;
        content << formatStatLine("Completion Rate",
                                  std::to_string(static_cast<int>(completionRate)) +
                                      "%");

        content << formatStatLine("Total Revenue", stats.revenue);
        content << formatStatLine("Unique Patients",
                                  static_cast<int>(stats.uniquePatients.size()));
      }

      report.content = content.str();
      return report;
    }

    Report
    ReportGenerator::generateAppointmentAnalysis(const std::string &startDate,
                                                 const std::string &endDate)
    {
      Report report;
      report.type = ReportType::APPOINTMENT_ANALYSIS;
      report.reportID = generateReportID(report.type);
      report.generatedDate = Utils::getCurrentDate();
      report.startDate = startDate;
      report.endDate = endDate;

      // Validate dates
      if (!Utils::isValidDateInternal(startDate) || !Utils::isValidDateInternal(endDate))
      {
        report.title = "Appointment Analysis - Invalid Dates";
        report.content = "Error: Invalid date format. Expected YYYY-MM-DD.";
        return report;
      }

      report.title = "APPOINTMENT ANALYSIS REPORT";

      Repositories repos;
      auto appointments = repos.appointments->getByDateRange(startDate, endDate);

      // Aggregate statistics
      report.statistics = aggregateAppointmentStats(appointments);

      // Group by date
      std::map<std::string, int> appointmentsByDate;
      for (const auto &appt : appointments)
      {
        appointmentsByDate[appt.getDate()]++;
      }

      // Find busiest day
      std::string busiestDay = "N/A";
      int maxCount = 0;
      for (const auto &[date, count] : appointmentsByDate)
      {
        if (count > maxCount)
        {
          maxCount = count;
          busiestDay = date;
        }
      }

      // Build content
      std::ostringstream content;
      content << std::string(50, '=') << "\n";
      content << report.title << "\n";
      content << "From " << Utils::formatDateDisplay(startDate) << " to "
              << Utils::formatDateDisplay(endDate) << "\n";
      content << std::string(50, '=') << "\n";
      content << "Generated: " << Utils::getCurrentDateTime() << "\n";

      content << formatSectionHeader("OVERVIEW");
      content << formatStatLine("Total Appointments",
                                report.statistics.totalAppointments);
      content << formatStatLine("Days with Appointments",
                                static_cast<int>(appointmentsByDate.size()));

      double avgPerDay =
          safeDivide(static_cast<double>(report.statistics.totalAppointments),
                     static_cast<double>(appointmentsByDate.size()));
      std::ostringstream avgOss;
      avgOss << std::fixed << std::setprecision(1) << avgPerDay;
      content << formatStatLine("Avg per Day", avgOss.str());
      content << formatStatLine("Busiest Day",
                                busiestDay + " (" + std::to_string(maxCount) + ")");

      content << formatSectionHeader("STATUS BREAKDOWN");
      content << formatStatLine("Scheduled",
                                report.statistics.scheduledAppointments);
      content << formatStatLine("Completed",
                                report.statistics.completedAppointments);
      content << formatStatLine("Cancelled",
                                report.statistics.cancelledAppointments);
      content << formatStatLine("No Show", report.statistics.noShowAppointments);

      // No-show rate calculated manually (not available in Statistics)
      double noShowRate =
          safeDivide(static_cast<double>(report.statistics.noShowAppointments),
                     static_cast<double>(report.statistics.totalAppointments)) *
          100.0;

      content << formatStatLine(
          "Completion Rate",
          std::to_string(static_cast<int>(report.statistics.getCompletionRate())) +
              "%");
      content << formatStatLine("No Show Rate",
                                std::to_string(static_cast<int>(noShowRate)) + "%");

      report.content = content.str();
      return report;
    }

    Report ReportGenerator::generateCustomReport(
        ReportType type, const std::string &startDate, const std::string &endDate,
        const std::vector<std::pair<std::string, std::string>> &filters)
    {
      // Delegate to appropriate report type
      switch (type)
      {
      case ReportType::DAILY_SUMMARY:
        return generateDailyReport(startDate);
      case ReportType::WEEKLY_SUMMARY:
        return generateWeeklyReport(startDate);
      case ReportType::REVENUE_REPORT:
        return generateRevenueReport(startDate, endDate);
      case ReportType::PATIENT_STATISTICS:
        return generatePatientReport();
      case ReportType::APPOINTMENT_ANALYSIS:
        return generateAppointmentAnalysis(startDate, endDate);
      case ReportType::DOCTOR_PERFORMANCE:
      {
        // Check for doctorID filter
        std::string doctorID;
        for (const auto &[key, value] : filters)
        {
          if (key == "doctorID")
          {
            doctorID = value;
            break;
          }
        }
        return generateDoctorPerformanceReport(doctorID, startDate, endDate);
      }
      default:
      {
        Report report;
        report.type = type;
        report.reportID = generateReportID(type);
        report.generatedDate = Utils::getCurrentDate();
        report.title = "Custom Report";
        report.content =
            "Custom report with " + std::to_string(filters.size()) + " filters.";
        return report;
      }
      }
    }

    // ==================== Export ====================

    bool ReportGenerator::exportReport(const Report &report, ExportFormat format,
                                       const std::string &filePath)
    {
      std::string content;

      switch (format)
      {
      case ExportFormat::TEXT:
        content = exportToText(report);
        break;
      case ExportFormat::CSV:
        content = exportToCSV(report);
        break;
      case ExportFormat::HTML:
        content = exportToHTML(report);
        break;
      case ExportFormat::PDF:
        // PDF export not implemented - would require external library
        return false;
      }

      // Write to file
      std::ofstream outFile(filePath);
      if (!outFile)
      {
        return false;
      }

      outFile << content;
      outFile.close();

      return true;
    }

    std::string ReportGenerator::exportToText(const Report &report)
    {
      std::ostringstream oss;
      oss << "Report ID: " << report.reportID << "\n";
      oss << "Generated: " << report.generatedDate << "\n";
      oss << "Period: " << report.startDate << " to " << report.endDate << "\n\n";
      oss << report.content;
      return oss.str();
    }

    std::string ReportGenerator::exportToCSV(const Report &report)
    {
      std::ostringstream oss;
      oss << "Report ID,Title,Generated Date,Start Date,End Date\n";
      oss << report.reportID << "," << report.title << "," << report.generatedDate
          << "," << report.startDate << "," << report.endDate << "\n\n";

      // Export statistics as CSV
      oss << "Metric,Value\n";
      oss << "Total Appointments," << report.statistics.totalAppointments << "\n";
      oss << "Completed," << report.statistics.completedAppointments << "\n";
      oss << "Cancelled," << report.statistics.cancelledAppointments << "\n";
      oss << "No Show," << report.statistics.noShowAppointments << "\n";
      oss << "Total Revenue," << std::fixed << std::setprecision(2)
          << report.statistics.totalRevenue << "\n";
      oss << "Paid Revenue," << report.statistics.paidRevenue << "\n";
      oss << "Unpaid Revenue," << report.statistics.unpaidRevenue << "\n";

      return oss.str();
    }

    std::string ReportGenerator::exportToHTML(const Report &report)
    {
      std::ostringstream oss;
      oss << "<!DOCTYPE html>\n";
      oss << "<html>\n<head>\n";
      oss << "<title>" << report.title << "</title>\n";
      oss << "<style>\n";
      oss << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
      oss << "h1 { color: #2c3e50; }\n";
      oss << "table { border-collapse: collapse; width: 100%; }\n";
      oss << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
      oss << "th { background-color: #3498db; color: white; }\n";
      oss << ".meta { color: #7f8c8d; font-size: 0.9em; }\n";
      oss << "</style>\n";
      oss << "</head>\n<body>\n";

      oss << "<h1>" << report.title << "</h1>\n";
      oss << "<p class='meta'>Report ID: " << report.reportID << "</p>\n";
      oss << "<p class='meta'>Generated: " << report.generatedDate << "</p>\n";
      oss << "<p class='meta'>Period: " << report.startDate << " to "
          << report.endDate << "</p>\n";

      oss << "<h2>Summary</h2>\n";
      oss << "<table>\n";
      oss << "<tr><th>Metric</th><th>Value</th></tr>\n";
      oss << "<tr><td>Total Appointments</td><td>"
          << report.statistics.totalAppointments << "</td></tr>\n";
      oss << "<tr><td>Completed</td><td>" << report.statistics.completedAppointments
          << "</td></tr>\n";
      oss << "<tr><td>Cancelled</td><td>" << report.statistics.cancelledAppointments
          << "</td></tr>\n";
      oss << "<tr><td>No Show</td><td>" << report.statistics.noShowAppointments
          << "</td></tr>\n";
      oss << "<tr><td>Total Revenue</td><td>"
          << Utils::formatMoney(report.statistics.totalRevenue) << "</td></tr>\n";
      oss << "</table>\n";

      oss << "<pre>\n"
          << report.content << "</pre>\n";

      oss << "</body>\n</html>\n";

      return oss.str();
    }

    // ==================== Display ====================

    void ReportGenerator::displayReport(const Report &report)
    {
      // Output to console
      std::cout << exportToText(report) << std::endl;
    }

  } // namespace BLL
} // namespace HMS
