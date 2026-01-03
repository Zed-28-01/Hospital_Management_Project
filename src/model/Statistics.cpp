#include "model/Statistics.h"
#include "common/Utils.h"
#include <iostream>
#include <format>

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
            std::string report;

            // ===== Core Statistics =====
            report += "THỐNG KÊ HỆ THỐNG\n";
            report += std::format("   - {:<20} : {}\n", "Tổng bệnh nhân", totalPatients);
            report += std::format("   - {:<20} : {}\n", "Tổng bác sĩ", totalDoctors);
            report += std::format("   - {:<20} : {}\n", "Tổng lịch hẹn", totalAppointments);
            report += std::format("   - {:<20} : {} ({:.2f}%)\n",
                                "Hoàn thành", completedAppointments, getCompletionRate());
            report += std::format("   - {:<20} : {} ({:.2f}%)\n",
                                "Đã hủy", cancelledAppointments, getCancellationRate());
            report += std::format("   - {:<20} : {}\n", "Vắng mặt", noShowAppointments);

            // ===== Financial Statistics =====
            report += "THỐNG KÊ DOANH THU\n";
            report += std::format("   - {:<20} : {}\n",
                                "Tổng doanh thu", Utils::formatMoney(totalRevenue));
            report += std::format("   - {:<20} : {} ({:.2f}%)\n",
                                "Đã thanh toán", Utils::formatMoney(paidRevenue), getPaymentRate());
            report += std::format("   - {:<20} : {}\n",
                                "Chưa thanh toán", Utils::formatMoney(unpaidRevenue));
            report += std::format("   - {:<20} : {}\n",
                                "Trung bình/ca", Utils::formatMoney(averageConsultationFee));

            // ===== Specialization Statistics =====
            if (!doctorsBySpecialization.empty())
            {
                report += "THỐNG KÊ THEO CHUYÊN KHOA\n";
                for (const auto &[specialization, count] : doctorsBySpecialization)
                {
                    report += std::format("   - {:<20} : {} bác sĩ\n", specialization, count);                }
            }

            // ===== Medicine Statistics (Advance) =====
            if (totalMedicines > 0)
            {
                report += "THỐNG KÊ THUỐC\n";
                report += std::format("   - {:<20} : {}\n", "Tổng số thuốc", totalMedicines);
                report += std::format("   - {:<20} : {} ({:.2f}%)\n",
                                    "Sắp hết hàng", lowStockMedicines, getLowStockRate());
                report += std::format("   - {:<20} : {} ({:.2f}%)\n",
                                    "Đã hết hạn", expiredMedicines, getExpiredRate());
                report += std::format("   - {:<20} : {}\n",
                                    "Sắp hết hạn", expiringSoonMedicines);
                report += std::format("   - {:<20} : {}\n",
                                    "Giá trị tồn kho", Utils::formatMoney(totalInventoryValue));


                if (!medicinesByCategory.empty())
                {
                    report += "   Theo danh mục:\n";
                    for (const auto &pair : medicinesByCategory)
                    {
                        report += std::format("     - {:<20} : {} loại\n", pair.first, pair.second);
                    }
                }
            }

            // ===== Department Statistics (Advance) =====
            if (totalDepartments > 0)
            {
                report += "THỐNG KÊ KHOA/PHÒNG\n";
                report += std::format("   - {:<20} : {}\n", "Tổng số khoa", totalDepartments);

                if (!doctorsByDepartment.empty())
                {
                    report += "   Bác sĩ theo khoa:\n";
                    for (const auto &pair : doctorsByDepartment)
                    {
                        report += std::format("     - {:<20} : {} bác sĩ\n", pair.first, pair.second);
                    }
                }

                if (!revenueByDepartment.empty())
                {
                    report += "   Doanh thu theo khoa:\n";
                    for (const auto &pair : revenueByDepartment)
                    {
                        report += std::format("     - {:<20} : {}\n", pair.first, Utils::formatMoney(pair.second));
                    }
                }
            }

            // ===== Prescription Statistics (Advance) =====
            if (totalPrescriptions > 0)
            {
                report += "THỐNG KÊ ĐƠN THUỐC\n";
                report += std::format("   - {:<20} : {}\n", "Tổng số đơn", totalPrescriptions);
                report += std::format("   - {:<20} : {} ({:.2f}%)\n", "Đã phát thuốc", dispensedPrescriptions, getDispenseRate());
                report += std::format("   - {:<20} : {}\n", "Chờ phát thuốc", pendingPrescriptions);
                report += std::format("   - {:<20} : {}\n", "Tổng số mục thuốc", totalPrescriptionItems);
            }

            return report;
        }

    } // namespace Model
} // namespace HMS,
