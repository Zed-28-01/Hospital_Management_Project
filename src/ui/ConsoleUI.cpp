#include "ui/ConsoleUI.h"
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
                dateOfBirth = DisplayHelper::getInput("Nhập ngày sinh (YYYY-MM-DD)");
                if (dateOfBirth.empty())
                    return;

                if (!InputValidator::validateDate(dateOfBirth))
                {
                    std::cout << InputValidator::getDateError(dateOfBirth) << "\n";
                    continue;
                }
                if (!InputValidator::validatePastDate(dateOfBirth))
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
                viewStatistics();
                break;
            case 7:
                viewReports();
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
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-6]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 6);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 6.");
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
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-7]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 7);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 7.");
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
                    inputChoice = DisplayHelper::getInput("Nhập lựa chọn của bạn [0-7]");
                    isValid = InputValidator::validateMenuChoice(inputChoice, 0, 7);
                    if (!isValid)
                    {
                        DisplayHelper::printError("Lựa chọn không hợp lệ. Vui lòng nhập số từ 0 đến 7.");
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
                dateOfBirth = DisplayHelper::getInput("Nhập ngày sinh (YYYY-MM-DD)");
                if (dateOfBirth.empty())
                    return;
                if (!InputValidator::validateDate(dateOfBirth) || !InputValidator::validatePastDate(dateOfBirth))
                {
                    DisplayHelper::printError("Ngày sinh không hợp lệ.");
                    continue;
                }
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
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("LỊCH LÀM VIỆC");

            std::string date = selectDate();
            if (date.empty())
                return;

            auto appointments = m_facade->getMySchedule(date);
            if (appointments.empty())
            {
                DisplayHelper::printNoData("lịch hẹn cho ngày này");
            }
            else
            {
                DisplayHelper::printAppointmentTable(appointments);
            }
            DisplayHelper::pause();
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
                dateOfBirth = DisplayHelper::getInput("Nhập ngày sinh (YYYY-MM-DD)");
                if (dateOfBirth.empty())
                    return;
                if (!InputValidator::validateDate(dateOfBirth) || !InputValidator::validatePastDate(dateOfBirth))
                {
                    DisplayHelper::printError("Ngày sinh không hợp lệ.");
                    continue;
                }
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
                date = DisplayHelper::getInput("Nhập ngày (YYYY-MM-DD, bỏ trống để quay lại)");
                if (date.empty())
                    return "";

                if (!InputValidator::validateDate(date))
                {
                    std::cout << InputValidator::getDateError(date) << "\n";
                    continue;
                }
                if (!InputValidator::validateFutureDate(date))
                {
                    DisplayHelper::printError("Ngày phải là hôm nay hoặc trong tương lai.");
                    continue;
                }
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

    } // namespace UI
} // namespace HMS
