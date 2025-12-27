#pragma once

/**
 * @file ReportGenerator.h
 * @brief Placeholder for advance Report Generation feature
 *
 * This file is a placeholder for the Report Generation feature
 * planned for advance implementation. When implemented, this will
 * generate various reports for administrative purposes.
 *
 * Planned features:
 * - Daily/Weekly/Monthly reports
 * - Revenue reports
 * - Patient statistics reports
 * - Doctor performance reports
 * - Export to PDF/CSV/Excel
 */

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "../model/Statistics.h"
#include "../model/Appointment.h"
#include "../model/Patient.h"
#include "../model/Doctor.h"

namespace HMS
{
    namespace BLL
    {

        /**
         * @enum ReportType
         * @brief Types of reports that can be generated
         */
        enum class ReportType
        {
            DAILY_SUMMARY,
            WEEKLY_SUMMARY,
            MONTHLY_SUMMARY,
            REVENUE_REPORT,
            PATIENT_STATISTICS,
            DOCTOR_PERFORMANCE,
            APPOINTMENT_ANALYSIS,
            CUSTOM
        };

        /**
         * @enum ExportFormat
         * @brief Supported export formats
         */
        enum class ExportFormat
        {
            TEXT,
            CSV,
            HTML,
            PDF // Would require external library
        };

        /**
         * @struct Report
         * @brief Represents a generated report
         */
        struct Report
        {
            std::string reportID;
            ReportType type;
            std::string title;
            std::string generatedDate;
            std::string startDate;
            std::string endDate;
            std::string content;
            Model::Statistics statistics;
        };

        /**
         * @class ReportGenerator
         * @brief Generates various reports for the HMS (ADVANCE IMPLEMENTATION)
         *
         * @note This is a placeholder. Implementation pending.
         */
        class ReportGenerator
        {
        private:
            // ==================== Singleton ====================
            static std::unique_ptr<ReportGenerator> s_instance;
            static std::mutex s_mutex;

            ReportGenerator() = default;

        public:
            // ==================== Singleton ====================

            static ReportGenerator *getInstance();

            /**
             * @brief Reset the singleton instance (for testing)
             */
            static void resetInstance();

            ReportGenerator(const ReportGenerator &) = delete;
            ReportGenerator &operator=(const ReportGenerator &) = delete;

            ~ReportGenerator() = default;

            // ==================== Report Generation ====================

            /**
             * @brief Generate daily summary report
             * @param date The date for the report (YYYY-MM-DD)
             * @return Generated report
             */
            Report generateDailyReport(const std::string &date);

            /**
             * @brief Generate weekly summary report
             * @param startDate Start date of the week
             * @return Generated report
             */
            Report generateWeeklyReport(const std::string &startDate);

            /**
             * @brief Generate monthly summary report
             * @param month Month (1-12)
             * @param year Year
             * @return Generated report
             */
            Report generateMonthlyReport(int month, int year);

            /**
             * @brief Generate revenue report
             * @param startDate Start date
             * @param endDate End date
             * @return Generated report
             */
            Report generateRevenueReport(const std::string &startDate,
                                         const std::string &endDate);

            /**
             * @brief Generate patient statistics report
             * @return Generated report
             */
            Report generatePatientReport();

            /**
             * @brief Generate doctor performance report
             * @param doctorID Doctor's ID (empty for all doctors)
             * @param startDate Start date
             * @param endDate End date
             * @return Generated report
             */
            Report generateDoctorPerformanceReport(const std::string &doctorID,
                                                   const std::string &startDate,
                                                   const std::string &endDate);

            /**
             * @brief Generate appointment analysis report
             * @param startDate Start date
             * @param endDate End date
             * @return Generated report
             */
            Report generateAppointmentAnalysis(const std::string &startDate,
                                               const std::string &endDate);

            /**
             * @brief Generate custom report with filters
             * @param type Report type
             * @param startDate Start date
             * @param endDate End date
             * @param filters Additional filters (key-value pairs)
             * @return Generated report
             */
            Report generateCustomReport(ReportType type,
                                        const std::string &startDate,
                                        const std::string &endDate,
                                        const std::vector<std::pair<std::string, std::string>> &filters);

            // ==================== Export ====================

            /**
             * @brief Export report to file
             * @param report The report to export
             * @param format Export format
             * @param filePath Output file path
             * @return True if export successful
             */
            bool exportReport(const Report &report,
                              ExportFormat format,
                              const std::string &filePath);

            /**
             * @brief Export report to text format
             * @param report The report
             * @return Text content
             */
            std::string exportToText(const Report &report);

            /**
             * @brief Export report to CSV format
             * @param report The report
             * @return CSV content
             */
            std::string exportToCSV(const Report &report);

            /**
             * @brief Export report to HTML format
             * @param report The report
             * @return HTML content
             */
            std::string exportToHTML(const Report &report);

            // ==================== Display ====================

            /**
             * @brief Display report to console
             * @param report The report to display
             */
            void displayReport(const Report &report);
        };

    } // namespace BLL
} // namespace HMS

/**
 * @note Advance files to create when implementing ReportGenerator:
 *
 * Implementation:
 * - src/bll/ReportGenerator.cpp
 *
 * For PDF export, consider using:
 * - libharu (free, lightweight)
 * - PDFWriter (open source)
 *
 * For Excel export, consider using:
 * - xlsxwriter
 * - OpenXLSX
 */
