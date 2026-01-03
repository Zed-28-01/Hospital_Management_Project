#include "ui/ConsoleUI.h"
#include "common/Utils.h"
#include <ctime>
#include <cstdio>

namespace HMS
{
    namespace UI
    {

        // ===================== CONSTRUCTOR ===================================
        ConsoleUI::ConsoleUI()
            : m_facade(nullptr), m_isRunning(false)
        {
        }

        // ====================== DESTRUCTOR ==================================
        ConsoleUI::~ConsoleUI()
        {
            // Facade is a singleton, we don't own it - nothing to delete
        }

        // ======================= STOP =======================================
        void ConsoleUI::stop()
        {
            m_isRunning = false;
        }

        // ====================== MAIN LOOP ===================================
        void ConsoleUI::run()
        {
            m_facade = HMSFacade::getInstance();
            if (!m_facade)
            {
                DisplayHelper::printError("Không thể khởi tạo hệ thống. Vui lòng thử lại.");
                return;
            }

            m_isRunning = true;
            while (m_isRunning)
            {
                showMainMenu();
            }
        }

        void ConsoleUI::showMainMenu()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printMainMenu();

            std::string inputChoice;
            bool isValid = false;

            do
            {
                inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-2]");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, 2);

                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 2.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            handleMainMenuChoice(choice);
        }

        void ConsoleUI::showLoginScreen()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("ĐĂNG NHẬP HỆ THỐNG");
            std::cout << "(Nhấn Enter để quay lại menu chính)\n\n";

            std::string username, password;

            while (true)
            {
                username = DisplayHelper::getInput("Nhập tên người dùng");

                if (username.empty())
                {
                    return; // Return to main menu naturally via the while loop
                }

                password = DisplayHelper::getInput("Nhập mật khẩu");

                bool isUserValid = InputValidator::validateUsername(username);
                bool isPassValid = InputValidator::validatePassword(password);

                if (isUserValid && isPassValid)
                {
                    break;
                }

                if (!isUserValid)
                {
                    std::cout << InputValidator::getUsernameError(username) << "\n";
                }
                if (!isPassValid)
                {
                    std::cout << InputValidator::getPasswordError(password) << "\n";
                }
                std::cout << "Vui lòng thử lại.\n";
            }

