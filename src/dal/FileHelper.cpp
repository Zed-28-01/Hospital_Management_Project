#include "dal/FileHelper.h"
#include "common/Constants.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cctype>

namespace fs = std::filesystem;

namespace HMS {
namespace DAL {

// ==================== Read Operations ====================

std::vector<std::string> FileHelper::readLines(const std::string& filePath) {
    std::vector<std::string> result;
    std::ifstream file(filePath);

    if (!file.is_open()) return result;

    std::string line;
    while (std::getline(file, line)) {
        if (isEmpty(line) || isComment(line)) continue;
        result.push_back(line);
    }

    return result;
}

std::vector<std::string> FileHelper::readAllLines(const std::string& filePath) {
    std::vector<std::string> result;
    std::ifstream file(filePath);

    if (!file.is_open()) return result;

    std::string line;
    while (std::getline(file, line)) {
        result.push_back(line);
    }

    return result;
}

std::optional<std::string> FileHelper::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return std::nullopt;

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// ==================== Write Operations ====================

bool FileHelper::writeLines(const std::string& filePath,
                            const std::vector<std::string>& lines) {
    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open()) return false;

    for (const auto& line : lines) {
        file << line << '\n';
    }
    return true;
}

bool FileHelper::writeFile(const std::string& filePath,
                           const std::string& content) {
    std::ofstream file(filePath, std::ios::trunc);
    if (!file.is_open()) return false;

    file << content;
    return true;
}

bool FileHelper::appendLine(const std::string& filePath,
                             const std::string& line) {
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) return false;

    file << line << '\n';
    return true;
}

bool FileHelper::appendLines(const std::string& filePath,
                              const std::vector<std::string>& lines) {
    std::ofstream file(filePath, std::ios::app);
    if (!file.is_open()) return false;

    for (const auto& line : lines) {
        file << line << '\n';
    }
    return true;
}

// ==================== File Management ====================

bool FileHelper::fileExists(const std::string& filePath) {
    return fs::exists(filePath);
}

bool FileHelper::createFileIfNotExists(const std::string& filePath) {
    if (fileExists(filePath)) return true;

    std::ofstream file(filePath);
    return file.is_open();
}

bool FileHelper::createDirectoryIfNotExists(const std::string& dirPath) {
    if (fs::exists(dirPath)) return true;
    return fs::create_directories(dirPath);
}

bool FileHelper::deleteFile(const std::string& filePath) {
    if (!fileExists(filePath)) return false;
    return fs::remove(filePath);
}

bool FileHelper::copyFile(const std::string& sourcePath,
                           const std::string& destPath) {
    if (!fileExists(sourcePath)) return false;

    fs::copy_file(
        sourcePath,
        destPath,
        fs::copy_options::overwrite_existing
    );
    return true;
}

// ==================== Backup Operations ====================

std::string FileHelper::getBackupPath(const std::string& filePath) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream ss;
    ss << fs::path(filePath).stem().string()
       << "_backup_"
       << std::put_time(&tm, "%Y%m%d_%H%M%S")
       << fs::path(filePath).extension().string();

    return (fs::path(HMS::Constants::BACKUP_DIR) / ss.str()).string();
}

bool FileHelper::createBackup(const std::string& filePath) {
    if (!fileExists(filePath)) return false;

    createDirectoryIfNotExists(HMS::Constants::BACKUP_DIR);
    return copyFile(filePath, getBackupPath(filePath));
}

bool FileHelper::restoreFromBackup(const std::string& filePath) {
    fs::path backupDir(HMS::Constants::BACKUP_DIR);
    if (!fs::exists(backupDir)) return false;

    std::string originalStem = fs::path(filePath).stem().string();

    fs::path latestBackup;
    std::time_t latestTime = 0;

    for (const auto& entry : fs::directory_iterator(backupDir)) {
        if (!entry.is_regular_file()) continue;

        std::string backupStem = entry.path().stem().string();

        if (backupStem.rfind(originalStem + "_backup_", 0) != 0)
            continue;

        auto ftime = fs::last_write_time(entry);

        auto sctp =
            std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now()
                + std::chrono::system_clock::now()
            );

        std::time_t tt = std::chrono::system_clock::to_time_t(sctp);

        if (tt > latestTime) {
            latestTime = tt;
            latestBackup = entry.path();
        }
    }

    if (latestBackup.empty()) return false;

    return copyFile(latestBackup.string(), filePath);
}

// ==================== Utility Methods ====================

bool FileHelper::isComment(const std::string& line) {
    return !line.empty() && line[0] == HMS::Constants::COMMENT_CHAR;
}

bool FileHelper::isEmpty(const std::string& line) {
    for (char c : line) {
        if (!std::isspace(static_cast<unsigned char>(c)))
            return false;
    }
    return true;
}

std::string FileHelper::getFileHeader(const std::string& fileType) {
    if (fileType == "Account") {
        return "# username|passwordHash|role|isActive|createdDate";
    }
    if (fileType == "Patient") {
        return "# patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory";
    }
    if (fileType == "Doctor") {
        return "# doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|fee";
    }
    if (fileType == "Appointment") {
        return "# appointmentID|patientUsername|doctorID|date|time|disease|price|isPaid|status|notes";
    }

    return "# Data file";
}

} // namespace DAL
} // namespace HMS
