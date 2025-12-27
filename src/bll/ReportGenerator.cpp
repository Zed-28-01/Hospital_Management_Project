#include "advance/ReportGenerator.h"
#include "dal/AppointmentRepository.h"
#include "bll/AppointmentService.h"
#include "common/Utils.h"
#include "common/Types.h"

#include <sstream>
#include <ctime>
#include <format>

namespace HMS
{
    namespace BLL
    {
        // ======================================== SINGLETON INIT =========================================

        ReportGenerator *ReportGenerator::s_instance = nullptr;

        ReportGenerator *ReportGenerator::getInstance() {
            if (!s_instance)
            {
                s_instance = new ReportGenerator();
            }
            return s_instance;
        }

        // ======================================== REPORT GENERATION =======================================

        Report ReportGenerator::generateDailyReport(const std::string &date) {

            Report report;

            if (!Utils::isValidDate(date)) {
                return report;
            }
            report.reportID = Utils::generateID("RPDL");
            report.type = ReportType::DAILY_SUMMARY;
            report.title = "BAO CAO HANG NGAY - " + date;
            report.generatedDate = Utils::getCurrentDate();
            report.startDate = date;
            report.endDate = date;

            auto apptRepo = DAL::AppointmentRepository::getInstance();
            auto appointments = apptRepo->getByDate(date);

            int total = 0;
            int completed = 0;
            int cancelled = 0;
            double dailyRevenue = 0.0;
            int pending = 0;

            for (const auto &appt : appointments) {
                switch (appt.getStatus()) {
                    case AppointmentStatus::COMPLETED:
                        completed++;
                        dailyRevenue += appt.getPrice();
                        break;
                    case AppointmentStatus::CANCELLED:
                        cancelled++;
                        break;
                    default:
                        break;
                }
            }

                std::stringstream ss;
                ss << "BAO CAO HANG NGAY\n";
                ss << "Ngay: " << date << "\n";
                ss << "  - Tong lich kham: " << static_cast<int>(appointments.size()) << "\n";
                ss << "  - Da hoan thanh: " << completed << "\n";
                ss << "  - Da huy: " << cancelled << "\n";
                ss << "  - Tong doanh thu: " << Utils::formatMoney(dailyRevenue) << "\n";

                report.content = ss.str();
                report.statistics.totalRevenue = dailyRevenue;
                report.statistics.totalAppointments = static_cast<int>(appointments.size());

                return report;
            }

            Report ReportGenerator::generateWeeklyReport(const std::string &startDate) {
                Report report;
                if (!Utils::isValidDate(startDate)) {
                    return report;
                }

                int y, m, d;
                sscanf(startDate.c_str(), "%d-%d-%d", &y, &m, &d);
                std::tm tm_start = {};
                tm_start.tm_year = y - 1900;
                tm_start.tm_mon = m - 1;
                tm_start.tm_mday = d;

                std::tm tm_end = tm_start;
                tm_end.tm_mday += 6;
                std::mktime(&tm_end);

                char buf[11];
                std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_end);
                std::string endDate(buf);

                auto appointments = AppointmentService::getInstance()->getAppointmentsInRange(startDate, endDate);

                double totalRevenue = 0.0;
                int completedCount = 0;
                int cancelledCount = 0;

                for (const auto &appt : appointments) {
                    if (appt.getStatus() == AppointmentStatus::COMPLETED) {
                        completedCount++;
                        totalRevenue += appt.getPrice();
                    } else if (appt.getStatus() == AppointmentStatus::CANCELLED) {
                        cancelledCount++;
                    }
                }

                report.reportID = Utils::generateID("RPWKL");
                report.type = ReportType::WEEKLY_SUMMARY;
                report.title = "BAO CAO THEO TUAN (" + startDate + " - " + endDate + ")";
                report.generatedDate = Utils::getCurrentDate();
                report.startDate = startDate;
                report.endDate = endDate;
                report.statistics.totalRevenue = totalRevenue;
                report.statistics.totalAppointments = static_cast<int>(appointments.size());
                std::stringstream ss;

                ss << "BAO CAO TUAN" << "\n";
                ss << "Tu ngay: " << startDate << " den " << endDate << "\n";
                ss << "  - Tong lich kham: " << appointments.size() << "\n";
                ss << "  - Da hoan thanh: " << completedCount << "\n";
                ss << "  - Bi huy: " << cancelledCount << "\n";
                ss << "  - Tong doanh thu:" << Utils::formatMoney(totalRevenue) << "\n";

                report.content = ss.str();
                return report;
            }


            Report ReportGenerator::generateMonthlyReport(int month, int year) {
                Report report;

                std::string startDate = Utils::toDateString(year, month, 1);
                std::string endDate = Utils::toDateString(year, month, Utils::getDaysInMonth(month, year));

                auto appointments = AppointmentService::getInstance()->getAppointmentsInRange(startDate, endDate);

                double revenue = 0.0;
                int completed = 0;
                int cancelled = 0;
                for (const auto &appt : appointments) {
                    if (appt.getStatus() == AppointmentStatus::COMPLETED) {
                        revenue += appt.getPrice();
                        completed++;
                    } else if (appt.getStatus() == AppointmentStatus::CANCELLED) {
                        cancelled++;
                    }
                }
                report.reportID = Utils::generateID("RPML");
                std::string title = std::format("BAO CAO THANG {}/{}", month, year);
                report.title = title;
                report.generatedDate = Utils::getCurrentDate();
                report.statistics.totalRevenue = revenue;

                std::stringstream ss;
                ss << "BAO CAO THANG " << month << "/" << year;
                ss << "  - Tong ca kham: " << static_cast<int>(appointments.size()) <<"\n";
                ss << "  - So ca hoan thanh: " << completed << "\n";
                ss << "  - So ca bi huy: " << cancelled << "\n";
                ss << "  - Tong doanh thu: " << Utils::formatMoney(revenue) << "\n";
                report.content = ss.str();
                return report;
            }

            Report ReportGenerator::generateRevenueReport(const std::string& startDate, const std::string& endDate) {
                auto appointments = AppointmentService::getInstance()->getAppointmentsInRange(startDate, endDate);
                double revenue = 0.0;

                for (auto const&  appt : appointments) {
                    if (appt.getStatus() == AppointmentStatus::COMPLETED) {
                        revenue += appt.getPrice();
                    }
                }

                Report report;
                report.reportID = Utils::generateID("RPRVN");
                std::string title = std::format("BAO CAO DOANH THU TU {} DEN", startDate, endDate);
                report.title = title;
                report.generatedDate = Utils::getCurrentDate();
                report.statistics.totalRevenue = revenue;

                std::stringstream ss;
                ss << "BAO CAO DOANH THU TU " << startDate << " DEN " << endDate;
                ss << "  - Tong doanh thu: " << Utils::formatMoney(revenue) << "\n";
                report.content = ss.str();
                return report;
            }


            
        }
    }


