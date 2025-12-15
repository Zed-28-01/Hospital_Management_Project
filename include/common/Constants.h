#pragma once

#include <string>

namespace HMS {
namespace Constants {

// ==================== File Paths ====================
constexpr const char* DATA_DIR = "data/";
constexpr const char* BACKUP_DIR = "data/backup/";

constexpr const char* ACCOUNT_FILE = "data/Account.txt";
constexpr const char* PATIENT_FILE = "data/Patient.txt";
constexpr const char* DOCTOR_FILE = "data/Doctor.txt";
constexpr const char* APPOINTMENT_FILE = "data/Appointment.txt";

// ==================== Field Delimiters ====================
constexpr char FIELD_DELIMITER = '|';
constexpr char COMMENT_CHAR = '#';

// ==================== Validation Rules ====================
constexpr int MIN_USERNAME_LENGTH = 3;
constexpr int MAX_USERNAME_LENGTH = 50;
constexpr int MIN_PASSWORD_LENGTH = 6;
constexpr int MAX_PASSWORD_LENGTH = 100;
constexpr int PHONE_LENGTH = 10;

// ==================== ID Prefixes ====================
constexpr const char* PATIENT_ID_PREFIX = "P";
constexpr const char* DOCTOR_ID_PREFIX = "D";
constexpr const char* ADMIN_ID_PREFIX = "A";
constexpr const char* APPOINTMENT_ID_PREFIX = "APT";

// ==================== Date/Time Formats ====================
constexpr const char* DATE_FORMAT = "YYYY-MM-DD";
constexpr const char* TIME_FORMAT = "HH:MM";
constexpr const char* DATETIME_FORMAT = "YYYY-MM-DD HH:MM";

// ==================== Menu Options ====================
namespace Menu {
    constexpr int EXIT = 0;
    constexpr int LOGIN = 1;
    constexpr int REGISTER = 2;

    namespace Patient {
        constexpr int BOOK_APPOINTMENT = 1;
        constexpr int VIEW_APPOINTMENTS = 2;
        constexpr int VIEW_UPCOMING = 3;
        constexpr int CANCEL_APPOINTMENT = 4;
        constexpr int VIEW_BILL = 5;
        constexpr int LOGOUT = 6;
    }

    namespace Doctor {
        constexpr int ADD_PATIENT = 1;
        constexpr int SEARCH_PATIENT = 2;
        constexpr int EDIT_PATIENT = 3;
        constexpr int DELETE_PATIENT = 4;
        constexpr int LIST_PATIENTS = 5;
        constexpr int VIEW_SCHEDULE = 6;
        constexpr int LOGOUT = 7;
    }

    namespace Admin {
        constexpr int ADD_DOCTOR = 1;
        constexpr int SEARCH_DOCTOR = 2;
        constexpr int EDIT_DOCTOR = 3;
        constexpr int DELETE_DOCTOR = 4;
        constexpr int LIST_DOCTORS = 5;
        constexpr int VIEW_STATISTICS = 6;
        constexpr int LOGOUT = 7;
    }
}

// ==================== Messages ====================
namespace Messages {
    constexpr const char* WELCOME = "=== Hospital Management System ===";
    constexpr const char* LOGIN_SUCCESS = "Login successful!";
    constexpr const char* LOGIN_FAILED = "Invalid username or password.";
    constexpr const char* REGISTER_SUCCESS = "Registration successful!";
    constexpr const char* REGISTER_FAILED = "Registration failed. Username may already exist.";
    constexpr const char* LOGOUT_SUCCESS = "Logged out successfully.";
    constexpr const char* INVALID_CHOICE = "Invalid choice. Please try again.";
    constexpr const char* OPERATION_SUCCESS = "Operation completed successfully.";
    constexpr const char* OPERATION_FAILED = "Operation failed. Please try again.";
    constexpr const char* NOT_FOUND = "Record not found.";
    constexpr const char* CONFIRM_DELETE = "Are you sure you want to delete? (y/n): ";
}

} // namespace Constants
} // namespace HMS
