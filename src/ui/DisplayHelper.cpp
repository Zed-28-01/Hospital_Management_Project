#include "ui/DisplayHelper.h"
#include "common/Utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

namespace HMS
{
    namespace UI
    {

        // ==================== ANSI Color Codes ====================
        // Already defined in header as static constexpr

        // ==================== Constants ====================
        namespace
        {
            constexpr int DEFAULT_PAGE_SIZE = 40;  // Items per page
            constexpr int DEFAULT_COL_WIDTH = 60;  // Default column width
        }

        // ==================== Pagination Helper ====================
        namespace
        {
            // Local color constants for helper functions
            constexpr const char* RESET_LOCAL = "\033[0m";
            constexpr const char* RED_LOCAL = "\033[31m";
            constexpr const char* GREEN_LOCAL = "\033[32m";
            constexpr const char* YELLOW_LOCAL = "\033[33m";
            constexpr const char* BLUE_LOCAL = "\033[34m";
            constexpr const char* CYAN_LOCAL = "\033[36m";
            constexpr const char* BOLD_LOCAL = "\033[1m";

            /**
             * @brief Interactive pagination handler for table display
             * @param headers Column headers
             * @param allRows All table rows
             * @param colWidths Column widths
             * @param pageSize Number of rows per page
             */
            void paginateTable(const std::vector<std::string> &headers,
                               const std::vector<std::vector<std::string>> &allRows,
                               const std::vector<int> &colWidths,
                               int pageSize = DEFAULT_PAGE_SIZE)
            {
                if (allRows.empty())
                {
                    std::cout << "Khong co du lieu de hien thi.\n";
                    return;
                }

                int totalRows = allRows.size();
                int totalPages = (totalRows + pageSize - 1) / pageSize;
                int currentPage = 1;

                while (true)
                {
                    // Clear screen
                    DisplayHelper::clearScreen();

                    // Calculate page boundaries
                    int startIdx = (currentPage - 1) * pageSize;
                    int endIdx = std::min(startIdx + pageSize, totalRows);

                    // Get rows for current page
                    std::vector<std::vector<std::string>> pageRows(
                        allRows.begin() + startIdx,
                        allRows.begin() + endIdx);

                    // Print table for current page
                    DisplayHelper::printTable(headers, pageRows, colWidths);

                    // Print pagination info
                    std::cout << "\n";
                    DisplayHelper::printSeparator(60, '-');
                    std::cout << CYAN_LOCAL << "Trang " << currentPage << "/" << totalPages
                              << " (Hien thi " << startIdx + 1 << "-" << endIdx
                              << " / " << totalRows << " muc)" << RESET_LOCAL << "\n";
                    DisplayHelper::printSeparator(60, '-');

                    // Print navigation menu
                    std::cout << "\n";
                    if (currentPage > 1)
                        std::cout << "[P] Trang truoc  ";
                    if (currentPage < totalPages)
                        std::cout << "[N] Trang sau  ";
                    if (totalPages > 1)
                        std::cout << "[G] Den trang  ";
                    std::cout << "[Q] Quay lại menu\n";
                    std::cout << "\nNhap lua chon: ";

                    std::string choice;
                    std::getline(std::cin, choice);
                    choice = Utils::toLower(Utils::trim(choice));

                    if (choice == "q" || choice == "quit" || choice.empty())
                    {
                        break;
                    }
                    else if (choice == "n" || choice == "next")
                    {
                        if (currentPage < totalPages)
                        {
                            currentPage++;
                        }
                        else
                        {
                            std::cout << YELLOW_LOCAL << "Day la trang cuoi cung!" << RESET_LOCAL << "\n";
                            DisplayHelper::pause();
                        }
                    }
                    else if (choice == "p" || choice == "prev" || choice == "previous")
                    {
                        if (currentPage > 1)
                        {
                            currentPage--;
                        }
                        else
                        {
                            std::cout << YELLOW_LOCAL << "Day la trang dau tien!" << RESET_LOCAL << "\n";
                            DisplayHelper::pause();
                        }
                    }
                    else if (choice == "g" || choice == "goto")
                    {
                        std::cout << "Nhap so trang (1-" << totalPages << "): ";
                        std::string pageStr;
                        std::getline(std::cin, pageStr);
                        try
                        {
                            int targetPage = std::stoi(pageStr);
                            if (targetPage >= 1 && targetPage <= totalPages)
                            {
                                currentPage = targetPage;
                            }
                            else
                            {
                                std::cout << YELLOW_LOCAL << "So trang khong hop le!" << RESET_LOCAL << "\n";
                                DisplayHelper::pause();
                            }
                        }
                        catch (...)
                        {
                            std::cout << RED_LOCAL << "Vui long nhap so hop le!" << RESET_LOCAL << "\n";
                            DisplayHelper::pause();
                        }
                    }
                    else
                    {
                        std::cout << YELLOW_LOCAL << "Lua chon khong hop le!" << RESET_LOCAL << "\n";
                        DisplayHelper::pause();
                    }
                }
            }
        }

