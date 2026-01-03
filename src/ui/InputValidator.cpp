#include "ui/InputValidator.h"
#include "common/Utils.h"
#include "common/Constants.h"

#include <algorithm>
#include <cctype>

namespace HMS
{
    namespace UI
    {

        // ==================== Username Validation ====================

        bool InputValidator::validateUsername(const std::string &username)
        {
            return Utils::isValidUsername(username);
        }

        std::string InputValidator::getUsernameError(const std::string &username)
        {
            if (username.empty())
            {
                return "Tên người dùng không được để trống.";
            }
            if (username.length() < Constants::MIN_USERNAME_LENGTH)
            {
                return "Tên người dùng phải có ít nhất " +
                       std::to_string(Constants::MIN_USERNAME_LENGTH) + " ký tự.";
            }
            if (username.length() > Constants::MAX_USERNAME_LENGTH)
            {
                return "Tên người dùng không được quá " +
                       std::to_string(Constants::MAX_USERNAME_LENGTH) + " ký tự.";
            }
            return "Tên người dùng chỉ có thể chứa chữ cái, số, dấu gạch dưới và dấu chấm.";
        }

        // ==================== Password Validation ====================

        bool InputValidator::validatePassword(const std::string &password)
        {
            return Utils::isValidPassword(password);
        }

        std::string InputValidator::getPasswordError(const std::string &password)
        {
            if (password.empty())
            {
                return "Mật khẩu không được để trống.";
            }
            if (password.length() < Constants::MIN_PASSWORD_LENGTH)
            {
                return "Mật khẩu phải có ít nhất " +
                       std::to_string(Constants::MIN_PASSWORD_LENGTH) + " ký tự.";
            }
            if (password.length() > Constants::MAX_PASSWORD_LENGTH)
            {
                return "Mật khẩu không được quá " +
                       std::to_string(Constants::MAX_PASSWORD_LENGTH) + " ký tự.";
            }
            return "Định dạng mật khẩu không hợp lệ.";
        }

        int InputValidator::getPasswordStrength(const std::string &password)
        {
            int strength = 0;
            if (password.length() >= 8)
                strength++;
            if (password.length() >= 12)
                strength++;

            bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            for (char c : password)
            {
                if (std::isupper(c))
                    hasUpper = true;
                else if (std::islower(c))
                    hasLower = true;
                else if (std::isdigit(c))
                    hasDigit = true;
                else
                    hasSpecial = true;
            }

            if (hasUpper && hasLower)
                strength++;
            if (hasDigit)
                strength++;
            if (hasSpecial)
                strength++;

            return std::min(strength, 4);
        }

        // ==================== Phone Validation ====================

        bool InputValidator::validatePhone(const std::string &phone)
        {
            return Utils::isValidPhone(phone);
        }

        std::string InputValidator::getPhoneError(const std::string &phone)
        {
            if (phone.empty())
            {
                return "Số điện thoại không được để trống.";
            }
            if (phone.length() != Constants::PHONE_LENGTH)
            {
                return "Số điện thoại phải có đúng " +
                       std::to_string(Constants::PHONE_LENGTH) + " chữ số.";
            }
            if (phone[0] != '0')
            {
                return "Số điện thoại phải bắt đầu bằng 0.";
            }
            return "Số điện thoại chỉ có thể chứa các chữ số.";
        }

        // ==================== Date Validation ====================

        bool InputValidator::validateDate(const std::string &date)
        {
            return Utils::isValidDate(date);
        }

        bool InputValidator::validateFutureDate(const std::string &date)
        {
            if (!Utils::isValidDate(date))
                return false;
            // Convert DD-MM-YYYY to YYYY-MM-DD for comparison
            std::string internalDate = Utils::dateFromInput(date);
            return Utils::isFutureDate(internalDate);
        }

        bool InputValidator::validatePastDate(const std::string &date)
        {
            if (!Utils::isValidDate(date))
                return false;
            // Convert DD-MM-YYYY to YYYY-MM-DD for comparison
            std::string internalDate = Utils::dateFromInput(date);
            return !Utils::isFutureDate(internalDate);
        }

        std::string InputValidator::getDateError(const std::string &date)
        {
            if (date.empty())
            {
                return "Ngày không được để trống.";
            }
            if (date.length() != 10)
            {
                return "Ngày phải có định dạng DD-MM-YYYY.";
            }
            return "Ngày không hợp lệ. Vui lòng sử dụng định dạng DD-MM-YYYY với giá trị hợp lệ.";
        }

        // ==================== Time Validation ====================

        bool InputValidator::validateTime(const std::string &time)
        {
            return Utils::isValidTime(time);
        }

        bool InputValidator::validateWorkingHours(const std::string &time)
        {
            if (!Utils::isValidTime(time))
                return false;

            int hour = std::stoi(time.substr(0, 2));
            return hour >= Constants::WORKING_HOUR_START && hour < Constants::WORKING_HOUR_END;
        }