            if (m_facade->login(username, password))
            {
                DisplayHelper::printSuccess("Đăng nhập thành công");
                DisplayHelper::pause();
                routeToRoleMenu();
            }
            else
            {
                DisplayHelper::printError("Đăng nhập không thành công. Tên người dùng/mật khẩu không tồn tại\n");
                DisplayHelper::pause();
                // Return naturally - main loop will call showMainMenu() again
            }
        }

        void ConsoleUI::showRegisterScreen()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("ĐĂNG KÝ TÀI KHOẢN BỆNH NHÂN");
            std::cout << "(Nhấn Enter để quay lại menu chính)\n\n";

            // Get username
            std::string username;
            while (true)
            {
                username = DisplayHelper::getInput("Nhập tên người dùng");
                if (username.empty())
                    return;

                if (!InputValidator::validateUsername(username))
                {
                    std::cout << InputValidator::getUsernameError(username) << "\n";
                    continue;
                }
                if (!m_facade->isUsernameAvailable(username))
                {
                    DisplayHelper::printError("Tên người dùng đã tồn tại. Vui lòng chọn tên khác.");
                    continue;
                }
                break;
            }

            // Get password
            std::string password;
            while (true)
            {
                password = DisplayHelper::getInput("Nhập mật khẩu");
                if (password.empty())
                    return;

                if (!InputValidator::validatePassword(password))
                {
                    std::cout << InputValidator::getPasswordError(password) << "\n";
                    continue;
                }
                break;
            }

            // Get name
            std::string name;
            while (true)
            {
                name = DisplayHelper::getInput("Nhập họ và tên");
                if (name.empty())
                    return;

                if (!InputValidator::validateName(name))
                {
                    std::cout << InputValidator::getNameError(name) << "\n";
                    continue;
                }
                break;
            }

            // Get phone
            std::string phone;
            while (true)
            {
                phone = DisplayHelper::getInput("Nhập số điện thoại");
                if (phone.empty())
                    return;

                if (!InputValidator::validatePhone(phone))
                {
                    std::cout << InputValidator::getPhoneError(phone) << "\n";
                    continue;
                }
                break;
            }

            // Get gender
            std::string gender;
            while (true)
            {
                gender = DisplayHelper::getInput("Nhập giới tính (Nam/Nữ/Khác)");
                if (gender.empty())
                    return;

                if (!InputValidator::validateGender(gender))
                {
                    DisplayHelper::printError("Giới tính không hợp lệ. Vui lòng nhập: Nam, Nữ, hoặc Khác.");
                    continue;
                }
                gender = InputValidator::normalizeGender(gender);
                break;
            }

            // Get date of birth
            std::string dateOfBirth;
            while (true)
            {
                std::string dateInput = DisplayHelper::getInput("Nhập ngày sinh (DD-MM-YYYY)");
                if (dateInput.empty())
                    return;

                if (!InputValidator::validateDate(dateInput))
                {
                    std::cout << InputValidator::getDateError(dateInput) << "\n";
                    continue;
                }
                // Convert to internal format for validation and storage
                dateOfBirth = ::HMS::Utils::dateFromInput(dateInput);
                if (!InputValidator::validatePastDate(dateInput))
                {
                    DisplayHelper::printError("Ngày sinh phải là ngày trong quá khứ.");
                    continue;
                }
                break;
            }

            // Get address
            std::string address = DisplayHelper::getInput("Nhập địa chỉ");
            if (address.empty())
                return;

            // Confirm registration
            DisplayHelper::printSubHeader("Xác nhận thông tin");
            std::cout << "Tên người dùng: " << username << "\n";
            std::cout << "Họ tên: " << name << "\n";
            std::cout << "Số điện thoại: " << phone << "\n";
            std::cout << "Giới tính: " << gender << "\n";
            std::cout << "Ngày sinh: " << dateOfBirth << "\n";
            std::cout << "Địa chỉ: " << address << "\n\n";

            if (!DisplayHelper::confirm("Xác nhận đăng ký tài khoản?"))
            {
                DisplayHelper::printInfo("Đã hủy đăng ký.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->registerPatient(username, password, name, phone, gender, dateOfBirth, address))
            {
                DisplayHelper::printSuccess("Đăng ký thành công! Bạn có thể đăng nhập ngay bây giờ.");
            }
            else
            {
                DisplayHelper::printError("Đăng ký thất bại. Vui lòng thử lại.");
            }
            DisplayHelper::pause();
        }

        // ======================== MENU HANDLERS ========================
        void ConsoleUI::handleMainMenuChoice(int choice)
        {
            switch (choice)
            {
            case 0:
                m_facade->shutdown();
                stop(); // FIX: Now properly stops the main loop
                break;
            case 1:
                showLoginScreen();
                break;
            case 2:
                showRegisterScreen();
                break;
            default:
                break;
            }
        }

        void ConsoleUI::handlePatientChoice(int choice)
        {
            switch (choice)
            {
            case 0:
                performLogout();
                break;
            case 1:
                viewMyProfile();
                break;
            case 2:
                bookAppointment();
                break;
            case 3:
                viewAppointments();
                break;
            case 4:
                viewUpcomingAppointments();
                break;
            case 5:
                cancelAppointment();
                break;
            case 6:
                viewMyPrescriptions();
                break;
            case 7:
                searchMedicinePatient();
                break;
            case 8:
                viewTotalBill();
                break;
            default:
                break;
            }
        }

        void ConsoleUI::handleDoctorChoice(int choice)
        {
            switch (choice)
            {
            case 0:
                performLogout();
                break;
            case 1:
                viewDoctorSchedule();
                break;
            case 2:
                markAppointmentComplete();
                break;
            case 3:
                listAllPatients();
                break;
            case 4:
                searchPatient();
                break;
            case 5:
                addPatient();
                break;
            case 6:
                editPatient();
                break;
            case 7:
                deletePatient();
                break;
            case 8:
                showPrescriptionMenu();
                break;
            default:
                break;
            }
        }

        void ConsoleUI::handleAdminChoice(int choice)
        {
            switch (choice)
            {
            case 0:
                performLogout();
                break;
            case 1:
                listAllDoctors();
                break;
            case 2:
                searchDoctor();
                break;
            case 3:
                addDoctor();
                break;
            case 4:
                editDoctor();
                break;
            case 5:
                deleteDoctor();
                break;
            case 6:
                showDepartmentMenu();
                break;
            case 7:
                showMedicineMenu();
                break;
            case 8:
                showReportMenu();
                break;
            case 9:
                viewStatistics();
                break;
            default:
                break;
            }
        }

        // ======================== ROLE MENUS ========================
        void ConsoleUI::showPatientMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printPatientMenu(m_facade->getCurrentUsername());

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-8]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 8);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 8.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);
                handlePatientChoice(choice);
            }
        }

        void ConsoleUI::showDoctorMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printDoctorMenu(m_facade->getCurrentUsername());

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-8]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 8);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 8.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);
                handleDoctorChoice(choice);
            }
        }

        void ConsoleUI::showAdminMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printAdminMenu(m_facade->getCurrentUsername());

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-9]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 9);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 9.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);
                handleAdminChoice(choice);
            }
        }

        // ============================ HELPER METHODS =================
        void ConsoleUI::routeToRoleMenu()
        {
            Role role = m_facade->getCurrentRole();

            switch (role)
            {
            case Role::ADMIN:
                showAdminMenu();
                break;
            case Role::DOCTOR:
                showDoctorMenu();
                break;
            case Role::PATIENT:
                showPatientMenu();
                break;
            default:
                DisplayHelper::printError("Tài khoản không xác định");
                m_facade->logout();
                break;
            }
        }

        void ConsoleUI::performLogout()
        {
            m_facade->logout();
            DisplayHelper::printSuccess("Đăng xuất thành công.");
            DisplayHelper::pause();
        }

        // ==================== Patient Operations ====================
        void ConsoleUI::viewMyProfile()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THÔNG TIN CÁ NHÂN");

            auto profile = m_facade->getMyProfile();
            if (profile.has_value())
            {
                DisplayHelper::printPatientInfo(profile.value());
            }
            else
            {
                DisplayHelper::printError("Không thể tải thông tin cá nhân.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::bookAppointment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("ĐẶT LỊCH KHÁM");

            // Select doctor
            std::string doctorId = selectDoctor();
            if (doctorId.empty())
                return;

            // Select date
            std::string date = selectDate();
            if (date.empty())
                return;

            // Get available slots
            auto slots = m_facade->getAvailableSlots(doctorId, date);
            if (slots.empty())
            {
                DisplayHelper::printError("Không có lịch trống vào ngày này.");
                DisplayHelper::pause();
                return;
            }

            // Select time slot
            std::string time = selectTimeSlot(slots);
            if (time.empty())
                return;

            // Get disease description
            std::string disease = DisplayHelper::getInput("Nhập triệu chứng/lý do khám");
            if (disease.empty())
            {
                DisplayHelper::printInfo("Đã hủy đặt lịch.");
                DisplayHelper::pause();
                return;
            }

            // Confirm booking
            if (!DisplayHelper::confirm("Xác nhận đặt lịch khám?"))
            {
                DisplayHelper::printInfo("Đã hủy đặt lịch.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->bookAppointment(doctorId, date, time, disease))
            {
                DisplayHelper::printSuccess("Đặt lịch khám thành công!");
            }
            else
            {
                DisplayHelper::printError("Đặt lịch thất bại. Vui lòng thử lại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewAppointments()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("LỊCH SỬ KHÁM BỆNH");

            auto appointments = m_facade->getMyAppointments();
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn");
            }
            else
            {
                DisplayHelper::printAppointmentTable(appointments);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewUpcomingAppointments()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("LỊCH HẸN SẮP TỚI");

            auto appointments = m_facade->getMyUpcomingAppointments();
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn sắp tới");
            }
            else
            {
                DisplayHelper::printAppointmentTable(appointments);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::cancelAppointment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("HỦY LỊCH HẸN");

            auto appointments = m_facade->getMyUpcomingAppointments();
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn có thể hủy");
                DisplayHelper::pause();
                return;
            }

            std::string appointmentId = selectAppointment(appointments);
            if (appointmentId.empty())
                return;

            if (!DisplayHelper::confirm("Bạn có chắc chắn muốn hủy lịch hẹn này?"))
            {
                DisplayHelper::printInfo("Đã hủy thao tác.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->cancelAppointment(appointmentId))
            {
                DisplayHelper::printSuccess("Hủy lịch hẹn thành công.");
            }
            else
            {
                DisplayHelper::printError("Không thể hủy lịch hẹn.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewTotalBill()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("TỔNG HÓA ĐƠN");

            double total = m_facade->getMyTotalBill();
            std::cout << "Tổng số tiền cần thanh toán: " << DisplayHelper::formatMoney(total) << "\n";
            DisplayHelper::pause();
        }

        // ==================== Doctor Operations ====================
        void ConsoleUI::addPatient()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THÊM BỆNH NHÂN MỚI");
            std::cout << "(Nhấn Enter để quay lại)\n\n";

            // Get patient info with validation
            std::string name;
            while (true)
            {
                name = DisplayHelper::getInput("Nhập họ và tên");
                if (name.empty())
                    return;
                if (!InputValidator::validateName(name))
                {
                    std::cout << InputValidator::getNameError(name) << "\n";
                    continue;
                }
                break;
            }

            std::string phone;
            while (true)
            {
                phone = DisplayHelper::getInput("Nhập số điện thoại");
                if (phone.empty())
                    return;
                if (!InputValidator::validatePhone(phone))
                {
                    std::cout << InputValidator::getPhoneError(phone) << "\n";
                    continue;
                }
                break;
            }

            std::string gender;
            while (true)
            {
                gender = DisplayHelper::getInput("Nhập giới tính (Nam/Nữ/Khác)");
                if (gender.empty())
                    return;
                if (!InputValidator::validateGender(gender))
                {
                    DisplayHelper::printError("Giới tính không hợp lệ.");
                    continue;
                }
                gender = InputValidator::normalizeGender(gender);
                break;
            }

            std::string dateOfBirth;
            while (true)
            {
                std::string dateInput = DisplayHelper::getInput("Nhập ngày sinh (DD-MM-YYYY)");
                if (dateInput.empty())
                    return;
                if (!InputValidator::validateDate(dateInput) || !InputValidator::validatePastDate(dateInput))
                {
                    DisplayHelper::printError("Ngày sinh không hợp lệ. Vui lòng nhập theo định dạng DD-MM-YYYY.");
                    continue;
                }
                dateOfBirth = ::HMS::Utils::dateFromInput(dateInput);
                break;
            }

            std::string address = DisplayHelper::getInput("Nhập địa chỉ");
            if (address.empty())
                return;

            std::string medicalHistory = DisplayHelper::getInput("Nhập tiền sử bệnh (nếu có)");

            if (!DisplayHelper::confirm("Xác nhận thêm bệnh nhân?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->addPatient(name, phone, gender, dateOfBirth, address, medicalHistory))
            {
                DisplayHelper::printSuccess("Thêm bệnh nhân thành công.");
            }
            else
            {
                DisplayHelper::printError("Thêm bệnh nhân thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::searchPatient()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("TÌM KIẾM BỆNH NHÂN");

            std::string keyword = DisplayHelper::getInput("Nhập từ khóa tìm kiếm (tên, ID, SĐT)");
            if (keyword.empty())
                return;

            auto patients = m_facade->searchPatients(keyword);
            if (patients.empty())
            {
                DisplayHelper::printNoData("bệnh nhân phù hợp");
            }
            else
            {
                DisplayHelper::printPatientTable(patients);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::editPatient()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẬP NHẬT THÔNG TIN BỆNH NHÂN");

            std::string patientId = selectPatient();
            if (patientId.empty())
                return;

            auto patient = m_facade->getPatientByID(patientId);
            if (!patient.has_value())
            {
                DisplayHelper::printError("Không tìm thấy bệnh nhân.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printPatientInfo(patient.value());
            std::cout << "\n(Để trống trường không muốn thay đổi)\n\n";

            std::string phone = DisplayHelper::getInput("Số điện thoại mới");
            std::string address = DisplayHelper::getInput("Địa chỉ mới");
            std::string medicalHistory = DisplayHelper::getInput("Tiền sử bệnh mới");

            if (!DisplayHelper::confirm("Xác nhận cập nhật?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->updatePatient(patientId, phone, address, medicalHistory))
            {
                DisplayHelper::printSuccess("Cập nhật thành công.");
            }
            else
            {
                DisplayHelper::printError("Cập nhật thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::deletePatient()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("XÓA BỆNH NHÂN");

            std::string patientId = selectPatient();
            if (patientId.empty())
                return;

            auto patient = m_facade->getPatientByID(patientId);
            if (!patient.has_value())
            {
                DisplayHelper::printError("Không tìm thấy bệnh nhân.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printPatientInfo(patient.value());

            if (!DisplayHelper::confirm("Bạn có chắc chắn muốn xóa bệnh nhân này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->deletePatient(patientId))
            {
                DisplayHelper::printSuccess("Xóa bệnh nhân thành công.");
            }
            else
            {
                DisplayHelper::printError("Không thể xóa bệnh nhân.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::listAllPatients()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANH SÁCH BỆNH NHÂN");

            auto patients = m_facade->getAllPatients();
            if (patients.empty())
            {
                DisplayHelper::printNoData("bệnh nhân");
            }
            else
            {
                DisplayHelper::printPatientTable(patients);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewDoctorSchedule()
        {
            while (true)
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printHeader("LỊCH LÀM VIỆC");

                // Show all upcoming appointments by default
                auto appointments = m_facade->getMyUpcomingAppointments();
                if (appointments.empty())
                {
                    DisplayHelper::printNoData("lịch hẹn sắp tới");
                }
                else
                {
                    std::cout << "Tất cả lịch hẹn sắp tới:\n\n";
                    DisplayHelper::printAppointmentTable(appointments);
                }

                std::cout << "\n";
                std::cout << "1. Lọc theo ngày cụ thể\n";
                std::cout << "0. Quay lại\n\n";

                std::string choice = DisplayHelper::getInput("Nhập lựa chọn");
                if (choice == "0" || choice.empty())
                    return;

                if (choice == "1")
                {
                    std::string date = selectDate();
                    if (date.empty())
                        continue;

                    auto filtered = m_facade->getMySchedule(date);
                    DisplayHelper::clearScreen();
                    DisplayHelper::printHeader("LỊCH LÀM VIỆC");
                    std::cout << "Lịch hẹn ngày " << DisplayHelper::formatDate(date) << ":\n\n";

                    if (filtered.empty())
                    {
                        DisplayHelper::printNoData("lịch hẹn cho ngày này");
                    }
                    else
                    {
                        DisplayHelper::printAppointmentTable(filtered);
                    }
                    DisplayHelper::pause();
                }
            }
        }

        void ConsoleUI::markAppointmentComplete()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("HOÀN THÀNH LỊCH HẸN");

            // Get today's date for schedule
            std::string today;
            {
                time_t now = time(nullptr);
                tm *ltm = localtime(&now);
                char buffer[40]; // Large enough for worst case
                snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                         1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
                today = buffer;
            }

            auto appointments = m_facade->getMySchedule(today);
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn hôm nay");
                DisplayHelper::pause();
                return;
            }

            std::string appointmentId = selectAppointment(appointments);
            if (appointmentId.empty())
                return;

            if (!DisplayHelper::confirm("Xác nhận hoàn thành lịch hẹn này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->markAppointmentCompleted(appointmentId))
            {
                DisplayHelper::printSuccess("Đã đánh dấu hoàn thành.");
            }
            else
            {
                DisplayHelper::printError("Không thể cập nhật trạng thái.");
            }
            DisplayHelper::pause();
        }

        // ==================== Admin Operations ====================
        void ConsoleUI::addDoctor()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THÊM BÁC SĨ MỚI");
            std::cout << "(Nhấn Enter để quay lại)\n\n";

            // Get username
            std::string username;
            while (true)
            {
                username = DisplayHelper::getInput("Nhập tên người dùng");
                if (username.empty())
                    return;
                if (!InputValidator::validateUsername(username))
                {
                    std::cout << InputValidator::getUsernameError(username) << "\n";
                    continue;
                }
                if (!m_facade->isUsernameAvailable(username))
                {
                    DisplayHelper::printError("Tên người dùng đã tồn tại.");
                    continue;
                }
                break;
            }

            std::string password;
            while (true)
            {
                password = DisplayHelper::getInput("Nhập mật khẩu");
                if (password.empty())
                    return;
                if (!InputValidator::validatePassword(password))
                {
                    std::cout << InputValidator::getPasswordError(password) << "\n";
                    continue;
                }
                break;
            }

            std::string name;
            while (true)
            {
                name = DisplayHelper::getInput("Nhập họ và tên");
                if (name.empty())
                    return;
                if (!InputValidator::validateName(name))
                {
                    std::cout << InputValidator::getNameError(name) << "\n";
                    continue;
                }
                break;
            }

            std::string phone;
            while (true)
            {
                phone = DisplayHelper::getInput("Nhập số điện thoại");
                if (phone.empty())
                    return;
                if (!InputValidator::validatePhone(phone))
                {
                    std::cout << InputValidator::getPhoneError(phone) << "\n";
                    continue;
                }
                break;
            }

            std::string gender;
            while (true)
            {
                gender = DisplayHelper::getInput("Nhập giới tính (Nam/Nữ/Khác)");
                if (gender.empty())
                    return;
                if (!InputValidator::validateGender(gender))
                {
                    DisplayHelper::printError("Giới tính không hợp lệ.");
                    continue;
                }
                gender = InputValidator::normalizeGender(gender);
                break;
            }

            std::string dateOfBirth;
            while (true)
            {
                std::string dateInput = DisplayHelper::getInput("Nhập ngày sinh (DD-MM-YYYY)");
                if (dateInput.empty())
                    return;
                if (!InputValidator::validateDate(dateInput) || !InputValidator::validatePastDate(dateInput))
                {
                    DisplayHelper::printError("Ngày sinh không hợp lệ. Vui lòng nhập theo định dạng DD-MM-YYYY.");
                    continue;
                }
                dateOfBirth = ::HMS::Utils::dateFromInput(dateInput);
                break;
            }

            std::string specialization = DisplayHelper::getInput("Nhập chuyên khoa");
            if (specialization.empty())
                return;

            std::string schedule = DisplayHelper::getInput("Nhập lịch làm việc (vd: Mon-Fri 8:00-17:00)");
            if (schedule.empty())
                return;

            double consultationFee = DisplayHelper::getDoubleInput("Nhập phí khám (VND)");
            if (consultationFee < 0)
            {
                DisplayHelper::printError("Phí khám không hợp lệ.");
                DisplayHelper::pause();
                return;
            }

            if (!DisplayHelper::confirm("Xác nhận thêm bác sĩ?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->addDoctor(username, password, name, phone, gender, dateOfBirth,
                                    specialization, schedule, consultationFee))
            {
                DisplayHelper::printSuccess("Thêm bác sĩ thành công.");
            }
            else
            {
                DisplayHelper::printError("Thêm bác sĩ thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::searchDoctor()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("TÌM KIẾM BÁC SĨ");

            std::cout << "1. Tìm theo chuyên khoa\n";
            std::cout << "2. Xem tất cả bác sĩ\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Nhập lựa chọn [1-2]");
                isValid = InputValidator::validateMenuChoice(inputChoice, 1, 2);
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);

            std::vector<Model::Doctor> doctors;
            if (choice == 1)
            {
                auto specs = m_facade->getAllSpecializations();
                if (specs.empty())
                {
                    DisplayHelper::printNoData("chuyên khoa");
                    DisplayHelper::pause();
                    return;
                }

                DisplayHelper::printSubHeader("Danh sách chuyên khoa");
                for (size_t i = 0; i < specs.size(); ++i)
                {
                    std::cout << (i + 1) << ". " << specs[i] << "\n";
                }

                std::string specChoice;
                do
                {
                    specChoice = DisplayHelper::getInput("Chọn chuyên khoa");
                    isValid = InputValidator::validateMenuChoice(specChoice, 1, static_cast<int>(specs.size()));
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ.");
                    }
                } while (!isValid);

                int specIndex = InputValidator::parseMenuChoice(specChoice) - 1;
                doctors = m_facade->getDoctorsBySpecialization(specs[specIndex]);
            }
            else
            {
                doctors = m_facade->getAllDoctors();
            }

            if (doctors.empty())
            {
                DisplayHelper::printNoData("bác sĩ");
            }
            else
            {
                DisplayHelper::printDoctorTable(doctors);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::editDoctor()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẬP NHẬT THÔNG TIN BÁC SĨ");

            std::string doctorId = selectDoctor();
            if (doctorId.empty())
                return;

            auto doctor = m_facade->getDoctorByID(doctorId);
            if (!doctor.has_value())
            {
                DisplayHelper::printError("Không tìm thấy bác sĩ.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printDoctorInfo(doctor.value());
            std::cout << "\n(Để trống trường không muốn thay đổi)\n\n";

            std::string specialization = DisplayHelper::getInput("Chuyên khoa mới");
            std::string schedule = DisplayHelper::getInput("Lịch làm việc mới");
            double consultationFee = DisplayHelper::getDoubleInput("Phí khám mới");

            if (!DisplayHelper::confirm("Xác nhận cập nhật?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->updateDoctor(doctorId, specialization, schedule, consultationFee))
            {
                DisplayHelper::printSuccess("Cập nhật thành công.");
            }
            else
            {
                DisplayHelper::printError("Cập nhật thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::deleteDoctor()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("XÓA BÁC SĨ");

            std::string doctorId = selectDoctor();
            if (doctorId.empty())
                return;

            auto doctor = m_facade->getDoctorByID(doctorId);
            if (!doctor.has_value())
            {
                DisplayHelper::printError("Không tìm thấy bác sĩ.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printDoctorInfo(doctor.value());

            if (!DisplayHelper::confirm("Bạn có chắc chắn muốn xóa bác sĩ này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->deleteDoctor(doctorId))
            {
                DisplayHelper::printSuccess("Xóa bác sĩ thành công.");
            }
            else
            {
                DisplayHelper::printError("Không thể xóa bác sĩ.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::listAllDoctors()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANH SÁCH BÁC SĨ");

            auto doctors = m_facade->getAllDoctors();
            if (doctors.empty())
            {
                DisplayHelper::printNoData("bác sĩ");
            }
            else
            {
                DisplayHelper::printDoctorTable(doctors);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewStatistics()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THỐNG KÊ HỆ THỐNG");

            auto stats = m_facade->getStatistics();
            DisplayHelper::printStatistics(stats);
            DisplayHelper::pause();
        }

        void ConsoleUI::viewReports()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("BÁO CÁO");

            std::string report = m_facade->generateReport();
            std::cout << report << "\n";
            DisplayHelper::pause();
        }

        // ==================== Selection Helpers ====================
        std::string ConsoleUI::selectDoctor()
        {
            auto doctors = m_facade->getAllDoctors();
            if (doctors.empty())
            {
                DisplayHelper::printNoData("bác sĩ");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printDoctorList(doctors);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn bác sĩ");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(doctors.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return doctors[choice - 1].getID();
        }

        std::string ConsoleUI::selectPatient()
        {
            auto patients = m_facade->getAllPatients();
            if (patients.empty())
            {
                DisplayHelper::printNoData("bệnh nhân");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printPatientList(patients);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn bệnh nhân");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(patients.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return patients[choice - 1].getID();
        }

        std::string ConsoleUI::selectAppointment(const std::vector<Model::Appointment> &appointments)
        {
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printAppointmentList(appointments);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn lịch hẹn");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(appointments.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return appointments[choice - 1].getAppointmentID();
        }

        std::string ConsoleUI::selectDate()
        {
            std::string date;
            while (true)
            {
                std::string dateInput = DisplayHelper::getInput("Nhập ngày (DD-MM-YYYY, bỏ trống để quay lại)");
                if (dateInput.empty())
                    return "";

                if (!InputValidator::validateDate(dateInput))
                {
                    std::cout << InputValidator::getDateError(dateInput) << "\n";
                    continue;
                }
                if (!InputValidator::validateFutureDate(dateInput))
                {
                    DisplayHelper::printError("Ngày phải là hôm nay hoặc trong tương lai.");
                    continue;
                }
                // Convert to internal format (YYYY-MM-DD) for storage
                date = ::HMS::Utils::dateFromInput(dateInput);
                break;
            }
            return date;
        }

        std::string ConsoleUI::selectTimeSlot(const std::vector<std::string> &slots)
        {
            if (slots.empty())
            {
                DisplayHelper::printNoData("khung giờ trống");
                return "";
            }

            DisplayHelper::printTimeSlotList(slots);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn khung giờ");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(slots.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return slots[choice - 1];
        }

        // ==================== Selection Helpers for Advanced Features ====================

        std::string ConsoleUI::selectMedicine()
        {
            auto medicines = m_facade->getAllMedicines();
            if (medicines.empty())
            {
                DisplayHelper::printNoData("thuốc");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printMedicineList(medicines);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn thuốc");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(medicines.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return medicines[choice - 1].getMedicineID();
        }

        std::string ConsoleUI::selectDepartment()
        {
            auto departments = m_facade->getAllDepartments();
            if (departments.empty())
            {
                DisplayHelper::printNoData("khoa");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printDepartmentList(departments);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn khoa");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(departments.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return departments[choice - 1].getDepartmentID();
        }

        std::string ConsoleUI::selectPrescription()
        {
            // Get prescriptions for current doctor
            auto doctor = m_facade->getDoctorByID(m_facade->getCurrentUsername());
            std::vector<Model::Prescription> prescriptions;

            if (m_facade->getCurrentRole() == Role::DOCTOR)
            {
                // Find doctor by username to get doctorID
                auto doctors = m_facade->getAllDoctors();
                for (const auto& doc : doctors)
                {
                    if (doc.getUsername() == m_facade->getCurrentUsername())
                    {
                        prescriptions = m_facade->getDoctorPrescriptions(doc.getDoctorID());
                        break;
                    }
                }
            }

            if (prescriptions.empty())
            {
                DisplayHelper::printNoData("đơn thuốc");
                DisplayHelper::pause();
                return "";
            }

            DisplayHelper::printPrescriptionList(prescriptions);
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn đơn thuốc");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, static_cast<int>(prescriptions.size()));
                if (!isValid)
                {
                    DisplayHelper::printError("Lựa chọn không hợp lệ.");
                }
            } while (!isValid);

            int choice = InputValidator::parseMenuChoice(inputChoice);
            if (choice == 0)
                return "";

            return prescriptions[choice - 1].getPrescriptionID();
        }

        // ==================== Medicine Management Sub-menu (Admin) ====================

        void ConsoleUI::showMedicineMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printMedicineMenu();

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-8]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 8);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);

                switch (choice)
                {
                case 0:
                    return;
                case 1:
                    viewMedicines();
                    break;
                case 2:
                    searchMedicine();
                    break;
                case 3:
                    addMedicine();
                    break;
                case 4:
                    editMedicine();
                    break;
                case 5:
                    deleteMedicine();
                    break;
                case 6:
                    updateMedicineStock();
                    break;
                case 7:
                    viewLowStockAlerts();
                    break;
                case 8:
                    viewExpiryAlerts();
                    break;
                }
            }
        }

        void ConsoleUI::viewMedicines()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANH SÁCH THUỐC");

            auto medicines = m_facade->getAllMedicines();
            if (medicines.empty())
            {
                DisplayHelper::printNoData("thuốc");
            }
            else
            {
                DisplayHelper::printMedicineTable(medicines);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::searchMedicine()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("TÌM KIẾM THUỐC");

            std::string keyword = DisplayHelper::getInput("Nhập từ khóa (tên, danh mục, mã thuốc)");
            if (keyword.empty())
                return;

            auto medicines = m_facade->searchMedicines(keyword);
            if (medicines.empty())
            {
                DisplayHelper::printNoData("thuốc phù hợp");
            }
            else
            {
                DisplayHelper::printMedicineTable(medicines);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::addMedicine()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THÊM THUỐC MỚI");
            std::cout << "(Nhấn Enter để quay lại)\n\n";

            std::string name = DisplayHelper::getInput("Tên thuốc");
            if (name.empty())
                return;

            std::string genericName = DisplayHelper::getInput("Tên khoa học/generic");
            if (genericName.empty())
                return;

            std::string category = DisplayHelper::getInput("Danh mục (vd: Kháng sinh, Giảm đau)");
            if (category.empty())
                return;

            std::string manufacturer = DisplayHelper::getInput("Nhà sản xuất");
            if (manufacturer.empty())
                return;

            std::string description = DisplayHelper::getInput("Mô tả (tùy chọn)");

            double unitPrice = DisplayHelper::getDoubleInput("Đơn giá (VND)");
            if (unitPrice < 0)
                return;

            int quantity = DisplayHelper::getIntInput("Số lượng tồn kho", 0, 1000000);
            if (quantity < 0)
                return;

            int reorderLevel = DisplayHelper::getIntInput("Mức đặt hàng lại", 1, 10000);
            if (reorderLevel < 0)
                return;

            std::string expiryDate;
            while (true)
            {
                std::string dateInput = DisplayHelper::getInput("Ngày hết hạn (DD-MM-YYYY)");
                if (dateInput.empty())
                    return;

                if (!InputValidator::validateDate(dateInput) || !InputValidator::validateFutureDate(dateInput))
                {
                    DisplayHelper::printError("Ngày hết hạn không hợp lệ hoặc phải là tương lai.");
                    continue;
                }
                expiryDate = ::HMS::Utils::dateFromInput(dateInput);
                break;
            }

            std::string dosageForm = DisplayHelper::getInput("Dạng bào chế (vd: Viên, Ống tiêm)");
            if (dosageForm.empty())
                return;

            std::string strength = DisplayHelper::getInput("Liều lượng (vd: 500mg, 10ml)");
            if (strength.empty())
                return;

            // Generate medicine ID
            auto allMeds = m_facade->getAllMedicines();
            int nextNum = allMeds.size() + 1;
            char idBuffer[10];
            snprintf(idBuffer, sizeof(idBuffer), "MED%03d", nextNum);
            std::string medicineID = idBuffer;

            // Show confirmation
            DisplayHelper::printSubHeader("Xác nhận thông tin thuốc");
            std::cout << "Mã thuốc: " << medicineID << "\n";
            std::cout << "Tên thuốc: " << name << "\n";
            std::cout << "Tên khoa học: " << genericName << "\n";
            std::cout << "Danh mục: " << category << "\n";
            std::cout << "Nhà sản xuất: " << manufacturer << "\n";
            std::cout << "Đơn giá: " << DisplayHelper::formatMoney(unitPrice) << "\n";
            std::cout << "Số lượng: " << quantity << "\n";
            std::cout << "Mức đặt hàng lại: " << reorderLevel << "\n";
            std::cout << "Hết hạn: " << DisplayHelper::formatDate(expiryDate) << "\n\n";

            if (!DisplayHelper::confirm("Xác nhận thêm thuốc?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->createMedicine(medicineID, name, genericName, category, manufacturer,
                                         description, unitPrice, quantity, reorderLevel, expiryDate, dosageForm, strength))
            {
                DisplayHelper::printSuccess("Thêm thuốc thành công. Mã thuốc: " + medicineID);
            }
            else
            {
                DisplayHelper::printError("Thêm thuốc thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::editMedicine()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẬP NHẬT THÔNG TIN THUỐC");

            std::string medicineID = selectMedicine();
            if (medicineID.empty())
                return;

            auto medicine = m_facade->getMedicineByID(medicineID);
            if (!medicine.has_value())
            {
                DisplayHelper::printError("Không tìm thấy thuốc.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printMedicineInfo(medicine.value());
            std::cout << "\n(Để trống trường không muốn thay đổi)\n\n";

            std::string name = DisplayHelper::getInput("Tên thuốc mới");
            std::string category = DisplayHelper::getInput("Danh mục mới");
            double unitPrice = DisplayHelper::getDoubleInput("Đơn giá mới (VND, -1 để giữ nguyên)");
            int reorderLevel = DisplayHelper::getIntInput("Mức đặt hàng lại mới", -1, 10000);

            std::string expiryDate;
            std::string dateInput = DisplayHelper::getInput("Ngày hết hạn mới (DD-MM-YYYY)");
            if (!dateInput.empty())
            {
                if (InputValidator::validateDate(dateInput) && InputValidator::validateFutureDate(dateInput))
                {
                    expiryDate = ::HMS::Utils::dateFromInput(dateInput);
                }
            }

            if (!DisplayHelper::confirm("Xác nhận cập nhật?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            // Use existing values if input is empty
            std::string finalName = name.empty() ? medicine->getName() : name;
            std::string finalCategory = category.empty() ? medicine->getCategory() : category;
            double finalPrice = (unitPrice < 0) ? medicine->getUnitPrice() : unitPrice;
            int finalReorder = (reorderLevel < 0) ? medicine->getReorderLevel() : reorderLevel;
            std::string finalExpiry = expiryDate.empty() ? medicine->getExpiryDate() : expiryDate;

            if (m_facade->updateMedicine(medicineID, finalName, finalCategory, finalPrice, finalReorder, finalExpiry))
            {
                DisplayHelper::printSuccess("Cập nhật thành công.");
            }
            else
            {
                DisplayHelper::printError("Cập nhật thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::deleteMedicine()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("XÓA THUỐC");

            std::string medicineID = selectMedicine();
            if (medicineID.empty())
                return;

            auto medicine = m_facade->getMedicineByID(medicineID);
            if (!medicine.has_value())
            {
                DisplayHelper::printError("Không tìm thấy thuốc.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printMedicineInfo(medicine.value());

            if (!DisplayHelper::confirm("Bạn có chắc chắn muốn xóa thuốc này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->deleteMedicine(medicineID))
            {
                DisplayHelper::printSuccess("Xóa thuốc thành công.");
            }
            else
            {
                DisplayHelper::printError("Không thể xóa thuốc.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::updateMedicineStock()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("NHẬP/XUẤT TỒN KHO");

            std::string medicineID = selectMedicine();
            if (medicineID.empty())
                return;

            auto medicine = m_facade->getMedicineByID(medicineID);
            if (!medicine.has_value())
            {
                DisplayHelper::printError("Không tìm thấy thuốc.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printMedicineInfo(medicine.value());

            std::cout << "\n1. Nhập kho (thêm số lượng)\n";
            std::cout << "2. Xuất kho (giảm số lượng)\n";
            std::cout << "0. Quay lại\n\n";

            std::string inputChoice;
            bool isValid = false;
            do
            {
                inputChoice = DisplayHelper::getInput("Chọn thao tác [0-2]");
                isValid = InputValidator::validateMenuChoice(inputChoice, 0, 2);
            } while (!isValid);

            int action = InputValidator::parseMenuChoice(inputChoice);
            if (action == 0)
                return;

            int quantity = DisplayHelper::getIntInput("Nhập số lượng", 1, 100000);
            if (quantity < 0)
                return;

            bool success = false;
            if (action == 1)
            {
                success = m_facade->addMedicineStock(medicineID, quantity);
                if (success)
                {
                    DisplayHelper::printSuccess("Nhập kho thành công. Tồn kho mới: " +
                                                std::to_string(medicine->getQuantityInStock() + quantity));
                }
            }
            else
            {
                if (quantity > medicine->getQuantityInStock())
                {
                    DisplayHelper::printError("Số lượng xuất vượt quá tồn kho hiện tại.");
                    DisplayHelper::pause();
                    return;
                }
                success = m_facade->removeMedicineStock(medicineID, quantity);
                if (success)
                {
                    DisplayHelper::printSuccess("Xuất kho thành công. Tồn kho mới: " +
                                                std::to_string(medicine->getQuantityInStock() - quantity));
                }
            }

            if (!success)
            {
                DisplayHelper::printError("Thao tác thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewLowStockAlerts()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẢNH BÁO TỒN KHO THẤP");

            auto alerts = m_facade->getLowStockAlerts();
            if (alerts.empty())
            {
                DisplayHelper::printInfo("Không có thuốc nào có tồn kho thấp.");
            }
            else
            {
                DisplayHelper::printStockAlertTable(alerts);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewExpiryAlerts()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẢNH BÁO HẾT HẠN");

            auto alerts = m_facade->getExpiryAlerts();
            if (alerts.empty())
            {
                DisplayHelper::printInfo("Không có thuốc nào sắp hết hạn.");
            }
            else
            {
                DisplayHelper::printExpiryAlertTable(alerts);
            }
            DisplayHelper::pause();
        }

        // ==================== Department Management Sub-menu (Admin) ====================

        void ConsoleUI::showDepartmentMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printDepartmentMenu();

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-6]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 6);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);

                switch (choice)
                {
                case 0:
                    return;
                case 1:
                    viewDepartments();
                    break;
                case 2:
                    addDepartment();
                    break;
                case 3:
                    editDepartment();
                    break;
                case 4:
                    deleteDepartment();
                    break;
                case 5:
                    assignDoctorToDepartment();
                    break;
                case 6:
                    viewDepartmentStats();
                    break;
                }
            }
        }

        void ConsoleUI::viewDepartments()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANH SÁCH KHOA");

            auto departments = m_facade->getAllDepartments();
            if (departments.empty())
            {
                DisplayHelper::printNoData("khoa");
            }
            else
            {
                DisplayHelper::printDepartmentTable(departments);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::addDepartment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THÊM KHOA MỚI");
            std::cout << "(Nhấn Enter để quay lại)\n\n";

            std::string name = DisplayHelper::getInput("Tên khoa");
            if (name.empty())
                return;

            std::string description = DisplayHelper::getInput("Mô tả");
            if (description.empty())
                return;

            std::string location = DisplayHelper::getInput("Vị trí (vd: Tòa A, Tầng 2)");
            if (location.empty())
                return;

            std::string phone = DisplayHelper::getInput("Số điện thoại liên hệ");
            if (phone.empty())
                return;

            // Optional: Select head doctor
            std::string headDoctorID = "";
            if (DisplayHelper::confirm("Bạn muốn chỉ định trưởng khoa ngay bây giờ?"))
            {
                headDoctorID = selectDoctor();
            }

            // Generate department ID
            auto allDepts = m_facade->getAllDepartments();
            int nextNum = allDepts.size() + 1;
            char idBuffer[10];
            snprintf(idBuffer, sizeof(idBuffer), "DEP%03d", nextNum);
            std::string departmentID = idBuffer;

            // Show confirmation
            DisplayHelper::printSubHeader("Xác nhận thông tin khoa");
            std::cout << "Mã khoa: " << departmentID << "\n";
            std::cout << "Tên khoa: " << name << "\n";
            std::cout << "Mô tả: " << description << "\n";
            std::cout << "Vị trí: " << location << "\n";
            std::cout << "Điện thoại: " << phone << "\n";
            if (!headDoctorID.empty())
            {
                std::cout << "Trưởng khoa: " << headDoctorID << "\n";
            }
            std::cout << "\n";

            if (!DisplayHelper::confirm("Xác nhận thêm khoa?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->createDepartment(departmentID, name, description, headDoctorID, location, phone))
            {
                DisplayHelper::printSuccess("Thêm khoa thành công. Mã khoa: " + departmentID);
            }
            else
            {
                DisplayHelper::printError("Thêm khoa thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::editDepartment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẬP NHẬT THÔNG TIN KHOA");

            std::string departmentID = selectDepartment();
            if (departmentID.empty())
                return;

            auto department = m_facade->getDepartmentByID(departmentID);
            if (!department.has_value())
            {
                DisplayHelper::printError("Không tìm thấy khoa.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printDepartmentInfo(department.value());
            std::cout << "\n(Để trống trường không muốn thay đổi)\n\n";

            std::string name = DisplayHelper::getInput("Tên khoa mới");
            std::string description = DisplayHelper::getInput("Mô tả mới");
            std::string location = DisplayHelper::getInput("Vị trí mới");
            std::string phone = DisplayHelper::getInput("Số điện thoại mới");

            std::string headDoctorID = "";
            if (DisplayHelper::confirm("Bạn muốn thay đổi trưởng khoa?"))
            {
                headDoctorID = selectDoctor();
            }

            if (!DisplayHelper::confirm("Xác nhận cập nhật?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            // Use existing values if input is empty
            std::string finalName = name.empty() ? department->getName() : name;
            std::string finalDesc = description.empty() ? department->getDescription() : description;
            std::string finalLoc = location.empty() ? department->getLocation() : location;
            std::string finalPhone = phone.empty() ? department->getPhone() : phone;
            std::string finalHead = headDoctorID.empty() ? department->getHeadDoctorID() : headDoctorID;

            if (m_facade->updateDepartment(departmentID, finalName, finalDesc, finalHead, finalLoc, finalPhone))
            {
                DisplayHelper::printSuccess("Cập nhật thành công.");
            }
            else
            {
                DisplayHelper::printError("Cập nhật thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::deleteDepartment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("XÓA KHOA");

            std::string departmentID = selectDepartment();
            if (departmentID.empty())
                return;

            auto department = m_facade->getDepartmentByID(departmentID);
            if (!department.has_value())
            {
                DisplayHelper::printError("Không tìm thấy khoa.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printDepartmentInfo(department.value());

            if (!DisplayHelper::confirm("Bạn có chắc chắn muốn xóa khoa này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->deleteDepartment(departmentID))
            {
                DisplayHelper::printSuccess("Xóa khoa thành công.");
            }
            else
            {
                DisplayHelper::printError("Không thể xóa khoa.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::assignDoctorToDepartment()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("PHÂN CÔNG BÁC SĨ VÀO KHOA");

            std::string departmentID = selectDepartment();
            if (departmentID.empty())
                return;

            std::string doctorID = selectDoctor();
            if (doctorID.empty())
                return;

            if (!DisplayHelper::confirm("Xác nhận phân công bác sĩ " + doctorID + " vào khoa " + departmentID + "?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->assignDoctorToDepartment(doctorID, departmentID))
            {
                DisplayHelper::printSuccess("Phân công thành công.");
            }
            else
            {
                DisplayHelper::printError("Phân công thất bại.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::viewDepartmentStats()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THỐNG KÊ KHOA");

            std::string departmentID = selectDepartment();
            if (departmentID.empty())
                return;

            auto department = m_facade->getDepartmentByID(departmentID);
            if (!department.has_value())
            {
                DisplayHelper::printError("Không tìm thấy khoa.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("THỐNG KÊ KHOA: " + department->getName());

            auto stats = m_facade->getDepartmentStats(departmentID);

            std::cout << "\n";
            std::cout << "Số bác sĩ: " << stats.doctorCount << "\n";
            std::cout << "Tổng lịch hẹn: " << stats.appointmentCount << "\n";
            std::cout << "Tổng doanh thu: " << DisplayHelper::formatMoney(stats.totalRevenue) << "\n";

            if (!stats.doctorNames.empty())
            {
                std::cout << "\nDanh sách bác sĩ:\n";
                for (size_t i = 0; i < stats.doctorNames.size(); ++i)
                {
                    std::cout << "  " << (i + 1) << ". " << stats.doctorNames[i] << "\n";
                }
            }

            DisplayHelper::pause();
        }

        // ==================== Report Generation Sub-menu (Admin) ====================

        void ConsoleUI::showReportMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printReportMenu();

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-5]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 5);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);

                switch (choice)
                {
                case 0:
                    return;
                case 1:
                    generateDailyReport();
                    break;
                case 2:
                    generateWeeklyReport();
                    break;
                case 3:
                    generateMonthlyReport();
                    break;
                case 4:
                    generateRevenueReport();
                    break;
                case 5:
                    exportReport();
                    break;
                }
            }
        }

        void ConsoleUI::generateDailyReport()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("BÁO CÁO NGÀY");

            std::string date = selectDate();
            if (date.empty())
                return;

            std::string report = m_facade->generateDailyReport(date, "txt");
            std::cout << "\n" << report << "\n";
            DisplayHelper::pause();
        }

        void ConsoleUI::generateWeeklyReport()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("BÁO CÁO TUẦN");

            std::cout << "Chọn ngày bắt đầu tuần:\n";
            std::string startDate = selectDate();
            if (startDate.empty())
                return;

            std::string report = m_facade->generateWeeklyReport(startDate, "txt");
            std::cout << "\n" << report << "\n";
            DisplayHelper::pause();
        }

        void ConsoleUI::generateMonthlyReport()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("BÁO CÁO THÁNG");

            int year = DisplayHelper::getIntInput("Nhập năm (2020-2030)", 2020, 2030);
            if (year < 0)
                return;

            int month = DisplayHelper::getIntInput("Nhập tháng (1-12)", 1, 12);
            if (month < 0)
                return;

            std::string report = m_facade->generateMonthlyReport(year, month, "txt");
            std::cout << "\n" << report << "\n";
            DisplayHelper::pause();
        }

        void ConsoleUI::generateRevenueReport()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("BÁO CÁO DOANH THU");

            std::cout << "Chọn ngày bắt đầu:\n";
            std::string startDate = selectDate();
            if (startDate.empty())
                return;

            std::cout << "Chọn ngày kết thúc:\n";
            std::string endDate = selectDate();
            if (endDate.empty())
                return;

            std::string report = m_facade->generateRevenueReport(startDate, endDate, "txt");
            std::cout << "\n" << report << "\n";
            DisplayHelper::pause();
        }

        void ConsoleUI::exportReport()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("XUẤT BÁO CÁO RA FILE");

            std::cout << "Chọn loại báo cáo:\n";
            std::cout << "1. Báo cáo ngày\n";
            std::cout << "2. Báo cáo tuần\n";
            std::cout << "3. Báo cáo tháng\n";
            std::cout << "4. Báo cáo doanh thu\n";
            std::cout << "0. Quay lại\n\n";

            int reportType = DisplayHelper::getIntInput("Chọn loại báo cáo [0-4]", 0, 4);
            if (reportType <= 0)
                return;

            std::string reportContent;
            std::string defaultFilename;

            switch (reportType)
            {
            case 1:
            {
                std::string date = selectDate();
                if (date.empty())
                    return;
                reportContent = m_facade->generateDailyReport(date, "txt");
                defaultFilename = "bao_cao_ngay_" + date;
                break;
            }
            case 2:
            {
                std::string startDate = selectDate();
                if (startDate.empty())
                    return;
                reportContent = m_facade->generateWeeklyReport(startDate, "txt");
                defaultFilename = "bao_cao_tuan_" + startDate;
                break;
            }
            case 3:
            {
                int year = DisplayHelper::getIntInput("Nhập năm", 2020, 2030);
                int month = DisplayHelper::getIntInput("Nhập tháng", 1, 12);
                reportContent = m_facade->generateMonthlyReport(year, month, "txt");
                defaultFilename = "bao_cao_thang_" + std::to_string(year) + "_" + std::to_string(month);
                break;
            }
            case 4:
            {
                std::string startDate = selectDate();
                std::string endDate = selectDate();
                if (startDate.empty() || endDate.empty())
                    return;
                reportContent = m_facade->generateRevenueReport(startDate, endDate, "txt");
                defaultFilename = "bao_cao_doanh_thu_" + startDate + "_" + endDate;
                break;
            }
            }

            std::cout << "\nChọn định dạng xuất:\n";
            std::cout << "1. Text (.txt)\n";
            std::cout << "2. CSV (.csv)\n";
            std::cout << "3. HTML (.html)\n";
            std::cout << "0. Quay lại\n\n";

            int formatChoice = DisplayHelper::getIntInput("Chọn định dạng [0-3]", 0, 3);
            if (formatChoice <= 0)
                return;

            std::string format;
            std::string extension;
            switch (formatChoice)
            {
            case 1:
                format = "txt";
                extension = ".txt";
                break;
            case 2:
                format = "csv";
                extension = ".csv";
                break;
            case 3:
                format = "html";
                extension = ".html";
                break;
            }

            std::string filename = DisplayHelper::getInput("Nhập tên file (mặc định: " + defaultFilename + ")");
            if (filename.empty())
            {
                filename = defaultFilename;
            }
            filename += extension;

            if (m_facade->exportReport(reportContent, filename, format))
            {
                DisplayHelper::printSuccess("Xuất báo cáo thành công: data/reports/" + filename);
            }
            else
            {
                DisplayHelper::printError("Xuất báo cáo thất bại.");
            }
            DisplayHelper::pause();
        }

        // ==================== Prescription Management Sub-menu (Doctor) ====================

        void ConsoleUI::showPrescriptionMenu()
        {
            while (m_isRunning && m_facade->isLoggedIn())
            {
                DisplayHelper::clearScreen();
                DisplayHelper::printPrescriptionMenu();

                std::string inputChoice;
                bool isValid = false;

                do
                {
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-4]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 4);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ.");
                    }
                } while (!isValid);

                int choice = InputValidator::parseMenuChoice(inputChoice);

                switch (choice)
                {
                case 0:
                    return;
                case 1:
                    createPrescription();
                    break;
                case 2:
                    viewPrescriptions();
                    break;
                case 3:
                    dispensePrescription();
                    break;
                case 4:
                    printPrescription();
                    break;
                }
            }
        }

        void ConsoleUI::createPrescription()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("KÊ ĐƠN THUỐC MỚI");
            std::cout << "(Nhấn Enter để quay lại)\n\n";

            // Get patient
            std::string patientID = selectPatient();
            if (patientID.empty())
                return;

            // Get today's completed appointments for this patient
            auto patient = m_facade->getPatientByID(patientID);
            if (!patient.has_value())
            {
                DisplayHelper::printError("Không tìm thấy bệnh nhân.");
                DisplayHelper::pause();
                return;
            }

            // Get diagnosis
            std::string diagnosis = DisplayHelper::getInput("Chẩn đoán");
            if (diagnosis.empty())
                return;

            std::string instructions = DisplayHelper::getInput("Hướng dẫn chung (tùy chọn)");

            // Get current doctor ID
            std::string doctorID;
            auto doctors = m_facade->getAllDoctors();
            for (const auto& doc : doctors)
            {
                if (doc.getUsername() == m_facade->getCurrentUsername())
                {
                    doctorID = doc.getDoctorID();
                    break;
                }
            }

            if (doctorID.empty())
            {
                DisplayHelper::printError("Không tìm thấy thông tin bác sĩ.");
                DisplayHelper::pause();
                return;
            }

            // Generate prescription ID
            auto allPresc = m_facade->getDoctorPrescriptions(doctorID);
            int nextNum = allPresc.size() + 1;
            char idBuffer[10];
            snprintf(idBuffer, sizeof(idBuffer), "PRE%03d", nextNum);
            std::string prescriptionID = idBuffer;

            // Get current date
            time_t now = time(nullptr);
            tm *ltm = localtime(&now);
            char dateBuffer[20];
            snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d",
                     1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
            std::string today = dateBuffer;

            // Create prescription first
            if (!m_facade->createPrescription(prescriptionID, patient->getUsername(), doctorID, "", today, diagnosis, instructions))
            {
                DisplayHelper::printError("Tạo đơn thuốc thất bại.");
                DisplayHelper::pause();
                return;
            }

            // Add medicines to prescription
            DisplayHelper::printSubHeader("THÊM THUỐC VÀO ĐƠN");
            std::cout << "Nhấn Enter để kết thúc thêm thuốc.\n\n";

            while (true)
            {
                std::string medicineID = selectMedicine();
                if (medicineID.empty())
                    break;

                auto medicine = m_facade->getMedicineByID(medicineID);
                if (!medicine.has_value())
                    continue;

                std::string dosage = DisplayHelper::getInput("Liều dùng (vd: 1 viên x 3 lần/ngày)");
                if (dosage.empty())
                    continue;

                std::string duration = DisplayHelper::getInput("Thời gian dùng (vd: 5 ngày)");
                if (duration.empty())
                    continue;

                int quantity = DisplayHelper::getIntInput("Số lượng", 1, 1000);
                if (quantity < 0)
                    continue;

                if (m_facade->addPrescriptionItem(prescriptionID, medicineID, dosage, duration, quantity))
                {
                    DisplayHelper::printSuccess("Đã thêm " + medicine->getName() + " vào đơn thuốc.");
                }
                else
                {
                    DisplayHelper::printError("Thêm thuốc thất bại.");
                }

                if (!DisplayHelper::confirm("Tiếp tục thêm thuốc khác?"))
                    break;
            }

            DisplayHelper::printSuccess("Tạo đơn thuốc thành công. Mã đơn: " + prescriptionID);
            DisplayHelper::pause();
        }

        void ConsoleUI::viewPrescriptions()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANH SÁCH ĐƠN THUỐC");

            // Get current doctor ID
            std::string doctorID;
            auto doctors = m_facade->getAllDoctors();
            for (const auto& doc : doctors)
            {
                if (doc.getUsername() == m_facade->getCurrentUsername())
                {
                    doctorID = doc.getDoctorID();
                    break;
                }
            }

            auto prescriptions = m_facade->getDoctorPrescriptions(doctorID);
            if (prescriptions.empty())
            {
                DisplayHelper::printNoData("đơn thuốc");
            }
            else
            {
                DisplayHelper::printPrescriptionTable(prescriptions);
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::dispensePrescription()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("CẤP PHÁT THUỐC THEO ĐƠN");

            std::string prescriptionID = selectPrescription();
            if (prescriptionID.empty())
                return;

            auto prescription = m_facade->getPrescriptionByID(prescriptionID);
            if (!prescription.has_value())
            {
                DisplayHelper::printError("Không tìm thấy đơn thuốc.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::printPrescriptionInfo(prescription.value());

            if (prescription->isDispensed())
            {
                DisplayHelper::printWarning("Đơn thuốc này đã được cấp phát trước đó.");
                DisplayHelper::pause();
                return;
            }

            if (!DisplayHelper::confirm("Xác nhận cấp phát thuốc theo đơn này?"))
            {
                DisplayHelper::printInfo("Đã hủy.");
                DisplayHelper::pause();
                return;
            }

            if (m_facade->dispensePrescription(prescriptionID))
            {
                DisplayHelper::printSuccess("Cấp phát thuốc thành công. Tồn kho đã được cập nhật.");
            }
            else
            {
                DisplayHelper::printError("Cấp phát thất bại. Có thể không đủ tồn kho.");
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::printPrescription()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("IN ĐƠN THUỐC");

            std::string prescriptionID = selectPrescription();
            if (prescriptionID.empty())
                return;

            auto prescription = m_facade->getPrescriptionByID(prescriptionID);
            if (!prescription.has_value())
            {
                DisplayHelper::printError("Không tìm thấy đơn thuốc.");
                DisplayHelper::pause();
                return;
            }

            DisplayHelper::clearScreen();
            std::cout << prescription->toPrintFormat() << "\n";
            DisplayHelper::pause();
        }

        // ==================== Patient View-Only Features ====================

        void ConsoleUI::viewMyPrescriptions()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("ĐƠN THUỐC CỦA TÔI");

            auto profile = m_facade->getMyProfile();
            if (!profile.has_value())
            {
                DisplayHelper::printError("Không thể tải thông tin.");
                DisplayHelper::pause();
                return;
            }

            auto prescriptions = m_facade->getPatientPrescriptions(profile->getPatientID());
            if (prescriptions.empty())
            {
                DisplayHelper::printNoData("đơn thuốc");
            }
            else
            {
                DisplayHelper::printPrescriptionTable(prescriptions);

                // Allow viewing details
                std::cout << "\nNhập số thứ tự để xem chi tiết (0 để quay lại): ";
                std::string inputChoice;
                std::getline(std::cin, inputChoice);
                inputChoice = Utils::trim(inputChoice);

                if (!inputChoice.empty() && inputChoice != "0")
                {
                    try
                    {
                        int choice = std::stoi(inputChoice);
                        if (choice > 0 && choice <= static_cast<int>(prescriptions.size()))
                        {
                            DisplayHelper::clearScreen();
                            DisplayHelper::printPrescriptionInfo(prescriptions[choice - 1]);
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
            DisplayHelper::pause();
        }

        void ConsoleUI::searchMedicinePatient()
        {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("TRA CỨU THUỐC");

            std::string keyword = DisplayHelper::getInput("Nhập tên thuốc hoặc danh mục");
            if (keyword.empty())
                return;

            auto medicines = m_facade->searchMedicines(keyword);
            if (medicines.empty())
            {
                DisplayHelper::printNoData("thuốc phù hợp");
            }
            else
            {
                // Display limited info for patients (no stock/reorder info)
                std::vector<std::string> headers = {"STT", "Tên thuốc", "Danh mục", "Dạng bào chế", "Liều lượng"};
                std::vector<std::vector<std::string>> rows;

                for (size_t i = 0; i < medicines.size(); ++i)
                {
                    rows.push_back({
                        std::to_string(i + 1),
                        medicines[i].getName(),
                        medicines[i].getCategory(),
                        medicines[i].getDosageForm(),
                        medicines[i].getStrength()});
                }

                DisplayHelper::printTable(headers, rows, {5, 30, 20, 15, 15});

                // Allow viewing details
                std::cout << "\nNhập số thứ tự để xem chi tiết (0 để quay lại): ";
                std::string inputChoice;
                std::getline(std::cin, inputChoice);
                inputChoice = Utils::trim(inputChoice);

                if (!inputChoice.empty() && inputChoice != "0")
                {
                    try
                    {
                        int choice = std::stoi(inputChoice);
                        if (choice > 0 && choice <= static_cast<int>(medicines.size()))
                        {
                            DisplayHelper::clearScreen();
                            auto& med = medicines[choice - 1];
                            DisplayHelper::printSubHeader("THÔNG TIN THUỐC");
                            std::cout << "Tên thuốc: " << med.getName() << "\n";
                            std::cout << "Tên khoa học: " << med.getGenericName() << "\n";
                            std::cout << "Danh mục: " << med.getCategory() << "\n";
                            std::cout << "Nhà sản xuất: " << med.getManufacturer() << "\n";
                            std::cout << "Dạng bào chế: " << med.getDosageForm() << "\n";
                            std::cout << "Liều lượng: " << med.getStrength() << "\n";
                            if (!med.getDescription().empty())
                            {
                                std::cout << "Mô tả: " << med.getDescription() << "\n";
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
            DisplayHelper::pause();
        }

    } // namespace UI
} // namespace HMS