        // ==================== Screen Control ====================

        void DisplayHelper::clearScreen()
        {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
        }

        void DisplayHelper::pause()
        {
            std::cout << "\nNhan Enter de tiep tuc...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        void DisplayHelper::pause(const std::string &message)
        {
            std::cout << "\n"
                      << message;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // ==================== Headers & Titles ====================

        void DisplayHelper::printAppHeader()
        {
            clearScreen();
            printSeparator(60, '=');
            std::cout << BOLD << CYAN;
            std::cout << "            HE THONG QUAN LY BENH VIEN\n";
            std::cout << "          HOSPITAL MANAGEMENT SYSTEM\n";
            std::cout << RESET;
            printSeparator(60, '=');
            std::cout << "\n";
        }

        void DisplayHelper::printHeader(const std::string &title)
        {
            std::cout << "\n";
            printSeparator(60, '=');
            std::cout << BOLD << BLUE << "  " << title << RESET << "\n";
            printSeparator(60, '=');
            std::cout << "\n";
        }

        void DisplayHelper::printSubHeader(const std::string &title)
        {
            std::cout << "\n";
            printThinSeparator(60);
            std::cout << BOLD << "  " << title << RESET << "\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        void DisplayHelper::printSeparator(int width, char ch)
        {
            std::cout << std::string(width, ch) << "\n";
        }

        void DisplayHelper::printThinSeparator(int width)
        {
            printSeparator(width, '-');
        }

        // ==================== Menu Display ====================

        void DisplayHelper::printMenu(const std::string &title,
                                      const std::vector<std::string> &options)
        {
            printHeader(title);
            for (size_t i = 0; i < options.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
            }
            std::cout << "  0. Quay lai\n";
            std::cout << "\n";
        }

        void DisplayHelper::printMainMenu()
        {
            printAppHeader();
            std::cout << BOLD << "MENU CHINH\n"
                      << RESET;
            printThinSeparator(40);
            std::cout << "  1. Dang nhap\n";
            std::cout << "  2. Dang ky (Benh nhan)\n";
            std::cout << "  0. Thoat\n";
            printThinSeparator(40);
            std::cout << "\n";
        }

        void DisplayHelper::printPatientMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << GREEN << "MENU BENH NHAN" << RESET << " - Xin chao, " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Xem thong tin ca nhan\n";
            std::cout << "  2. Dat lich kham\n";
            std::cout << "  3. Xem lich su kham benh\n";
            std::cout << "  4. Xem lich hen sap toi\n";
            std::cout << "  5. Huy lich hen\n";
            std::cout << "  6. Xem tong hoa don\n";
            std::cout << "  0. Đăng xuất\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        void DisplayHelper::printDoctorMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << BLUE << "MENU BAC SI" << RESET << " - Xin chao, Dr. " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Xem lich lam viec\n";
            std::cout << "  2. Danh dau lich hen hoan thanh\n";
            std::cout << "  3. Danh sách bệnh nhân\n";
            std::cout << "  4. Tìm kiếm bệnh nhân\n";
            std::cout << "  5. Them benh nhan moi\n";
            std::cout << "  6. Cap nhat thong tin benh nhan\n";
            std::cout << "  7. Xoa benh nhan\n";
            std::cout << "  0. Đăng xuất\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        void DisplayHelper::printAdminMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << YELLOW << "MENU QUAN TRI" << RESET << " - Xin chao, Admin " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Danh sách bác sĩ\n";
            std::cout << "  2. Tìm kiếm bác sĩ\n";
            std::cout << "  3. Them bac si moi\n";
            std::cout << "  4. Cap nhat thong tin bac si\n";
            std::cout << "  5. Xoa bac si\n";
            std::cout << "  6. Xem thong ke he thong\n";
            std::cout << "  7. Xem báo cáo\n";
            std::cout << "  0. Đăng xuất\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        // ==================== Input Prompts ====================

        std::string DisplayHelper::getInput(const std::string &prompt)
        {
            std::cout << CYAN << prompt << ": " << RESET;
            std::string input;
            std::getline(std::cin, input);
            return Utils::trim(input);
        }

        int DisplayHelper::getIntInput(const std::string &prompt, int min, int max)
        {
            while (true)
            {
                std::string input = getInput(prompt);
                if (input.empty())
                {
                    return -1;
                }

                try
                {
                    int value = std::stoi(input);
                    if (value >= min && value <= max)
                    {
                        return value;
                    }
                    printError("Gia tri phai trong khoang " + std::to_string(min) + " den " + std::to_string(max));
                }
                catch (...)
                {
                    printError("Vui long nhap so nguyen hop le!");
                }
            }
        }

        double DisplayHelper::getDoubleInput(const std::string &prompt)
        {
            while (true)
            {
                std::string input = getInput(prompt);
                if (input.empty())
                {
                    return -1.0;
                }

                try
                {
                    double value = std::stod(input);
                    if (value >= 0.0)
                    {
                        return value;
                    }
                    printError("Gia tri phai lon hon hoac bang 0!");
                }
                catch (...)
                {
                    printError("Vui long nhap so thuc hop le!");
                }
            }
        }

        bool DisplayHelper::confirm(const std::string &message)
        {
            std::cout << YELLOW << message << " (y/n): " << RESET;
            std::string input;
            std::getline(std::cin, input);
            input = Utils::toLower(Utils::trim(input));
            return input == "y" || input == "yes";
        }

        std::string DisplayHelper::getPasswordInput(const std::string &prompt)
        {
            std::cout << CYAN << prompt << ": " << RESET;
            std::string password;

#ifdef _WIN32
            // Windows: Use _getch() to hide input
            char ch;
            while ((ch = _getch()) != '\r')
            { // '\r' is Enter key
                if (ch == '\b' && !password.empty())
                { // Backspace
                    password.pop_back();
                    std::cout << "\b \b"; // Erase character from console
                }
                else if (ch != '\b')
                {
                    password += ch;
                    std::cout << '*';
                }
            }
            std::cout << "\n";
#else
            // Unix/Linux: Disable echo
            termios oldSettings, newSettings;
            tcgetattr(STDIN_FILENO, &oldSettings);
            newSettings = oldSettings;
            newSettings.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &newSettings);

            std::getline(std::cin, password);

            tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
            std::cout << "\n";
#endif

            return Utils::trim(password);
        }

        // ==================== Entity Display ====================

        void DisplayHelper::printPatientInfo(const Model::Patient &patient)
        {
            printSubHeader("THONG TIN BENH NHAN");
            std::cout << std::left;
            std::cout << std::setw(20) << "Ma benh nhan:" << patient.getPatientID() << "\n";
            std::cout << std::setw(20) << "Ten:" << patient.getName() << "\n";
            std::cout << std::setw(20) << "So dien thoai:" << patient.getPhone() << "\n";
            std::cout << std::setw(20) << "Gioi tinh:" << genderToString(patient.getGender()) << "\n";
            std::cout << std::setw(20) << "Ngay sinh:" << formatDate(patient.getDateOfBirth()) << "\n";
            std::cout << std::setw(20) << "Dia chi:" << patient.getAddress() << "\n";
            if (!patient.getMedicalHistory().empty())
            {
                std::cout << std::setw(20) << "Tien su benh:" << patient.getMedicalHistory() << "\n";
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printDoctorInfo(const Model::Doctor &doctor)
        {
            printSubHeader("THONG TIN BAC SI");
            std::cout << std::left;
            std::cout << std::setw(20) << "Ma bac si:" << doctor.getDoctorID() << "\n";
            std::cout << std::setw(20) << "Ten:" << doctor.getName() << "\n";
            std::cout << std::setw(20) << "So dien thoai:" << doctor.getPhone() << "\n";
            std::cout << std::setw(20) << "Gioi tinh:" << genderToString(doctor.getGender()) << "\n";
            std::cout << std::setw(20) << "Ngay sinh:" << formatDate(doctor.getDateOfBirth()) << "\n";
            std::cout << std::setw(20) << "Chuyen khoa:" << doctor.getSpecialization() << "\n";
            std::cout << std::setw(20) << "Lich lam viec:" << doctor.getSchedule() << "\n";
            std::cout << std::setw(20) << "Phi kham:" << formatMoney(doctor.getConsultationFee()) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printAppointmentInfo(const Model::Appointment &appointment)
        {
            printSubHeader("THONG TIN LICH HEN");
            std::cout << std::left;
            std::cout << std::setw(20) << "Ma lich hen:" << appointment.getAppointmentID() << "\n";
            std::cout << std::setw(20) << "Ma benh nhan:" << appointment.getPatientUsername() << "\n";
            std::cout << std::setw(20) << "Ma bac si:" << appointment.getDoctorID() << "\n";
            std::cout << std::setw(20) << "Ngay kham:" << formatDate(appointment.getDate()) << "\n";
            std::cout << std::setw(20) << "Gio kham:" << appointment.getTime() << "\n";
            std::cout << std::setw(20) << "Trieu chung:" << appointment.getDisease() << "\n";
            std::cout << std::setw(20) << "Trang thai:" << formatStatus(appointment.getStatus()) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printStatistics(const Model::Statistics &stats)
        {
            printSubHeader("THONG KE HE THONG");
            std::cout << std::left;
            std::cout << std::setw(30) << "Tong so benh nhan:" << stats.totalPatients << "\n";
            std::cout << std::setw(30) << "Tong so bac si:" << stats.totalDoctors << "\n";
            std::cout << std::setw(30) << "Tong so lich hen:" << stats.totalAppointments << "\n";
            std::cout << std::setw(30) << "Lich hen hoan thanh:" << stats.completedAppointments << "\n";
            std::cout << std::setw(30) << "Lich hen bi huy:" << stats.cancelledAppointments << "\n";
            std::cout << std::setw(30) << "Tong doanh thu:" << formatMoney(stats.totalRevenue) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printMedicineInfo(const Model::Medicine &medicine)
        {
            printSubHeader("THONG TIN THUOC");
            std::cout << std::left;
            std::cout << std::setw(25) << "Ma thuoc:" << medicine.getMedicineID() << "\n";
            std::cout << std::setw(25) << "Ten thuoc:" << medicine.getName() << "\n";
            std::cout << std::setw(25) << "Ten khoa hoc:" << medicine.getGenericName() << "\n";
            std::cout << std::setw(25) << "Danh muc:" << medicine.getCategory() << "\n";
            std::cout << std::setw(25) << "Nha san xuat:" << medicine.getManufacturer() << "\n";
            std::cout << std::setw(25) << "Don gia:" << formatMoney(medicine.getUnitPrice()) << "\n";
            std::cout << std::setw(25) << "Ton kho:" << medicine.getQuantityInStock() << "\n";
            std::cout << std::setw(25) << "Muc dat hang lai:" << medicine.getReorderLevel() << "\n";
            std::cout << std::setw(25) << "Ngay het han:" << formatDate(medicine.getExpiryDate()) << "\n";
            std::cout << std::setw(25) << "Dang bao che:" << medicine.getDosageForm() << "\n";
            std::cout << std::setw(25) << "Lieu luong:" << medicine.getStrength() << "\n";
            if (!medicine.getDescription().empty())
            {
                std::cout << std::setw(25) << "Mo ta:" << medicine.getDescription() << "\n";
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printDepartmentInfo(const Model::Department &department)
        {
            printSubHeader("THONG TIN KHOA");
            std::cout << std::left;
            std::cout << std::setw(25) << "Ma khoa:" << department.getDepartmentID() << "\n";
            std::cout << std::setw(25) << "Ten khoa:" << department.getName() << "\n";
            std::cout << std::setw(25) << "Mo ta:" << department.getDescription() << "\n";
            std::cout << std::setw(25) << "Truong khoa:" << department.getHeadDoctorID() << "\n";
            std::cout << std::setw(25) << "Vi tri:" << department.getLocation() << "\n";
            std::cout << std::setw(25) << "So dien thoai:" << department.getPhone() << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printPrescriptionInfo(const Model::Prescription &prescription)
        {
            printSubHeader("THONG TIN DON THUOC");
            std::cout << std::left;
            std::cout << std::setw(25) << "Ma don thuoc:" << prescription.getPrescriptionID() << "\n";
            std::cout << std::setw(25) << "Ma benh nhan:" << prescription.getPatientUsername() << "\n";
            std::cout << std::setw(25) << "Ma bac si:" << prescription.getDoctorID() << "\n";
            std::cout << std::setw(25) << "Ngay ke don:" << formatDate(prescription.getPrescriptionDate()) << "\n";
            std::cout << std::setw(25) << "Chan doan:" << prescription.getDiagnosis() << "\n";
            if (!prescription.getNotes().empty())
            {
                std::cout << std::setw(25) << "Ghi chu:" << prescription.getNotes() << "\n";
            }

            auto items = prescription.getItems();
            if (!items.empty())
            {
                std::cout << "\n";
                printSubHeader("DANH SACH THUOC");
                for (size_t i = 0; i < items.size(); ++i)
                {
                    std::cout << (i + 1) << ". " << items[i].medicineName
                              << " - SL: " << items[i].quantity
                              << " - Lieu: " << items[i].dosage
                              << " - Thoi gian: " << items[i].duration << "\n";
                    if (!items[i].instructions.empty())
                    {
                        std::cout << "   Huong dan: " << items[i].instructions << "\n";
                    }
                }
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printStockAlert(const BLL::StockAlert &alert)
        {
            std::cout << YELLOW << "[CANH BAO TON KHO] " << RESET;
            std::cout << "Thuoc: " << alert.medicineName
                      << " (ID: " << alert.medicineID << ")"
                      << " - Ton kho: " << alert.currentStock
                      << " / Muc toi thieu: " << alert.reorderLevel << "\n";
        }

        void DisplayHelper::printExpiryAlert(const BLL::ExpiryAlert &alert)
        {
            std::cout << RED << "[CANH BAO HET HAN] " << RESET;
            std::cout << "Thuoc: " << alert.medicineName
                      << " (ID: " << alert.medicineID << ")"
                      << " - Het han: " << formatDate(alert.expiryDate)
                      << " - Con: " << alert.daysUntilExpiry << " ngay\n";
        }

        // ==================== Table Display ====================

        void DisplayHelper::printTable(const std::vector<std::string> &headers,
                                       const std::vector<std::vector<std::string>> &rows,
                                       const std::vector<int> &colWidths)
        {
            if (headers.empty())
            {
                return;
            }

            // Calculate column widths if not provided
            std::vector<int> widths = colWidths;
            if (widths.empty())
            {
                widths.resize(headers.size(), 15);
                for (size_t i = 0; i < headers.size(); ++i)
                {
                    widths[i] = std::max(static_cast<int>(headers[i].length()), 10);
                    for (const auto &row : rows)
                    {
                        if (i < row.size())
                        {
                            widths[i] = std::max(widths[i], static_cast<int>(row[i].length()));
                        }
                    }
                    widths[i] = std::min(widths[i], 40); // Max width
                }
            }

            // Calculate total width
            int totalWidth = 1;
            for (int w : widths)
            {
                totalWidth += w + 3;
            }

            // Print top border
            std::cout << BOLD;
            printSeparator(totalWidth, '=');

            // Print headers
            std::cout << BOLD << CYAN << "|";
            for (size_t i = 0; i < headers.size(); ++i)
            {
                std::cout << " " << std::left << std::setw(widths[i]) << truncate(headers[i], widths[i]) << " |";
            }
            std::cout << RESET << "\n";
            printSeparator(totalWidth, '=');

            // Print rows
            for (const auto &row : rows)
            {
                std::cout << "|";
                for (size_t i = 0; i < headers.size(); ++i)
                {
                    std::string cell = (i < row.size()) ? row[i] : "";
                    std::cout << " " << std::left << std::setw(widths[i]) << truncate(cell, widths[i]) << " |";
                }
                std::cout << "\n";
            }

            // Print bottom border
            printSeparator(totalWidth, '=');
        }

        void DisplayHelper::printPatientTable(const std::vector<Model::Patient> &patients)
        {
            if (patients.empty())
            {
                printNoData("benh nhan");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma BN", "Ho ten", "SDT", "Gioi tinh", "Ngay sinh"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < patients.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    patients[i].getPatientID(),
                    patients[i].getName(),
                    patients[i].getPhone(),
                    genderToString(patients[i].getGender()),
                    formatDate(patients[i].getDateOfBirth()),
                });
            }

            std::vector<int> widths = {5, 10, 25, 12, 10, 12};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printDoctorTable(const std::vector<Model::Doctor> &doctors)
        {
            if (doctors.empty())
            {
                printNoData("bac si");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma BS", "Ho ten", "Chuyen khoa", "SDT", "Phi kham"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < doctors.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    doctors[i].getDoctorID(),
                    doctors[i].getName(),
                    doctors[i].getSpecialization(),
                    doctors[i].getPhone(),
                    formatMoney(doctors[i].getConsultationFee()),
                });
            }

            std::vector<int> widths = {5, 10, 25, 20, 12, 15};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printAppointmentTable(const std::vector<Model::Appointment> &appointments)
        {
            if (appointments.empty())
            {
                printNoData("lich hen");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma LH", "Benh nhan", "Bac si", "Ngay", "Gio", "Trang thai"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < appointments.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    appointments[i].getAppointmentID(),
                    appointments[i].getPatientUsername(),
                    appointments[i].getDoctorID(),
                    formatDate(appointments[i].getDate()),
                    appointments[i].getTime(),
                    formatStatus(appointments[i].getStatus()),
                });
            }

            std::vector<int> widths = {5, 10, 15, 10, 12, 8, 12};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printMedicineTable(const std::vector<Model::Medicine> &medicines)
        {
            if (medicines.empty())
            {
                printNoData("thuoc");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma thuoc", "Ten thuoc", "Danh muc", "Gia", "Ton kho", "Het han"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < medicines.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    medicines[i].getMedicineID(),
                    medicines[i].getName(),
                    medicines[i].getCategory(),
                    formatMoney(medicines[i].getUnitPrice()),
                    std::to_string(medicines[i].getQuantityInStock()),
                    formatDate(medicines[i].getExpiryDate()),
                });
            }

            std::vector<int> widths = {5, 10, 25, 15, 12, 10, 12};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printDepartmentTable(const std::vector<Model::Department> &departments)
        {
            if (departments.empty())
            {
                printNoData("khoa");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma khoa", "Ten khoa", "Truong khoa", "Vi tri", "SDT"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < departments.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    departments[i].getDepartmentID(),
                    departments[i].getName(),
                    departments[i].getHeadDoctorID(),
                    departments[i].getLocation(),
                    departments[i].getPhone(),
                });
            }

            std::vector<int> widths = {5, 10, 25, 15, 20, 12};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printPrescriptionTable(const std::vector<Model::Prescription> &prescriptions)
        {
            if (prescriptions.empty())
            {
                printNoData("don thuoc");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma DT", "Benh nhan", "Bac si", "Ngay ke", "Chan doan"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < prescriptions.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    prescriptions[i].getPrescriptionID(),
                    prescriptions[i].getPatientUsername(),
                    prescriptions[i].getDoctorID(),
                    formatDate(prescriptions[i].getPrescriptionDate()),
                    truncate(prescriptions[i].getDiagnosis(), 30),
                });
            }

            std::vector<int> widths = {5, 10, 15, 10, 12, 30};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printStockAlertTable(const std::vector<BLL::StockAlert> &alerts)
        {
            if (alerts.empty())
            {
                printInfo("Khong co canh bao ton kho.");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma thuoc", "Ten thuoc", "Ton kho", "Muc toi thieu"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < alerts.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    alerts[i].medicineID,
                    alerts[i].medicineName,
                    std::to_string(alerts[i].currentStock),
                    std::to_string(alerts[i].reorderLevel),
                });
            }

            std::vector<int> widths = {5, 10, 30, 12, 15};
            paginateTable(headers, rows, widths);
        }

        void DisplayHelper::printExpiryAlertTable(const std::vector<BLL::ExpiryAlert> &alerts)
        {
            if (alerts.empty())
            {
                printInfo("Khong co canh bao het han.");
                return;
            }

            std::vector<std::string> headers = {"STT", "Ma thuoc", "Ten thuoc", "Ngay het han", "Con (ngay)"};
            std::vector<std::vector<std::string>> rows;

            for (size_t i = 0; i < alerts.size(); ++i)
            {
                rows.push_back({
                    std::to_string(i + 1),
                    alerts[i].medicineID,
                    alerts[i].medicineName,
                    formatDate(alerts[i].expiryDate),
                    std::to_string(alerts[i].daysUntilExpiry),
                });
            }

            std::vector<int> widths = {5, 10, 30, 15, 12};
            paginateTable(headers, rows, widths);
        }

        // ==================== List Display (with numbering) ====================

        void DisplayHelper::printPatientList(const std::vector<Model::Patient> &patients)
        {
            if (patients.empty())
            {
                printNoData("benh nhan");
                return;
            }

            printSubHeader("DANH SACH BENH NHAN");
            for (size_t i = 0; i < patients.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(30) << patients[i].getName()
                          << " (ID: " << patients[i].getPatientID()
                          << ", SDT: " << patients[i].getPhone() << ")\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printDoctorList(const std::vector<Model::Doctor> &doctors)
        {
            if (doctors.empty())
            {
                printNoData("bac si");
                return;
            }

            printSubHeader("DANH SACH BAC SI");
            for (size_t i = 0; i < doctors.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(30) << doctors[i].getName()
                          << " - " << std::setw(20) << doctors[i].getSpecialization()
                          << " (ID: " << doctors[i].getDoctorID() << ")\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printAppointmentList(const std::vector<Model::Appointment> &appointments)
        {
            if (appointments.empty())
            {
                printNoData("lich hen");
                return;
            }

            printSubHeader("DANH SACH LICH HEN");
            for (size_t i = 0; i < appointments.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << formatDate(appointments[i].getDate()) << " "
                          << appointments[i].getTime()
                          << " - BS: " << appointments[i].getDoctorID()
                          << " - " << formatStatus(appointments[i].getStatus())
                          << "\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printTimeSlotList(const std::vector<std::string> &slots)
        {
            if (slots.empty())
            {
                printNoData("khung gio trong");
                return;
            }

            printSubHeader("KHUNG GIO TRONG");
            int count = 0;
            for (size_t i = 0; i < slots.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". " << slots[i] << "    ";
                count++;
                if (count % 4 == 0)
                {
                    std::cout << "\n";
                }
            }
            if (count % 4 != 0)
            {
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printMedicineList(const std::vector<Model::Medicine> &medicines)
        {
            if (medicines.empty())
            {
                printNoData("thuoc");
                return;
            }

            printSubHeader("DANH SACH THUOC");
            for (size_t i = 0; i < medicines.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(35) << medicines[i].getName()
                          << " - " << std::setw(15) << medicines[i].getCategory()
                          << " (ID: " << medicines[i].getMedicineID()
                          << ", Ton: " << medicines[i].getQuantityInStock() << ")\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printDepartmentList(const std::vector<Model::Department> &departments)
        {
            if (departments.empty())
            {
                printNoData("khoa");
                return;
            }

            printSubHeader("DANH SACH KHOA");
            for (size_t i = 0; i < departments.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(30) << departments[i].getName()
                          << " (ID: " << departments[i].getDepartmentID()
                          << ", Vi tri: " << departments[i].getLocation() << ")\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printPrescriptionList(const std::vector<Model::Prescription> &prescriptions)
        {
            if (prescriptions.empty())
            {
                printNoData("don thuoc");
                return;
            }

            printSubHeader("DANH SACH DON THUOC");
            for (size_t i = 0; i < prescriptions.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << "Ngay: " << formatDate(prescriptions[i].getPrescriptionDate())
                          << " - BS: " << prescriptions[i].getDoctorID()
                          << " - Chan doan: " << truncate(prescriptions[i].getDiagnosis(), 40)
                          << "\n";
            }
            std::cout << "\n";
        }

        // ==================== Menu Display (Advanced Features) ====================

        void DisplayHelper::printMedicineMenu()
        {
            printHeader("QUAN LY THUOC");
            std::cout << "  1. Xem danh sach thuoc\n";
            std::cout << "  2. Tìm kiếm thuốc\n";
            std::cout << "  3. Them thuoc moi\n";
            std::cout << "  4. Cap nhat thong tin thuoc\n";
            std::cout << "  5. Xoa thuoc\n";
            std::cout << "  6. Nhap/Xuat ton kho\n";
            std::cout << "  7. Canh bao ton kho thap\n";
            std::cout << "  8. Canh bao het han\n";
            std::cout << "  0. Quay lai\n";
            std::cout << "\n";
        }

        void DisplayHelper::printDepartmentMenu()
        {
            printHeader("QUAN LY KHOA");
            std::cout << "  1. Xem danh sach khoa\n";
            std::cout << "  2. Them khoa moi\n";
            std::cout << "  3. Cap nhat thong tin khoa\n";
            std::cout << "  4. Xoa khoa\n";
            std::cout << "  5. Phan cong bac si vao khoa\n";
            std::cout << "  6. Xem thong ke khoa\n";
            std::cout << "  0. Quay lai\n";
            std::cout << "\n";
        }

        void DisplayHelper::printPrescriptionMenu()
        {
            printHeader("QUAN LY DON THUOC");
            std::cout << "  1. Ke don thuoc moi\n";
            std::cout << "  2. Xem don thuoc\n";
            std::cout << "  3. Cap phat thuoc theo don\n";
            std::cout << "  4. In don thuoc\n";
            std::cout << "  0. Quay lai\n";
            std::cout << "\n";
        }

        void DisplayHelper::printReportMenu()
        {
            printHeader("TAO BAO CAO");
            std::cout << "  1. Bao cao ngay\n";
            std::cout << "  2. Bao cao tuan\n";
            std::cout << "  3. Bao cao thang\n";
            std::cout << "  4. Bao cao doanh thu\n";
            std::cout << "  5. Xuat bao cao ra file\n";
            std::cout << "  0. Quay lai\n";
            std::cout << "\n";
        }

        // ==================== Messages ====================

        void DisplayHelper::printSuccess(const std::string &message)
        {
            std::cout << GREEN << "[THANH CONG] " << message << RESET << "\n";
        }

        void DisplayHelper::printError(const std::string &message)
        {
            std::cout << RED << "[LOI] " << message << RESET << "\n";
        }

        void DisplayHelper::printWarning(const std::string &message)
        {
            std::cout << YELLOW << "[CANH BAO] " << message << RESET << "\n";
        }

        void DisplayHelper::printInfo(const std::string &message)
        {
            std::cout << CYAN << "[THONG TIN] " << message << RESET << "\n";
        }

        void DisplayHelper::printNoData(const std::string &entityType)
        {
            std::cout << YELLOW << "Khong tim thay " << entityType << " nao." << RESET << "\n";
        }

        // ==================== Formatting Utilities ====================

        std::string DisplayHelper::formatMoney(double amount)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(0);
            oss << amount;
            std::string result = oss.str();

            // Add thousand separators
            int pos = result.length() - 3;
            while (pos > 0)
            {
                result.insert(pos, ",");
                pos -= 3;
            }

            return result + " VND";
        }

        std::string DisplayHelper::formatDate(const std::string &date)
        {
            if (date.empty() || date.length() != 10)
            {
                return date;
            }

            // Convert YYYY-MM-DD to DD/MM/YYYY
            std::string day = date.substr(8, 2);
            std::string month = date.substr(5, 2);
            std::string year = date.substr(0, 4);

            // Month names in Vietnamese
            static const char *months[] = {
                "Thg 1", "Thg 2", "Thg 3", "Thg 4", "Thg 5", "Thg 6",
                "Thg 7", "Thg 8", "Thg 9", "Thg 10", "Thg 11", "Thg 12"};

            try
            {
                int monthNum = std::stoi(month);
                if (monthNum >= 1 && monthNum <= 12)
                {
                    return day + " " + months[monthNum - 1] + " " + year;
                }
            }
            catch (...)
            {
            }

            return day + "/" + month + "/" + year;
        }

        std::string DisplayHelper::formatStatus(AppointmentStatus status)
        {
            switch (status)
            {
            case AppointmentStatus::SCHEDULED:
                return std::string(CYAN) + "Da dat lich" + RESET;
            case AppointmentStatus::COMPLETED:
                return std::string(GREEN) + "Hoan thanh" + RESET;
            case AppointmentStatus::CANCELLED:
                return std::string(RED) + "Da huy" + RESET;
            case AppointmentStatus::NO_SHOW:
                return std::string(YELLOW) + "Vang mat" + RESET;
            default:
                return "Khong xac dinh";
            }
        }

        std::string DisplayHelper::truncate(const std::string &str, size_t maxLength)
        {
            if (str.length() <= maxLength)
            {
                return str;
            }
            return str.substr(0, maxLength - 3) + "...";
        }

        std::string DisplayHelper::pad(const std::string &str, size_t width, bool leftAlign)
        {
            if (str.length() >= width)
            {
                return str.substr(0, width);
            }

            size_t padding = width - str.length();
            if (leftAlign)
            {
                return str + std::string(padding, ' ');
            }
            else
            {
                return std::string(padding, ' ') + str;
            }
        }

    } // namespace UI
} // namespace HMS
