#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <functional>

namespace HMS {

// ==================== Role Enum ====================
enum class Role {
    PATIENT,
    DOCTOR,
    ADMIN,
    UNKNOWN
};

// Role conversion utilities
inline std::string roleToString(Role role) {
    switch (role) {
        case Role::PATIENT: return "patient";
        case Role::DOCTOR:  return "doctor";
        case Role::ADMIN:   return "admin";
        default:            return "unknown";
    }
}

inline Role stringToRole(const std::string& str) {
    if (str == "patient") return Role::PATIENT;
    if (str == "doctor")  return Role::DOCTOR;
    if (str == "admin")   return Role::ADMIN;
    return Role::UNKNOWN;
}

// ==================== Appointment Status Enum ====================
enum class AppointmentStatus {
    SCHEDULED,
    COMPLETED,
    CANCELLED,
    NO_SHOW,
    UNKNOWN
};

// Status conversion utilities
inline std::string statusToString(AppointmentStatus status) {
    switch (status) {
        case AppointmentStatus::SCHEDULED:  return "scheduled";
        case AppointmentStatus::COMPLETED:  return "completed";
        case AppointmentStatus::CANCELLED:  return "cancelled";
        case AppointmentStatus::NO_SHOW:    return "no_show";
        default:                            return "unknown";
    }
}

inline AppointmentStatus stringToStatus(const std::string& str) {
    if (str == "scheduled")  return AppointmentStatus::SCHEDULED;
    if (str == "completed")  return AppointmentStatus::COMPLETED;
    if (str == "cancelled")  return AppointmentStatus::CANCELLED;
    if (str == "no_show")    return AppointmentStatus::NO_SHOW;
    return AppointmentStatus::UNKNOWN;
}

// ==================== Gender Enum ====================
enum class Gender {
    MALE,
    FEMALE,
    OTHER,
    UNKNOWN
};

inline std::string genderToString(Gender gender) {
    switch (gender) {
        case Gender::MALE:   return "Male";
        case Gender::FEMALE: return "Female";
        case Gender::OTHER:  return "Other";
        default:             return "Unknown";
    }
}

inline Gender stringToGender(const std::string& str) {
    if (str == "Male" || str == "male")     return Gender::MALE;
    if (str == "Female" || str == "female") return Gender::FEMALE;
    if (str == "Other" || str == "other")   return Gender::OTHER;
    return Gender::UNKNOWN;
}

// ==================== Type Aliases ====================
using ID = std::string;
using Username = std::string;
using PasswordHash = std::string;
using Date = std::string;      // Format: YYYY-MM-DD
using Time = std::string;      // Format: HH:MM
using Phone = std::string;
using Money = double;

// ==================== Result Types ====================
template<typename T>
using Result = std::optional<T>;

template<typename T>
using List = std::vector<T>;

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using UniquePtr = std::unique_ptr<T>;

// ==================== Callback Types ====================
using VoidCallback = std::function<void()>;
using BoolCallback = std::function<bool()>;
using StringCallback = std::function<std::string()>;

} // namespace HMS
