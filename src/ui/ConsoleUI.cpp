#include "../../include/ui/ConsoleUI.h"


namespace HMS {
    namespace UI {
// ===================== CONSTRUCTOR ===================================
        ConsoleUI::ConsoleUI() {
            m_isRunning = false;
             m_facade = HMSFacade::getInstance();
        }
// ====================== DESTRUCTOR ==================================
        ConsoleUI::~ConsoleUI() {
            // do nothing
        }
// ====================== MAIN LOOP ===================================
        void ConsoleUI::run() {
            m_isRunning = true;
            if (!m_facade) {
                m_facade = HMSFacade::getInstance();
            }
            while (m_isRunning) {
                showMainMenu();
            }
        }
        void ConsoleUI::showMainMenu() {
        DisplayHelper::clearScreen();
        DisplayHelper::printMainMenu();

        std::string inputChoice;
        bool isValid = false;

        do {
            inputChoice = DisplayHelper::getInput("Nhap lua chon cua ban [0-2]");
            isValid = InputValidator::validateMenuChoice(inputChoice, 0, 2);

            if (!isValid) {
                DisplayHelper::printError("Lua chon khong hop le. Vui long nhap so tu 0 den 2.");
            }
        } while (!isValid);

        int choice = InputValidator::parseMenuChoice(inputChoice);
        handleMainMenuChoice(choice);
    }

    void ConsoleUI::showLoginScreen() {
            DisplayHelper::clearScreen();
            DisplayHelper::printHeader("DANG NHAP HE THONG");
            std::cout << "(De quay lai menu chinh, hay bo trong Username va nhan Enter)\n\n";

            std::string username, password;
            while (true) {
                username = DisplayHelper::getInput("Nhap ten nguoi dung");

                if (username.empty()) return;

                password = DisplayHelper::getInput("Nhap mat khau");

                bool isUserValid = InputValidator::validateUsername(username);
                bool isPassValid = InputValidator::validatePassword(password);

                if (isUserValid && isPassValid) {
                    break;
                }
                if (!isUserValid) {
                    std::cout << InputValidator::getUsernameError(username) << "\n";
                }
                if (!isPassValid) {
                    std::cout << InputValidator::getPasswordError(password) << "\n";
                }
                std::cout << "Vui long thu lai.\n";
            }

            if (m_facade->login(username, password)) {
                DisplayHelper::printSuccess("Dang nhap thanh cong");
                DisplayHelper::pause();
                routeToRoleMenu();
            } else {
                DisplayHelper::printError("Dang nhap khong thanh cong. Ten nguoi dung/mat khau khong ton tai\n");
                showMainMenu();
            }
    }
    // ======================== MENU HANDLERS ========================
        void ConsoleUI::handleMainMenuChoice(int choice) {
            switch (choice){
                case 0:
                    m_facade->shutdown();
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
// ============================ HELPER METHODS =================
        void ConsoleUI::routeToRoleMenu() {
            Role role = m_facade->getCurrentRole();

            switch (role) {
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
                    DisplayHelper::printError("Tai khoan khong xac dinh");
                    m_facade->logout();
                    break;
            }
        }
    }
}
