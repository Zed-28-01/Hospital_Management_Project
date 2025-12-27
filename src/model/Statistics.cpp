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
            // Core counts
            totalPatients = 0;
            totalDoctors = 0;
            totalAppointments = 0;

            // Appointment status
            scheduledAppointments = 0;
            completedAppointments = 0;
            cancelledAppointments = 0;
            noShowAppointments = 0;

            // Financial
            totalRevenue = 0.0;
            paidRevenue = 0.0;
            unpaidRevenue = 0.0;
            averageConsultationFee = 0.0;

            // Time-based
            appointmentsToday = 0;
            appointmentsThisWeek = 0;
            appointmentsThisMonth = 0;

            // Specialization distribution
            doctorsBySpecialization.clear();
            appointmentsBySpecialization.clear();

            // Medicine statistics (Advance)
            totalMedicines = 0;
            lowStockMedicines = 0;
            expiredMedicines = 0;
            expiringSoonMedicines = 0;
            totalInventoryValue = 0.0;
            medicinesByCategory.clear();
            inventoryValueByCategory.clear();

            // Department statistics (Advance)
            totalDepartments = 0;
            doctorsByDepartment.clear();
            revenueByDepartment.clear();
            appointmentsByDepartment.clear();

            // Prescription statistics (Advance)
            totalPrescriptions = 0;
            dispensedPrescriptions = 0;
            pendingPrescriptions = 0;
            totalPrescriptionItems = 0;
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

            // Calculate pending prescriptions
            pendingPrescriptions = totalPrescriptions - dispensedPrescriptions;
            if (pendingPrescriptions < 0)
            {
                pendingPrescriptions = 0;
            }
        }

        // ==================== Rate Calculations ====================

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

        double Statistics::getLowStockRate() const
        {
            if (totalMedicines == 0)
                return 0.0;
            return (static_cast<double>(lowStockMedicines) / totalMedicines) * 100.0;
        }

        double Statistics::getExpiredRate() const
        {
            if (totalMedicines == 0)
                return 0.0;
            return (static_cast<double>(expiredMedicines) / totalMedicines) * 100.0;
        }

        double Statistics::getDispenseRate() const
        {
            if (totalPrescriptions == 0)
                return 0.0;
            return (static_cast<double>(dispensedPrescriptions) / totalPrescriptions) * 100.0;
        }

        // ==================== Report Generation ====================

        std::string Statistics::toReport() const
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);

            // ===== Core Statistics =====
            oss << "THONG KE HE THONG\n";
            oss << "   - Tong benh nhan: " << totalPatients << "\n";
            oss << "   - Tong bac si:    " << totalDoctors << "\n";
            oss << "   - Tong lich hen:  " << totalAppointments << "\n";
            oss << "   - Hoan thanh: " << completedAppointments
                << " (" << getCompletionRate() << "%)\n";
            oss << "   - Da huy:     " << cancelledAppointments
                << " (" << getCancellationRate() << "%)\n";
            oss << "   - Vang mat:   " << noShowAppointments << "\n";

            // ===== Financial Statistics =====
            oss << "THONG KE DOANH THU\n";
            oss << "   - Tong doanh thu: " << Utils::formatMoney(totalRevenue) << "\n";
            oss << "   - Da thanh toan:  " << Utils::formatMoney(paidRevenue)
                << " (" << getPaymentRate() << "%)\n";
            oss << "   - Chua thanh toan: " << Utils::formatMoney(unpaidRevenue) << "\n";
            oss << "   - Trung binh/ca:  " << Utils::formatMoney(averageConsultationFee) << "\n";

            // ===== Specialization Statistics =====
            if (!doctorsBySpecialization.empty())
            {
                oss << "THONG KE THEO CHUYEN KHOA\n";
                for (const auto &pair : doctorsBySpecialization)
                {
                    oss << "   - " << pair.first << ": " << pair.second << " bac si\n";
                }
            }

            // ===== Medicine Statistics (Advance) =====
            if (totalMedicines > 0)
            {
                oss << "THONG KE THUOC\n";
                oss << "   - Tong so thuoc:    " << totalMedicines << "\n";
                oss << "   - Sap het hang:     " << lowStockMedicines
                    << " (" << getLowStockRate() << "%)\n";
                oss << "   - Da het han:       " << expiredMedicines
                    << " (" << getExpiredRate() << "%)\n";
                oss << "   - Sap het han:      " << expiringSoonMedicines << "\n";
                oss << "   - Gia tri ton kho:  " << Utils::formatMoney(totalInventoryValue) << "\n";

                if (!medicinesByCategory.empty())
                {
                    oss << "   Theo danh muc:\n";
                    for (const auto &pair : medicinesByCategory)
                    {
                        oss << "     - " << pair.first << ": " << pair.second << " loai\n";
                    }
                }
            }

            // ===== Department Statistics (Advance) =====
            if (totalDepartments > 0)
            {
                oss << "THONG KE KHOA/PHONG\n";
                oss << "   - Tong so khoa: " << totalDepartments << "\n";

                if (!doctorsByDepartment.empty())
                {
                    oss << "   Bac si theo khoa:\n";
                    for (const auto &pair : doctorsByDepartment)
                    {
                        oss << "     - " << pair.first << ": " << pair.second << " bac si\n";
                    }
                }

                if (!revenueByDepartment.empty())
                {
                    oss << "   Doanh thu theo khoa:\n";
                    for (const auto &pair : revenueByDepartment)
                    {
                        oss << "     - " << pair.first << ": "
                            << Utils::formatMoney(pair.second) << "\n";
                    }
                }
            }

            // ===== Prescription Statistics (Advance) =====
            if (totalPrescriptions > 0)
            {
                oss << "THONG KE DON THUOC\n";
                oss << "   - Tong so don:      " << totalPrescriptions << "\n";
                oss << "   - Da phat thuoc:    " << dispensedPrescriptions
                    << " (" << getDispenseRate() << "%)\n";
                oss << "   - Cho phat thuoc:   " << pendingPrescriptions << "\n";
                oss << "   - Tong so muc thuoc: " << totalPrescriptionItems << "\n";
            }

            return oss.str();
        }

    } // namespace Model
} // namespace HMS
