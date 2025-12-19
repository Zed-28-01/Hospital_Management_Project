#include "model/Statistics.h"
#include "common/Utils.h"
#include <iomanip>
#include <iostream>
#include <sstream>
namespace HMS
{
    namespace Model
    {
        void Statistics::display() const
        {
            std::cout << toReport() << "\n";
        }
        void Statistics::reset()
        {
            totalPatients = 0;
            totalDoctors = 0;
            totalAppointments = 0;

            scheduledAppointments = 0;
            completedAppointments = 0;
            cancelledAppointments = 0;
            noShowAppointments = 0;

            totalRevenue = 0.0;
            paidRevenue = 0.0;
            unpaidRevenue = 0.0;
            averageConsultationFee = 0.0;

            appointmentsToday = 0;
            appointmentsThisWeek = 0;
            appointmentsThisMonth = 0;

            doctorsBySpecialization.clear();
            appointmentsBySpecialization.clear();
        }
        void Statistics::calculate()
        {
            if (totalAppointments > 0)
            {
                averageConsultationFee = totalRevenue / totalAppointments;
            }
            else
            {
                averageConsultationFee = 0.0;
            }
        }
        double Statistics::getCompletionRate() const
        {
            if (totalAppointments == 0)
                return 0.0;
            return (static_cast<double>(completedAppointments) / totalAppointments) * 100.0;
        }
        double Statistics::getCancellationRate() const
        {
            if (totalAppointments == 0)
                return 0.0;
            return (static_cast<double>(cancelledAppointments) / totalAppointments) * 100.0;
        }
        double Statistics::getPaymentRate() const
        {
            if (totalRevenue <= 0.0)
                return 0.0;
            return (paidRevenue / totalRevenue) * 100.0;
        }
        std::string Statistics::toReport() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2); // Định dạng số thập phân 2 chữ số

            oss << "THONG KE HE THONG\n";

            oss << "   - Tong benh nhan: " << totalPatients << "\n";
            oss << "   - Tong bac si:    " << totalDoctors << "\n";
            oss << "   - Tong lich hen:  " << totalAppointments << "\n";
            oss << "   - Hoan thanh: " << completedAppointments
                << " (" << getCompletionRate() << "%)\n";
            oss << "   - Da huy:     " << cancelledAppointments
                << " (" << getCancellationRate() << "%)\n";
            oss << "   - Vang mat:   " << noShowAppointments << "\n";

            oss << "THONG KE DOANH THU" << "\n";
            oss << "   - Tong doanh thu: " << Utils::formatMoney(totalRevenue) << "\n";
            oss << "   - Da thanh toan:  " << Utils::formatMoney(paidRevenue) << " (" << getPaymentRate() << "%)\n";
            oss << "   - Chua thanh toan: " << Utils::formatMoney(unpaidRevenue) << "\n";
            oss << "   - Trung binh/ca:  " << Utils::formatMoney(averageConsultationFee) << "\n";

            oss << "THONG KE THEO KHOA" << "\n";
            for (const auto &pair : doctorsBySpecialization)
            {
                oss << "   - " << pair.first << ": " << pair.second << " bac si\n";
            }
            return oss.str();
        }
    }
}