        std::string InputValidator::getTimeError(const std::string &time)
        {
            if (time.empty())
            {
                return "Thời gian không được để trống.";
            }
            if (time.length() != 5)
            {
                return "Thời gian phải có định dạng HH:MM.";
            }
            return "Thời gian không hợp lệ. Vui lòng sử dụng định dạng HH:MM (24 giờ).";
        }

        // ==================== Name Validation ====================

        bool InputValidator::validateName(const std::string &name)
        {
            if (name.length() < 2 || name.length() > 100)
                return false;

            for (char c : name)
            {
                if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ' && c != '-' && c != '\'')
                {
                    // Allow UTF-8 characters (Vietnamese names, etc.)
                    if (static_cast<unsigned char>(c) < 128)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        std::string InputValidator::getNameError(const std::string &name)
        {
            if (name.empty())
            {
                return "Tên không được để trống.";
            }
            if (name.length() < 2)
            {
                return "Tên phải có ít nhất 2 ký tự.";
            }
            if (name.length() > 100)
            {
                return "Tên không được quá 100 ký tự.";
            }
            return "Tên chỉ có thể chứa chữ cái, khoảng trắng, dấu gạch ngang và dấu nháy đơn.";
        }

        // ==================== Gender Validation ====================

        bool InputValidator::validateGender(const std::string &gender)
        {
            std::string lower = Utils::toLower(gender);
            // English
            if (lower == "male" || lower == "female" || lower == "other" ||
                lower == "m" || lower == "f" || lower == "o")
                return true;
            // Vietnamese
            if (lower == "nam" || lower == "nu" || lower == "nữ" || lower == "khac" || lower == "khác")
                return true;
            return false;
        }

        std::string InputValidator::normalizeGender(const std::string &gender)
        {
            std::string lower = Utils::toLower(gender);
            // English
            if (lower == "male" || lower == "m")
                return "Male";
            if (lower == "female" || lower == "f")
                return "Female";
            if (lower == "other" || lower == "o")
                return "Other";
            // Vietnamese
            if (lower == "nam")
                return "Male";
            if (lower == "nu" || lower == "nữ")
                return "Female";
            if (lower == "khac" || lower == "khác")
                return "Other";
            return "Unknown";
        }

        // ==================== Money Validation ====================

        bool InputValidator::validateMoney(const std::string &amount)
        {
            return Utils::isValidMoney(amount);
        }

        double InputValidator::parseMoney(const std::string &amount)
        {
            if (!validateMoney(amount))
                return -1.0;
            try
            {
                return std::stod(amount);
            }
            catch (...)
            {
                return -1.0;
            }
        }

        // ==================== ID Validation ====================

        bool InputValidator::validatePatientID(const std::string &id)
        {
            if (id.empty() || id[0] != 'P')
                return false;
            return Utils::isNumeric(id.substr(1));
        }

        bool InputValidator::validateDoctorID(const std::string &id)
        {
            if (id.empty() || id[0] != 'D')
                return false;
            return Utils::isNumeric(id.substr(1));
        }

        bool InputValidator::validateAppointmentID(const std::string &id)
        {
            if (id.length() < 4 || id.substr(0, 3) != "APT")
                return false;
            return Utils::isNumeric(id.substr(3));
        }

        bool InputValidator::validateMedicineID(const std::string &id)
        {
            if (id.length() < 4 || id.substr(0, 3) != "MED")
                return false;
            return Utils::isNumeric(id.substr(3));
        }

        bool InputValidator::validateDepartmentID(const std::string &id)
        {
            if (id.length() < 4 || id.substr(0, 3) != "DEP")
                return false;
            return Utils::isNumeric(id.substr(3));
        }

        bool InputValidator::validatePrescriptionID(const std::string &id)
        {
            if (id.length() < 4 || id.substr(0, 3) != "PRE")
                return false;
            return Utils::isNumeric(id.substr(3));
        }

        std::string InputValidator::getMedicineIDError(const std::string &id)
        {
            if (id.empty())
            {
                return "Mã thuốc không được để trống.";
            }
            if (id.length() < 4)
            {
                return "Mã thuốc phải có ít nhất 4 ký tự (ví dụ: MED001).";
            }
            if (id.substr(0, 3) != "MED")
            {
                return "Mã thuốc phải bắt đầu bằng 'MED'.";
            }
            return "Mã thuốc phải có định dạng: MED theo sau bằng các chữ số (ví dụ: MED001).";
        }

        std::string InputValidator::getDepartmentIDError(const std::string &id)
        {
            if (id.empty())
            {
                return "Mã khoa không được để trống.";
            }
            if (id.length() < 4)
            {
                return "Mã khoa phải có ít nhất 4 ký tự (ví dụ: DEP001).";
            }
            if (id.substr(0, 3) != "DEP")
            {
                return "Mã khoa phải bắt đầu bằng 'DEP'.";
            }
            return "Mã khoa phải có định dạng: DEP theo sau bằng các chữ số (ví dụ: DEP001).";
        }

        std::string InputValidator::getPrescriptionIDError(const std::string &id)
        {
            if (id.empty())
            {
                return "Mã đơn thuốc không được để trống.";
            }
            if (id.length() < 4)
            {
                return "Mã đơn thuốc phải có ít nhất 4 ký tự (ví dụ: PRE001).";
            }
            if (id.substr(0, 3) != "PRE")
            {
                return "Mã đơn thuốc phải bắt đầu bằng 'PRE'.";
            }
            return "Mã đơn thuốc phải có định dạng: PRE theo sau bằng các chữ số (ví dụ: PRE001).";
        }

        // ==================== Advanced Data Validation ====================

        bool InputValidator::validateQuantity(const std::string &quantity)
        {
            return validateNonNegativeInteger(quantity);
        }

        bool InputValidator::validatePrice(const std::string &price)
        {
            return validatePositiveDouble(price);
        }

        bool InputValidator::validateCategory(const std::string &category)
        {
            return validateNonEmptyString(category, 100);
        }

        bool InputValidator::validateReorderLevel(const std::string &level)
        {
            return validateNonNegativeInteger(level);
        }

        bool InputValidator::validateDosage(const std::string &dosage)
        {
            return validateNonEmptyString(dosage, 200);
        }

        bool InputValidator::validateDuration(const std::string &duration)
        {
            if (duration.empty() || duration.length() > 50)
                return false;

            // Allow formats like: "5 days", "2 weeks", "1 month", etc.
            std::string lower = Utils::toLower(Utils::trim(duration));

            // Simple validation: should contain a number and a time unit
            bool hasDigit = false;
            for (char c : lower)
            {
                if (std::isdigit(c))
                {
                    hasDigit = true;
                    break;
                }
            }

            return hasDigit && (lower.find("day") != std::string::npos ||
                                lower.find("week") != std::string::npos ||
                                lower.find("month") != std::string::npos ||
                                lower.find("year") != std::string::npos);
        }

        bool InputValidator::validateInstructions(const std::string &instructions)
        {
            return validateNonEmptyString(instructions, 500);
        }

        int InputValidator::parseQuantity(const std::string &quantity)
        {
            if (!validateQuantity(quantity))
                return -1;
            try
            {
                int value = std::stoi(quantity);
                return value >= 0 ? value : -1;
            }
            catch (...)
            {
                return -1;
            }
        }

        double InputValidator::parsePrice(const std::string &price)
        {
            if (!validatePrice(price))
                return -1.0;
            try
            {
                double value = std::stod(price);
                return value > 0.0 ? value : -1.0;
            }
            catch (...)
            {
                return -1.0;
            }
        }

        bool InputValidator::validatePositiveInteger(const std::string &value)
        {
            if (!Utils::isNumeric(value))
                return false;
            try
            {
                int num = std::stoi(value);
                return num > 0;
            }
            catch (...)
            {
                return false;
            }
        }

        bool InputValidator::validateNonNegativeInteger(const std::string &value)
        {
            if (!Utils::isNumeric(value))
                return false;
            try
            {
                int num = std::stoi(value);
                return num >= 0;
            }
            catch (...)
            {
                return false;
            }
        }

        bool InputValidator::validatePositiveDouble(const std::string &value)
        {
            if (!Utils::isValidMoney(value))
                return false;
            try
            {
                double num = std::stod(value);
                return num > 0.0;
            }
            catch (...)
            {
                return false;
            }
        }

        bool InputValidator::validateNonEmptyString(const std::string &str, size_t maxLength)
        {
            std::string trimmed = Utils::trim(str);
            return !trimmed.empty() && trimmed.length() <= maxLength;
        }

        // ==================== Menu Choice Validation ====================

        bool InputValidator::validateMenuChoice(const std::string &choice, int min, int max)
        {
            int parsed = parseMenuChoice(choice);
            return parsed >= min && parsed <= max;
        }

        int InputValidator::parseMenuChoice(const std::string &choice)
        {
            if (!Utils::isNumeric(choice))
                return -1;
            try
            {
                return std::stoi(choice);
            }
            catch (...)
            {
                return -1;
            }
        }

        // ==================== General Validation ====================

        bool InputValidator::isEmpty(const std::string &str)
        {
            return Utils::trim(str).empty();
        }

        bool InputValidator::isNumeric(const std::string &str)
        {
            return Utils::isNumeric(str);
        }

        bool InputValidator::isAlphabetic(const std::string &str)
        {
            if (str.empty())
                return false;
            for (char c : str)
            {
                if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ')
                {
                    return false;
                }
            }
            return true;
        }

        std::string InputValidator::sanitizeInput(const std::string &input)
        {
            std::string result = Utils::trim(input);
            // Remove pipe characters to prevent delimiter injection
            std::string sanitized;
            sanitized.reserve(result.size());
            for (char c : result)
            {
                if (c != Constants::FIELD_DELIMITER)
                {
                    sanitized += c;
                }
            }
            return sanitized;
        }

        bool InputValidator::isYesConfirmation(const std::string &input)
        {
            std::string lower = Utils::toLower(Utils::trim(input));
            return lower == "y" || lower == "yes";
        }

    } // namespace UI
} // namespace HMS
