#include "bll/AdminService.h"

#include "common/Utils.h"
#include "common/Types.h"

#include <algorithm>
#include <sstream>

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

            std::string today = HMS::Utils::getCurrentDate();

            for (const auto &appt : appointments)
            {

                // ===== Status =====
                switch (appt.getStatus())
                {
                case HMS::AppointmentStatus::SCHEDULED:
                    stats.scheduledAppointments++;
                    break;

                case HMS::AppointmentStatus::COMPLETED:
                    stats.completedAppointments++;
                    stats.totalRevenue += appt.getPrice();

                    if (appt.isPaid())
                        stats.paidRevenue += appt.getPrice();
                    else
                        stats.unpaidRevenue += appt.getPrice();
                    break;

                case HMS::AppointmentStatus::CANCELLED:
                    stats.cancelledAppointments++;
                    break;

                case HMS::AppointmentStatus::NO_SHOW:
                    stats.noShowAppointments++;
                    break;
                }

                // ===== Time-based =====
                if (appt.getDate() == today)
                    stats.appointmentsToday++;

                if (appt.getDate().substr(0, 7) == today.substr(0, 7))
                    stats.appointmentsThisMonth++;

                // ===== Specialization =====
                auto docOpt = m_doctorService->getDoctorByID(appt.getDoctorID());
                if (docOpt)
                {
                    const std::string &spec = docOpt->getSpecialization();
                    stats.appointmentsBySpecialization[spec]++;
                }
            }

            // ===== Doctor specialization =====
            for (const auto &doc : doctors)
            {
                stats.doctorsBySpecialization[doc.getSpecialization()]++;
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
            return getStatistics().totalRevenue;
        }

        double AdminService::getPaidRevenue()
        {
            return getStatistics().paidRevenue;
        }

        double AdminService::getUnpaidRevenue()
        {
            return getStatistics().unpaidRevenue;
        }

        // ==================== Status-Based ====================

        int AdminService::getScheduledAppointmentsCount()
        {
            return getStatistics().scheduledAppointments;
        }

        int AdminService::getCompletedAppointmentsCount()
        {
            return getStatistics().completedAppointments;
        }

        int AdminService::getCancelledAppointmentsCount()
        {
            return getStatistics().cancelledAppointments;
        }

        int AdminService::getNoShowAppointmentsCount()
        {
            return getStatistics().noShowAppointments;
        }

    } // namespace BLL
} // namespace HMS
