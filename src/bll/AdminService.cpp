#include "bll/AdminService.h"
#include "common/Utils.h"
#include "common/Types.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace HMS
{
    namespace BLL
    {

        // ==================== Static Members ====================

        std::unique_ptr<AdminService> AdminService::s_instance = nullptr;
        std::mutex AdminService::s_mutex;

        // ==================== Constructor ====================

        AdminService::AdminService()
        {
            m_patientService = PatientService::getInstance();
            m_doctorService = DoctorService::getInstance();
            m_appointmentService = AppointmentService::getInstance();
        }

        AdminService::~AdminService() = default;

        // ==================== Singleton ====================

        AdminService *AdminService::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<AdminService>(new AdminService());
            }
            return s_instance.get();
        }

        void AdminService::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Statistics ====================

        Model::Statistics AdminService::getStatistics()
        {
            Model::Statistics stats;
            stats.reset();

            auto patients = m_patientService->getAllPatients();
            auto doctors = m_doctorService->getAllDoctors();
            auto appointments = m_appointmentService->getAllAppointments();

            stats.totalPatients = static_cast<int>(patients.size());
            stats.totalDoctors = static_cast<int>(doctors.size());
            stats.totalAppointments = static_cast<int>(appointments.size());

            // Pre-build doctor lookup map to avoid N+1 queries
            std::unordered_map<std::string, std::string> doctorSpecMap;
            for (const auto &doc : doctors)
            {
                doctorSpecMap[doc.getID()] = doc.getSpecialization();
                stats.doctorsBySpecialization[doc.getSpecialization()]++;
            }

            const std::string today = Utils::getCurrentDate();
            const std::string currentMonth = today.substr(0, 7);

            // Get week range for appointmentsThisWeek calculation
            std::string weekStart, weekEnd;
            Utils::getWeekRange(today, weekStart, weekEnd);

            for (const auto &appt : appointments)
            {
                // ===== Status =====
                switch (appt.getStatus())
                {
                case AppointmentStatus::SCHEDULED:
                    stats.scheduledAppointments++;
                    break;

                case AppointmentStatus::COMPLETED:
                    stats.completedAppointments++;
                    stats.totalRevenue += appt.getPrice();

                    if (appt.isPaid())
                        stats.paidRevenue += appt.getPrice();
                    else
                        stats.unpaidRevenue += appt.getPrice();
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

                // ===== Time-based =====
                const std::string &apptDate = appt.getDate();
                if (apptDate == today)
                    stats.appointmentsToday++;

                if (apptDate.length() >= 7 && apptDate.substr(0, 7) == currentMonth)
                    stats.appointmentsThisMonth++;

                // Check if appointment is within this week
                if (Utils::compareDates(apptDate, weekStart) >= 0 &&
                    Utils::compareDates(apptDate, weekEnd) <= 0)
                {
                    stats.appointmentsThisWeek++;
                }

                // ===== Specialization (using pre-built map) =====
                auto it = doctorSpecMap.find(appt.getDoctorID());
                if (it != doctorSpecMap.end())
                {
                    stats.appointmentsBySpecialization[it->second]++;
                }
            }

            stats.calculate();
            return stats;
        }

        // ==================== Simple Getters ====================

        int AdminService::getTotalPatients()
        {
            return static_cast<int>(m_patientService->getAllPatients().size());
        }

        int AdminService::getTotalDoctors()
        {
            return static_cast<int>(m_doctorService->getAllDoctors().size());
        }

        int AdminService::getTotalAppointments()
        {
            return static_cast<int>(m_appointmentService->getAllAppointments().size());
        }

        double AdminService::getTotalRevenue()
        {
            return m_appointmentService->getTotalRevenue();
        }

        double AdminService::getPaidRevenue()
        {
            return m_appointmentService->getPaidRevenue();
        }

        double AdminService::getUnpaidRevenue()
        {
            return m_appointmentService->getUnpaidRevenue();
        }

        // ==================== Status-Based ====================

        int AdminService::getScheduledAppointmentsCount()
        {
            return static_cast<int>(m_appointmentService->getCountByStatus(AppointmentStatus::SCHEDULED));
        }

        int AdminService::getCompletedAppointmentsCount()
        {
            return static_cast<int>(m_appointmentService->getCountByStatus(AppointmentStatus::COMPLETED));
        }

        int AdminService::getCancelledAppointmentsCount()
        {
            return static_cast<int>(m_appointmentService->getCountByStatus(AppointmentStatus::CANCELLED));
        }

        int AdminService::getNoShowAppointmentsCount()
        {
            return static_cast<int>(m_appointmentService->getCountByStatus(AppointmentStatus::NO_SHOW));
        }

        // ==================== Date-Based Queries ====================

        List<Model::Appointment> AdminService::getAppointmentsToday()
        {
            return m_appointmentService->getTodayAppointments();
        }

        List<Model::Appointment> AdminService::getAppointmentsThisWeek()
        {
            std::string startDate, endDate;
            if (!Utils::getWeekRange(Utils::getCurrentDate(), startDate, endDate))
            {
                return {};
            }
            return m_appointmentService->getAppointmentsInRange(startDate, endDate);
        }

        List<Model::Appointment> AdminService::getAppointmentsThisMonth()
        {
            const std::string today = Utils::getCurrentDate();
            const std::string startDate = today.substr(0, 7) + "-01";

            int year = std::stoi(today.substr(0, 4));
            int month = std::stoi(today.substr(5, 2));
            int daysInMonth = Utils::getDaysInMonth(month, year);

            std::ostringstream endOss;
            endOss << today.substr(0, 7) << "-" << std::setfill('0') << std::setw(2) << daysInMonth;

            return m_appointmentService->getAppointmentsInRange(startDate, endOss.str());
        }

        List<Model::Appointment> AdminService::getAppointmentsByDateRange(
            const std::string &startDate,
            const std::string &endDate)
        {
            return m_appointmentService->getAppointmentsInRange(startDate, endDate);
        }

        // ==================== Doctor Statistics ====================

        List<Model::Doctor> AdminService::getDoctorsByActivity()
        {
            auto doctors = m_doctorService->getAllDoctors();

            // Build map of doctor ID -> appointment count
            std::unordered_map<std::string, size_t> activityMap;
            for (const auto &doc : doctors)
            {
                activityMap[doc.getID()] = m_doctorService->getDoctorAppointmentCount(doc.getID());
            }

            // Sort by activity (descending)
            std::sort(doctors.begin(), doctors.end(),
                      [&activityMap](const Model::Doctor &a, const Model::Doctor &b)
                      {
                          return activityMap[a.getID()] > activityMap[b.getID()];
                      });

            return doctors;
        }

        List<Model::Doctor> AdminService::getDoctorsByRevenue()
        {
            auto doctors = m_doctorService->getAllDoctors();

            // Build map of doctor ID -> revenue
            std::unordered_map<std::string, double> revenueMap;
            for (const auto &doc : doctors)
            {
                revenueMap[doc.getID()] = m_doctorService->getDoctorRevenue(doc.getID());
            }

            // Sort by revenue (descending)
            std::sort(doctors.begin(), doctors.end(),
                      [&revenueMap](const Model::Doctor &a, const Model::Doctor &b)
                      {
                          return revenueMap[a.getID()] > revenueMap[b.getID()];
                      });

            return doctors;
        }

        std::map<std::string, int> AdminService::getDoctorCountBySpecialization()
        {
            std::map<std::string, int> result;
            auto doctors = m_doctorService->getAllDoctors();

            for (const auto &doc : doctors)
            {
                result[doc.getSpecialization()]++;
            }

            return result;
        }

        std::map<std::string, int> AdminService::getAppointmentCountBySpecialization()
        {
            std::map<std::string, int> result;
            auto doctors = m_doctorService->getAllDoctors();
            auto appointments = m_appointmentService->getAllAppointments();

            // Build doctor ID -> specialization map
            std::unordered_map<std::string, std::string> doctorSpecMap;
            for (const auto &doc : doctors)
            {
                doctorSpecMap[doc.getID()] = doc.getSpecialization();
            }

            // Count appointments by specialization
            for (const auto &appt : appointments)
            {
                auto it = doctorSpecMap.find(appt.getDoctorID());
                if (it != doctorSpecMap.end())
                {
                    result[it->second]++;
                }
            }

            return result;
        }

        // ==================== Reports ====================

        std::string AdminService::generateDailyReport(const std::string &date)
        {
            // Validate date format
            if (!Utils::isValidDate(date))
            {
                return "Error: Invalid date format. Expected YYYY-MM-DD.";
            }

            auto appointments = m_appointmentService->getAppointmentsByDate(date);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "BAO CAO NGAY " << Utils::formatDateDisplay(date) << "\n";
            oss << std::string(50, '=') << "\n\n";

            int scheduled = 0, completed = 0, cancelled = 0, noShow = 0;
            double revenue = 0.0;

            for (const auto &appt : appointments)
            {
                switch (appt.getStatus())
                {
                case AppointmentStatus::SCHEDULED:
                    scheduled++;
                    break;
                case AppointmentStatus::COMPLETED:
                    completed++;
                    revenue += appt.getPrice();
                    break;
                case AppointmentStatus::CANCELLED:
                    cancelled++;
                    break;
                case AppointmentStatus::NO_SHOW:
                    noShow++;
                    break;
                default:
                    break;
                }
            }

            oss << "TONG QUAN\n";
            oss << "   - Tong lich hen:  " << appointments.size() << "\n";
            oss << "   - Da len lich:    " << scheduled << "\n";
            oss << "   - Hoan thanh:     " << completed << "\n";
            oss << "   - Da huy:         " << cancelled << "\n";
            oss << "   - Vang mat:       " << noShow << "\n";
            oss << "   - Doanh thu:      " << Utils::formatMoney(revenue) << "\n";

            return oss.str();
        }

        std::string AdminService::generateWeeklyReport(const std::string &startDate)
        {
            // Validate date format
            if (!Utils::isValidDate(startDate))
            {
                return "Error: Invalid date format. Expected YYYY-MM-DD.";
            }

            // Calculate end date (6 days after start)
            int year = std::stoi(startDate.substr(0, 4));
            int month = std::stoi(startDate.substr(5, 2));
            int day = std::stoi(startDate.substr(8, 2));

            std::tm tm = {};
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day + 6;
            std::mktime(&tm);

            char endBuffer[11];
            std::strftime(endBuffer, sizeof(endBuffer), "%Y-%m-%d", &tm);
            std::string endDate(endBuffer);

            auto appointments = m_appointmentService->getAppointmentsInRange(startDate, endDate);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "BAO CAO TUAN\n";
            oss << "Tu " << Utils::formatDateDisplay(startDate) << " den " << Utils::formatDateDisplay(endDate) << "\n";
            oss << std::string(50, '=') << "\n\n";

            int scheduled = 0, completed = 0, cancelled = 0, noShow = 0;
            double revenue = 0.0, paidRevenue = 0.0;

            for (const auto &appt : appointments)
            {
                switch (appt.getStatus())
                {
                case AppointmentStatus::SCHEDULED:
                    scheduled++;
                    break;
                case AppointmentStatus::COMPLETED:
                    completed++;
                    revenue += appt.getPrice();
                    if (appt.isPaid())
                        paidRevenue += appt.getPrice();
                    break;
                case AppointmentStatus::CANCELLED:
                    cancelled++;
                    break;
                case AppointmentStatus::NO_SHOW:
                    noShow++;
                    break;
                default:
                    break;
                }
            }

            oss << "TONG QUAN\n";
            oss << "   - Tong lich hen:   " << appointments.size() << "\n";
            oss << "   - Da len lich:     " << scheduled << "\n";
            oss << "   - Hoan thanh:      " << completed << "\n";
            oss << "   - Da huy:          " << cancelled << "\n";
            oss << "   - Vang mat:        " << noShow << "\n\n";
            oss << "DOANH THU\n";
            oss << "   - Tong doanh thu:  " << Utils::formatMoney(revenue) << "\n";
            oss << "   - Da thanh toan:   " << Utils::formatMoney(paidRevenue) << "\n";
            oss << "   - Chua thanh toan: " << Utils::formatMoney(revenue - paidRevenue) << "\n";

            return oss.str();
        }

        std::string AdminService::generateMonthlyReport(int month, int year)
        {
            // Validate month
            if (month < 1 || month > 12)
            {
                return "Error: Invalid month. Expected 1-12.";
            }

            // Validate year (reasonable range)
            if (year < 1900 || year > 2100)
            {
                return "Error: Invalid year.";
            }

            // Build start and end dates
            std::ostringstream startOss;
            startOss << year << "-" << std::setfill('0') << std::setw(2) << month << "-01";
            std::string startDate = startOss.str();

            int daysInMonth = Utils::getDaysInMonth(month, year);

            std::ostringstream endOss;
            endOss << year << "-" << std::setfill('0') << std::setw(2) << month << "-"
                   << std::setfill('0') << std::setw(2) << daysInMonth;
            std::string endDate = endOss.str();

            auto appointments = m_appointmentService->getAppointmentsInRange(startDate, endDate);

            static const char *monthNames[] = {
                "Thang 1", "Thang 2", "Thang 3", "Thang 4", "Thang 5", "Thang 6",
                "Thang 7", "Thang 8", "Thang 9", "Thang 10", "Thang 11", "Thang 12"};

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "BAO CAO " << monthNames[month - 1] << " NAM " << year << "\n";
            oss << std::string(50, '=') << "\n\n";

            int scheduled = 0, completed = 0, cancelled = 0, noShow = 0;
            double revenue = 0.0, paidRevenue = 0.0;

            for (const auto &appt : appointments)
            {
                switch (appt.getStatus())
                {
                case AppointmentStatus::SCHEDULED:
                    scheduled++;
                    break;
                case AppointmentStatus::COMPLETED:
                    completed++;
                    revenue += appt.getPrice();
                    if (appt.isPaid())
                        paidRevenue += appt.getPrice();
                    break;
                case AppointmentStatus::CANCELLED:
                    cancelled++;
                    break;
                case AppointmentStatus::NO_SHOW:
                    noShow++;
                    break;
                default:
                    break;
                }
            }

            double completionRate = appointments.empty() ? 0.0 : (static_cast<double>(completed) / appointments.size()) * 100.0;
            double paymentRate = revenue > 0.0 ? (paidRevenue / revenue) * 100.0 : 0.0;

            oss << "TONG QUAN\n";
            oss << "   - Tong lich hen:   " << appointments.size() << "\n";
            oss << "   - Da len lich:     " << scheduled << "\n";
            oss << "   - Hoan thanh:      " << completed << " (" << completionRate << "%)\n";
            oss << "   - Da huy:          " << cancelled << "\n";
            oss << "   - Vang mat:        " << noShow << "\n\n";
            oss << "DOANH THU\n";
            oss << "   - Tong doanh thu:  " << Utils::formatMoney(revenue) << "\n";
            oss << "   - Da thanh toan:   " << Utils::formatMoney(paidRevenue) << " (" << paymentRate << "%)\n";
            oss << "   - Chua thanh toan: " << Utils::formatMoney(revenue - paidRevenue) << "\n";

            return oss.str();
        }

        std::string AdminService::generateSummaryReport()
        {
            auto stats = getStatistics();
            return stats.toReport();
        }

        // ==================== Data Management ====================

        bool AdminService::saveAllData()
        {
            bool patientSaved = m_patientService->saveData();
            bool doctorSaved = m_doctorService->saveData();
            bool appointmentSaved = m_appointmentService->saveData();

            return patientSaved && doctorSaved && appointmentSaved;
        }

        bool AdminService::loadAllData()
        {
            bool patientLoaded = m_patientService->loadData();
            bool doctorLoaded = m_doctorService->loadData();
            bool appointmentLoaded = m_appointmentService->loadData();

            return patientLoaded && doctorLoaded && appointmentLoaded;
        }

        bool AdminService::createBackup()
        {
            // TODO: Implement backup functionality
            // This would typically copy data files to a backup directory
            return saveAllData();
        }

        bool AdminService::restoreFromBackup()
        {
            // TODO: Implement restore functionality
            // This would typically copy backup files back to data directory
            return loadAllData();
        }

        // ==================== System Health ====================

        bool AdminService::checkSystemHealth()
        {
            // Check if all services are accessible
            bool patientServiceOk = (m_patientService != nullptr);
            bool doctorServiceOk = (m_doctorService != nullptr);
            bool appointmentServiceOk = (m_appointmentService != nullptr);

            // Try to load data as a health check
            bool dataAccessible = true;
            try
            {
                m_patientService->getAllPatients();
                m_doctorService->getAllDoctors();
                m_appointmentService->getAllAppointments();
            }
            catch (...)
            {
                dataAccessible = false;
            }

            return patientServiceOk && doctorServiceOk && appointmentServiceOk && dataAccessible;
        }

        std::string AdminService::getSystemStatus()
        {
            std::ostringstream oss;
            oss << "TRANG THAI HE THONG\n";
            oss << std::string(50, '=') << "\n\n";

            bool isHealthy = checkSystemHealth();
            oss << "Trang thai chung: " << (isHealthy ? "HOAT DONG" : "LOI") << "\n\n";

            oss << "DICH VU\n";
            oss << "   - PatientService:     " << (m_patientService ? "OK" : "LOI") << "\n";
            oss << "   - DoctorService:      " << (m_doctorService ? "OK" : "LOI") << "\n";
            oss << "   - AppointmentService: " << (m_appointmentService ? "OK" : "LOI") << "\n\n";

            oss << "DU LIEU\n";
            oss << "   - So benh nhan: " << getTotalPatients() << "\n";
            oss << "   - So bac si:    " << getTotalDoctors() << "\n";
            oss << "   - So lich hen:  " << getTotalAppointments() << "\n";

            return oss.str();
        }

    } // namespace BLL
} // namespace HMS
