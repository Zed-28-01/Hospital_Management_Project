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
            oss << "THỐNG KÊ HỆ THỐNG\n";
            oss << "   - Tổng bệnh nhân: " << totalPatients << "\n";
            oss << "   - Tổng bác sĩ:    " << totalDoctors << "\n";
            oss << "   - Tổng lịch hẹn:  " << totalAppointments << "\n";
            oss << "   - Hoàn thành: " << completedAppointments
                << " (" << getCompletionRate() << "%)\n";
            oss << "   - Đã hủy:     " << cancelledAppointments
                << " (" << getCancellationRate() << "%)\n";
            oss << "   - Vắng mặt:   " << noShowAppointments << "\n";
            // ===== Financial Statistics =====
            oss << "THỐNG KÊ DOANH THU\n";
            oss << "   - Tổng doanh thu: " << Utils::formatMoney(totalRevenue) << "\n";
            oss << "   - Đã thanh toán:  " << Utils::formatMoney(paidRevenue)
                << " (" << getPaymentRate() << "%)\n";
            oss << "   - Chưa thanh toán: " << Utils::formatMoney(unpaidRevenue) << "\n";
            oss << "   - Trung bình/ca:  " << Utils::formatMoney(averageConsultationFee) << "\n";

            // ===== Specialization Statistics =====
            if (!doctorsBySpecialization.empty())
            {
                oss << "THỐNG KÊ THEO CHUYÊN KHOA\n";
                for (const auto &pair : doctorsBySpecialization)
                {
                    oss << "   - " << pair.first << ": " << pair.second << " bác sĩ\n";
                }
            }

            // ===== Medicine Statistics (Advance) =====
            if (totalMedicines > 0)
            {
                oss << "THỐNG KÊ THUỐC\n";
                oss << "   - Tổng số thuốc:    " << totalMedicines << "\n";
                oss << "   - Sắp hết hàng:     " << lowStockMedicines
                    << " (" << getLowStockRate() << "%)\n";
                oss << "   - Đã hết hạn:       " << expiredMedicines
                    << " (" << getExpiredRate() << "%)\n";
                oss << "   - Sắp hết hạn:      " << expiringSoonMedicines << "\n";
                oss << "   - Giá trị tồn kho:  " << Utils::formatMoney(totalInventoryValue) << "\n";

                if (!medicinesByCategory.empty())
                {
                    oss << "   Theo danh mục:\n";
                    for (const auto &pair : medicinesByCategory)
                    {
                        oss << "     - " << pair.first << ": " << pair.second << " loại\n";
                    }
                }
            }

            // ===== Department Statistics (Advance) =====
            if (totalDepartments > 0)
            {
                oss << "THỐNG KÊ KHOA/PHÒNG\n";
                oss << "   - Tổng số khoa: " << totalDepartments << "\n";

                if (!doctorsByDepartment.empty())
                {
                    oss << "   Bác sĩ theo khoa:\n";
                    for (const auto &pair : doctorsByDepartment)
                    {
                        oss << "     - " << pair.first << ": " << pair.second << " bác sĩ\n";
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
                oss << "THỐNG KÊ ĐƠN THUỐC\n";
                oss << "   - Tổng số đơn:      " << totalPrescriptions << "\n";
                oss << "   - Đã phát thuốc:    " << dispensedPrescriptions
                    << " (" << getDispenseRate() << "%)\n";
                oss << "   - Chờ phát thuốc:   " << pendingPrescriptions << "\n";
                oss << "   - Tổng số mục thuốc: " << totalPrescriptionItems << "\n";
            }

            return oss.str();
        }

    } // namespace Model
} // namespace HMS
