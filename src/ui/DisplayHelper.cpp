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
            constexpr int DEFAULT_PAGE_SIZE = 40; // Items per page
            constexpr int DEFAULT_COL_WIDTH = 60; // Default column width
        }

        // ==================== Pagination Helper ====================
        namespace
        {
            // Local color constants for helper functions
            constexpr const char *RESET_LOCAL = "\033[0m";
            constexpr const char *RED_LOCAL = "\033[31m";
            constexpr const char *GREEN_LOCAL = "\033[32m";
            constexpr const char *YELLOW_LOCAL = "\033[33m";
            constexpr const char *BLUE_LOCAL = "\033[34m";
            constexpr const char *CYAN_LOCAL = "\033[36m";
            constexpr const char *BOLD_LOCAL = "\033[1m";

            int getVisibleLength(const std::string &str) {
                int length = 0;
                bool inAnsi = false;
                for (size_t i = 0; i < str.length(); ++i)
                {
                    unsigned char c = static_cast<unsigned char>(str[i]);
                    if (c == 0x1B) {
                        inAnsi = true;
                        continue;
                    }
                    if (inAnsi) {
                        if (isalpha(c)) inAnsi = false;
                        continue;
                    }
                    if ((c & 0xC0) != 0x80) {
                        length++;
                    }
                }
                return length;
            }

            std::vector<std::string> wrapText(const std::string &text, int width)
            {
                std::vector<std::string> lines;
                if (text.empty()) { lines.push_back(""); return lines; }

                std::istringstream words(text);
                std::string word;
                std::string currentLine;

                while (words >> word)
                {
                    int wordLen = getVisibleLength(word);
                    int currentLen = getVisibleLength(currentLine);

                    if (wordLen > width) {
                        if (!currentLine.empty()) {
                            lines.push_back(currentLine);
                            currentLine = "";
                        }
                        lines.push_back(word);
                    }
                    else if (currentLen + (currentLen > 0 ? 1 : 0) + wordLen > width) {
                        lines.push_back(currentLine);
                        currentLine = word;
                    }
                    else {
                        currentLine += (currentLine.empty() ? "" : " ") + word;
                    }
                }
                if (!currentLine.empty()) lines.push_back(currentLine);
                return lines;
            }

            void printTableInternal(const std::vector<std::string> &headers,
                                    const std::vector<std::vector<std::string>> &rows,
                                    const std::vector<int> &colWidths) {

                if (headers.empty()) return;
                std::vector<int> widths = colWidths;
                if (widths.empty()) widths.resize(headers.size(), 15);

                for (size_t i = 0; i < headers.size(); ++i) {
                    widths[i] = std::max(widths[i], getVisibleLength(headers[i]));
                    for (const auto &row : rows) {
                        if (i < row.size()) {
                            int contentLen = getVisibleLength(row[i]);
                            widths[i] = std::max(widths[i], std::min(contentLen, 40));
                        }
                    }
                }

                int totalWidth = 1;
                for (int w : widths) totalWidth += w + 3;

                std::cout << BOLD_LOCAL;
                DisplayHelper::printSeparator(totalWidth, '=');
                std::cout << CYAN_LOCAL << "|";
                for (size_t i = 0; i < headers.size(); ++i) {
                    int visibleLen = getVisibleLength(headers[i]);
                    int padding = widths[i] - visibleLen;
                    if (padding < 0) padding = 0;
                    std::cout << " " << headers[i] << std::string(padding, ' ') << " |";
                }
                std::cout << RESET_LOCAL << "\n";
                DisplayHelper::printSeparator(totalWidth, '=');

                for (const auto &row : rows) {
                    std::vector<std::vector<std::string>> rowBlocks;
                    size_t maxHeight = 1;

                    for (size_t i = 0; i < headers.size(); ++i) {
                        std::string cellText = (i < row.size()) ? row[i] : "";
                        std::vector<std::string> lines = wrapText(cellText, widths[i]);
                        rowBlocks.push_back(lines);
                        maxHeight = std::max(maxHeight, lines.size());
                    }

                    for (size_t h = 0; h < maxHeight; ++h) {
                        std::cout << "|";
                        for (size_t col = 0; col < headers.size(); ++col) {
                            std::string textToPrint = "";
                            if (h < rowBlocks[col].size()) textToPrint = rowBlocks[col][h];

                            int visibleLen = getVisibleLength(textToPrint);
                            int padding = widths[col] - visibleLen;
                            if (padding < 0) padding = 0;

                            std::cout << " " << textToPrint << std::string(padding, ' ') << " |";
                        }
                        std::cout << "\n";
                    }
                    DisplayHelper::printSeparator(totalWidth, '-');
                }
                std::cout << "\r";
                DisplayHelper::printSeparator(totalWidth, '=');
            }

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
                    std::cout << "Không có dữ liệu để hiển thị.\n";
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
                    printTableInternal(headers, pageRows, colWidths);

                    // Print pagination info
                    std::cout << "\n";
                    DisplayHelper::printSeparator(80, '-');
                    std::cout << CYAN_LOCAL << "Trang " << currentPage << "/" << totalPages
                              << " (Hiển thị " << startIdx + 1 << "-" << endIdx
                              << " / " << totalRows << " mục)" << RESET_LOCAL << "\n";
                    DisplayHelper::printSeparator(80, '-');

                    // Print navigation menu
                    std::cout << "\n";
                    if (currentPage > 1)
                        std::cout << "[P] Trang trước  ";
                    if (currentPage < totalPages)
                        std::cout << "[N] Trang sau  ";
                    if (totalPages > 1)
                        std::cout << "[G] Đến trang  ";
                    std::cout << "[Q] Quay lại menu\n";
                    std::cout << "\nNhập lựa chọn: ";

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
                            std::cout << YELLOW_LOCAL << "Đây là trang cuối cùng!" << RESET_LOCAL << "\n";
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
                            std::cout << YELLOW_LOCAL << "Đây là trang đầu tiên!" << RESET_LOCAL << "\n";
                            DisplayHelper::pause();
                        }
                    }
                    else if (choice == "g" || choice == "goto")
                    {
                        std::cout << "Nhập số trang (1-" << totalPages << "): ";
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
                                std::cout << YELLOW_LOCAL << "Số trang không hợp lệ!" << RESET_LOCAL << "\n";
                                DisplayHelper::pause();
                            }
                        }
                        catch (...)
                        {
                            std::cout << RED_LOCAL << "Vui lòng nhập số hợp lệ!" << RESET_LOCAL << "\n";
                            DisplayHelper::pause();
                        }
                    }
                    else
                    {
                        std::cout << YELLOW_LOCAL << "Lựa chọn không hợp lệ!" << RESET_LOCAL << "\n";
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
            std::cout << "\nNhấn Enter để tiếp tục...";
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
            std::cout << "            HỆ THỐNG QUẢN LÝ BỆNH VIỆN\n";
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
            std::cout << "  0. Quay lại\n";
            std::cout << "\n";
        }

        void DisplayHelper::printMainMenu()
        {
            printAppHeader();
            std::cout << BOLD << "MENU CHÍNH\n"
                      << RESET;
            printThinSeparator(40);
            std::cout << "  1. Đăng nhập\n";
            std::cout << "  2. Đăng ký (Bệnh nhân)\n";
            std::cout << "  0. Thoát\n";
            printThinSeparator(40);
            std::cout << "\n";
        }

        void DisplayHelper::printPatientMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << GREEN << "MENU BỆNH NHÂN" << RESET << " - Xin chào, " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Xem thông tin cá nhân\n";
            std::cout << "  2. Đặt lịch khám\n";
            std::cout << "  3. Xem lịch sử khám bệnh\n";
            std::cout << "  4. Xem lịch hẹn sắp tới\n";
            std::cout << "  5. Hủy lịch hẹn\n";
            std::cout << "  6. Xem tổng hóa đơn\n";
            std::cout << "  0. Đăng xuất\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        void DisplayHelper::printDoctorMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << BLUE << "MENU BÁC SĨ" << RESET << " - Xin chào, Dr. " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Xem lịch làm việc\n";
            std::cout << "  2. Đánh dấu lịch hẹn hoàn thành\n";
            std::cout << "  3. Danh sách bệnh nhân\n";
            std::cout << "  4. Tìm kiếm bệnh nhân\n";
            std::cout << "  5. Thêm bệnh nhân mới\n";
            std::cout << "  6. Cập nhật thông tin bệnh nhân\n";
            std::cout << "  7. Xóa bệnh nhân\n";
            std::cout << "  0. Đăng xuất\n";
            printThinSeparator(60);
            std::cout << "\n";
        }

        void DisplayHelper::printAdminMenu(const std::string &username)
        {
            printAppHeader();
            std::cout << BOLD << YELLOW << "MENU QUẢN TRỊ" << RESET << " - Xin chào, Admin " << BOLD << username << RESET << "\n";
            printThinSeparator(60);
            std::cout << "  1. Danh sách bác sĩ\n";
            std::cout << "  2. Tìm kiếm bác sĩ\n";
            std::cout << "  3. Thêm bác sĩ mới\n";
            std::cout << "  4. Cập nhật thông tin bác sĩ\n";
            std::cout << "  5. Xóa bác sĩ\n";
            std::cout << "  6. Xem thống kê hệ thống\n";
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
                    printError("Giá trị phải trong khoảng " + std::to_string(min) + " đến " + std::to_string(max));
                }
                catch (...)
                {
                    printError("Vui lòng nhập số nguyên hợp lệ!");
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
                    printError("Giá trị phải lớn hơn hoặc bằng 0!");
                }
                catch (...)
                {
                    printError("Vui lòng nhập số thực hợp lệ!");
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
            printSubHeader("THÔNG TIN BỆNH NHÂN");
            std::cout << std::left;
            std::cout << std::setw(20) << "Mã bệnh nhân:" << patient.getPatientID() << "\n";
            std::cout << std::setw(20) << "Tên:" << patient.getName() << "\n";
            std::cout << std::setw(20) << "Số điện thoại:" << patient.getPhone() << "\n";
            std::cout << std::setw(20) << "Giới tính:" << genderToString(patient.getGender()) << "\n";
            std::cout << std::setw(20) << "Ngày sinh:" << formatDate(patient.getDateOfBirth()) << "\n";
            std::cout << std::setw(20) << "Địa chỉ:" << patient.getAddress() << "\n";
            if (!patient.getMedicalHistory().empty())
            {
                std::cout << std::setw(20) << "Tiền sử bệnh:" << patient.getMedicalHistory() << "\n";
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printDoctorInfo(const Model::Doctor &doctor)
        {
            printSubHeader("THÔNG TIN BÁC SĨ");
            std::cout << std::left;
            std::cout << std::setw(20) << "Mã bác sĩ:" << doctor.getDoctorID() << "\n";
            std::cout << std::setw(20) << "Tên:" << doctor.getName() << "\n";
            std::cout << std::setw(20) << "Số điện thoại:" << doctor.getPhone() << "\n";
            std::cout << std::setw(20) << "Giới tính:" << genderToString(doctor.getGender()) << "\n";
            std::cout << std::setw(20) << "Ngày sinh:" << formatDate(doctor.getDateOfBirth()) << "\n";
            std::cout << std::setw(20) << "Chuyên khoa:" << doctor.getSpecialization() << "\n";
            std::cout << std::setw(20) << "Lịch làm việc:" << doctor.getSchedule() << "\n";
            std::cout << std::setw(20) << "Phí khám:" << formatMoney(doctor.getConsultationFee()) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printAppointmentInfo(const Model::Appointment &appointment)
        {
            printSubHeader("THÔNG TIN LỊCH HẸN");
            std::cout << std::left;
            std::cout << std::setw(20) << "Mã lịch hẹn:" << appointment.getAppointmentID() << "\n";
            std::cout << std::setw(20) << "Mã bệnh nhân:" << appointment.getPatientUsername() << "\n";
            std::cout << std::setw(20) << "Mã bác sĩ:" << appointment.getDoctorID() << "\n";
            std::cout << std::setw(20) << "Ngày khám:" << formatDate(appointment.getDate()) << "\n";
            std::cout << std::setw(20) << "Giờ khám:" << appointment.getTime() << "\n";
            std::cout << std::setw(20) << "Triệu chứng:" << appointment.getDisease() << "\n";
            std::cout << std::setw(20) << "Trạng thái:" << formatStatus(appointment.getStatus()) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printStatistics(const Model::Statistics &stats)
        {
            printSubHeader("THỐNG KÊ HỆ THỐNG");
            std::cout << std::left;
            std::cout << std::setw(30) << "Tổng số bệnh nhân:" << stats.totalPatients << "\n";
            std::cout << std::setw(30) << "Tổng số bác sĩ:" << stats.totalDoctors << "\n";
            std::cout << std::setw(30) << "Tổng số lịch hẹn:" << stats.totalAppointments << "\n";
            std::cout << std::setw(30) << "Lịch hẹn hoàn thành:" << stats.completedAppointments << "\n";
            std::cout << std::setw(30) << "Lịch hẹn bị hủy:" << stats.cancelledAppointments << "\n";
            std::cout << std::setw(30) << "Tổng doanh thu:" << formatMoney(stats.totalRevenue) << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printMedicineInfo(const Model::Medicine &medicine)
        {
            printSubHeader("THÔNG TIN THUỐC");
            std::cout << std::left;
            std::cout << std::setw(25) << "Mã thuốc:" << medicine.getMedicineID() << "\n";
            std::cout << std::setw(25) << "Tên thuốc:" << medicine.getName() << "\n";
            std::cout << std::setw(25) << "Tên khoa học:" << medicine.getGenericName() << "\n";
            std::cout << std::setw(25) << "Danh mục:" << medicine.getCategory() << "\n";
            std::cout << std::setw(25) << "Nhà sản xuất:" << medicine.getManufacturer() << "\n";
            std::cout << std::setw(25) << "Đơn giá:" << formatMoney(medicine.getUnitPrice()) << "\n";
            std::cout << std::setw(25) << "Tồn kho:" << medicine.getQuantityInStock() << "\n";
            std::cout << std::setw(25) << "Mức đặt hàng lại:" << medicine.getReorderLevel() << "\n";
            std::cout << std::setw(25) << "Ngày hết hạn:" << formatDate(medicine.getExpiryDate()) << "\n";
            std::cout << std::setw(25) << "Dạng bào chế:" << medicine.getDosageForm() << "\n";
            std::cout << std::setw(25) << "Liều lượng:" << medicine.getStrength() << "\n";
            if (!medicine.getDescription().empty())
            {
                std::cout << std::setw(25) << "Mô tả:" << medicine.getDescription() << "\n";
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printDepartmentInfo(const Model::Department &department)
        {
            printSubHeader("THÔNG TIN KHOA");
            std::cout << std::left;
            std::cout << std::setw(25) << "Mã khoa:" << department.getDepartmentID() << "\n";
            std::cout << std::setw(25) << "Tên khoa:" << department.getName() << "\n";
            std::cout << std::setw(25) << "Mô tả:" << department.getDescription() << "\n";
            std::cout << std::setw(25) << "Trưởng khoa:" << department.getHeadDoctorID() << "\n";
            std::cout << std::setw(25) << "Vị trí:" << department.getLocation() << "\n";
            std::cout << std::setw(25) << "Số điện thoại:" << department.getPhone() << "\n";
            printThinSeparator(60);
        }

        void DisplayHelper::printPrescriptionInfo(const Model::Prescription &prescription)
        {
            printSubHeader("THÔNG TIN ĐƠN THUỐC");
            std::cout << std::left;
            std::cout << std::setw(25) << "Mã đơn thuốc:" << prescription.getPrescriptionID() << "\n";
            std::cout << std::setw(25) << "Mã bệnh nhân:" << prescription.getPatientUsername() << "\n";
            std::cout << std::setw(25) << "Mã bác sĩ:" << prescription.getDoctorID() << "\n";
            std::cout << std::setw(25) << "Ngày kê đơn:" << formatDate(prescription.getPrescriptionDate()) << "\n";
            std::cout << std::setw(25) << "Chẩn đoán:" << prescription.getDiagnosis() << "\n";
            if (!prescription.getNotes().empty())
            {
                std::cout << std::setw(25) << "Ghi chú:" << prescription.getNotes() << "\n";
            }

            auto items = prescription.getItems();
            if (!items.empty())
            {
                std::cout << "\n";
                printSubHeader("DANH SÁCH THUỐC");
                for (size_t i = 0; i < items.size(); ++i)
                {
                    std::cout << (i + 1) << ". " << items[i].medicineName
                              << " - SL: " << items[i].quantity
                              << " - Liều: " << items[i].dosage
                              << " - Thời gian: " << items[i].duration << "\n";
                    if (!items[i].instructions.empty())
                    {
                        std::cout << "   Hướng dẫn: " << items[i].instructions << "\n";
                    }
                }
            }
            printThinSeparator(60);
        }

        void DisplayHelper::printStockAlert(const BLL::StockAlert &alert)
        {
            std::cout << YELLOW << "[CẢNH BÁO TỒN KHO] " << RESET;
            std::cout << "Thuốc: " << alert.medicineName
                      << " (ID: " << alert.medicineID << ")"
                      << " - Tồn kho: " << alert.currentStock
                      << " / Mức tối thiểu: " << alert.reorderLevel << "\n";
        }

        void DisplayHelper::printExpiryAlert(const BLL::ExpiryAlert &alert)
        {
            std::cout << RED << "[CẢNH BÁO HẾT HẠN] " << RESET;
            std::cout << "Thuốc: " << alert.medicineName
                      << " (ID: " << alert.medicineID << ")"
                      << " - Hết hạn: " << formatDate(alert.expiryDate)
                      << " - Còn: " << alert.daysUntilExpiry << " ngày\n";
        }

        // ==================== Table Display ====================

        void DisplayHelper::printTable(const std::vector<std::string> &headers,
                                       const std::vector<std::vector<std::string>> &rows,
                                       const std::vector<int> &colWidths)
        {
           printTableInternal(headers, rows, colWidths);
        }

        void DisplayHelper::printPatientTable(const std::vector<Model::Patient> &patients)
        {
            if (patients.empty())
            {
                printNoData("bệnh nhân");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã BN", "Họ tên", "SĐT", "Giới tính", "Ngày sinh"};
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
                printNoData("bác sĩ");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã BS", "Họ tên", "Chuyên khoa", "SĐT", "Phí khám"};
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
                printNoData("lịch hẹn");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã LH", "Bệnh nhân", "Bác sĩ", "Ngày", "Giờ", "Trạng thái"};
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
                printNoData("thuốc");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã thuốc", "Tên thuốc", "Danh mục", "Giá", "Tồn kho", "Hết hạn"};
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

            std::vector<std::string> headers = {"STT", "Mã khoa", "Tên khoa", "Trưởng khoa", "Vị trí", "SĐT"};
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
                printNoData("đơn thuốc");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã ĐT", "Bệnh nhân", "Bác sĩ", "Ngày kê", "Chẩn đoán"};
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
                printInfo("Không có cảnh báo tồn kho.");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã thuốc", "Tên thuốc", "Tồn kho", "Mức tối thiểu"};
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
                printInfo("Không có cảnh báo hết hạn.");
                return;
            }

            std::vector<std::string> headers = {"STT", "Mã thuốc", "Tên thuốc", "Ngày hết hạn", "Còn (ngày)"};
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
                printNoData("bệnh nhân");
                return;
            }

            printSubHeader("DANH SÁCH BỆNH NHÂN");
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
                printNoData("bác sĩ");
                return;
            }

            printSubHeader("DANH SÁCH BÁC SĨ");
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
                printNoData("lịch hẹn");
                return;
            }

            printSubHeader("DANH SÁCH LỊCH HẸN");
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
                printNoData("khung giờ trống");
                return;
            }

            printSubHeader("KHUNG GIỜ TRỐNG");
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
                printNoData("thuốc");
                return;
            }

            printSubHeader("DANH SÁCH THUỐC");
            for (size_t i = 0; i < medicines.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(35) << medicines[i].getName()
                          << " - " << std::setw(15) << medicines[i].getCategory()
                          << " (ID: " << medicines[i].getMedicineID()
                          << ", Tồn: " << medicines[i].getQuantityInStock() << ")\n";
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

            printSubHeader("DANH SÁCH KHOA");
            for (size_t i = 0; i < departments.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << std::left << std::setw(30) << departments[i].getName()
                          << " (ID: " << departments[i].getDepartmentID()
                          << ", Vị trí: " << departments[i].getLocation() << ")\n";
            }
            std::cout << "\n";
        }

        void DisplayHelper::printPrescriptionList(const std::vector<Model::Prescription> &prescriptions)
        {
            if (prescriptions.empty())
            {
                printNoData("đơn thuốc");
                return;
            }

            printSubHeader("DANH SÁCH ĐƠN THUỐC");
            for (size_t i = 0; i < prescriptions.size(); ++i)
            {
                std::cout << "  " << (i + 1) << ". "
                          << "Ngày: " << formatDate(prescriptions[i].getPrescriptionDate())
                          << " - BS: " << prescriptions[i].getDoctorID()
                          << " - Chẩn đoán: " << truncate(prescriptions[i].getDiagnosis(), 40)
                          << "\n";
            }
            std::cout << "\n";
        }

        // ==================== Menu Display (Advanced Features) ====================

        void DisplayHelper::printMedicineMenu()
        {
            printHeader("QUẢN LÝ THUỐC");
            std::cout << "  1. Xem danh sách thuốc\n";
            std::cout << "  2. Tìm kiếm thuốc\n";
            std::cout << "  3. Thêm thuốc mới\n";
            std::cout << "  4. Cập nhật thông tin thuốc\n";
            std::cout << "  5. Xóa thuốc\n";
            std::cout << "  6. Nhập/Xuất tồn kho\n";
            std::cout << "  7. Cảnh báo tồn kho thấp\n";
            std::cout << "  8. Cảnh báo hết hạn\n";
            std::cout << "  0. Quay lại\n";
            std::cout << "\n";
        }

        void DisplayHelper::printDepartmentMenu()
        {
            printHeader("QUẢN LÝ KHOA");
            std::cout << "  1. Xem danh sách khoa\n";
            std::cout << "  2. Thêm khoa mới\n";
            std::cout << "  3. Cập nhật thông tin khoa\n";
            std::cout << "  4. Xóa khoa\n";
            std::cout << "  5. Phân công bác sĩ vào khoa\n";
            std::cout << "  6. Xem thống kê khoa\n";
            std::cout << "  0. Quay lại\n";
            std::cout << "\n";
        }

        void DisplayHelper::printPrescriptionMenu()
        {
            printHeader("QUẢN LÝ ĐƠN THUỐC");
            std::cout << "  1. Kê đơn thuốc mới\n";
            std::cout << "  2. Xem đơn thuốc\n";
            std::cout << "  3. Cấp phát thuốc theo đơn\n";
            std::cout << "  4. In đơn thuốc\n";
            std::cout << "  0. Quay lại\n";
            std::cout << "\n";
        }

        void DisplayHelper::printReportMenu()
        {
            printHeader("TẠO BÁO CÁO");
            std::cout << "  1. Báo cáo ngày\n";
            std::cout << "  2. Báo cáo tuần\n";
            std::cout << "  3. Báo cáo tháng\n";
            std::cout << "  4. Báo cáo doanh thu\n";
            std::cout << "  5. Xuất báo cáo ra file\n";
            std::cout << "  0. Quay lại\n";
            std::cout << "\n";
        }

        // ==================== Messages ====================

        void DisplayHelper::printSuccess(const std::string &message)
        {
            std::cout << GREEN << "[THÀNH CÔNG] " << message << RESET << "\n";
        }

        void DisplayHelper::printError(const std::string &message)
        {
            std::cout << RED << "[LỖI] " << message << RESET << "\n";
        }

        void DisplayHelper::printWarning(const std::string &message)
        {
            std::cout << YELLOW << "[CẢNH BÁO] " << message << RESET << "\n";
        }

        void DisplayHelper::printInfo(const std::string &message)
        {
            std::cout << CYAN << "[THÔNG TIN] " << message << RESET << "\n";
        }

        void DisplayHelper::printNoData(const std::string &entityType)
        {
            std::cout << YELLOW << "Không tìm thấy " << entityType << " nào." << RESET << "\n";
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
                return std::string(CYAN) + "Đã đặt lịch" + RESET;
            case AppointmentStatus::COMPLETED:
                return std::string(GREEN) + "Hoàn thành" + RESET;
            case AppointmentStatus::CANCELLED:
                return std::string(RED) + "Đã hủy" + RESET;
            case AppointmentStatus::NO_SHOW:
                return std::string(YELLOW) + "Vắng mặt" + RESET;
            default:
                return "Không xác định";
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
