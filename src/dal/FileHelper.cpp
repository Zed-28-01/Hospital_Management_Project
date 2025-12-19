#include "dal/FileHelper.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

namespace HMS
{
    namespace DAL
    {

        // ==================== Read Operations ====================

        std::vector<std::string> FileHelper::readLines(const std::string &filePath)
        {
            std::vector<std::string> lines;
            std::ifstream file(filePath);

            if (!file.is_open())
            {
                return lines;
            }

            std::string line;
            while (std::getline(file, line))
            {
                // Sử dụng Utils::trim để loại bỏ khoảng trắng thừa
                std::string trimmed = Utils::trim(line);

                // Bỏ qua dòng trống
                if (isEmpty(trimmed))
                    continue;

                // Bỏ qua dòng comment
                if (isComment(trimmed))
                    continue;

                lines.push_back(line); // Có thể lưu 'line' gốc hoặc 'trimmed' tùy nhu cầu
            }

            return lines;
        }

        std::vector<std::string> FileHelper::readAllLines(const std::string &filePath)
        {
            std::vector<std::string> lines;
            std::ifstream file(filePath);

            if (file.is_open())
            {
                std::string line;
                while (std::getline(file, line))
                {
                    lines.push_back(line);
                }
            }
            return lines;
        }

        std::optional<std::string> FileHelper::readFile(const std::string &filePath)
        {
            std::ifstream file(filePath);
            if (!file.is_open())
            {
                return std::nullopt;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        // ==================== Write Operations ====================

        bool FileHelper::writeLines(const std::string &filePath, const std::vector<std::string> &lines)
        {
            // Đảm bảo thư mục tồn tại
            if (!createFileIfNotExists(filePath))
            {
                return false;
            }

            std::ofstream file(filePath, std::ios::trunc); // Xóa nội dung cũ
            if (!file.is_open())
            {
                return false;
            }

            for (const auto &line : lines)
            {
                file << line << "\n";
            }
            return true;
        }

        bool FileHelper::writeFile(const std::string &filePath, const std::string &content)
        {
            if (!createFileIfNotExists(filePath))
            {
                return false;
            }

            std::ofstream file(filePath, std::ios::trunc);
            if (!file.is_open())
            {
                return false;
            }

            file << content;
            return true;
        }

        bool FileHelper::appendLine(const std::string &filePath, const std::string &line)
        {
            if (!createFileIfNotExists(filePath))
            {
                return false;
            }

            std::ofstream file(filePath, std::ios::app); // Chế độ append
            if (!file.is_open())
            {
                return false;
            }

            file << line << "\n";
            return true;
        }

        bool FileHelper::appendLines(const std::string &filePath, const std::vector<std::string> &lines)
        {
            if (!createFileIfNotExists(filePath))
            {
                return false;
            }

            std::ofstream file(filePath, std::ios::app);
            if (!file.is_open())
            {
                return false;
            }

            for (const auto &line : lines)
            {
                file << line << "\n";
            }
            return true;
        }

        // ==================== File Management ====================

        bool FileHelper::fileExists(const std::string &filePath)
        {
            return fs::exists(filePath) && fs::is_regular_file(filePath);
        }

        bool FileHelper::createFileIfNotExists(const std::string &filePath)
        {
            if (fileExists(filePath))
            {
                return true;
            }

            try
            {
                fs::path path(filePath);
                // Tạo thư mục cha nếu chưa có
                if (path.has_parent_path())
                {
                    fs::create_directories(path.parent_path());
                }

                std::ofstream file(filePath);
                return file.is_open();
            }
            catch (...)
            {
                return false;
            }
        }

        bool FileHelper::createDirectoryIfNotExists(const std::string &dirPath)
        {
            if (fs::exists(dirPath))
            {
                return true;
            }
            try
            {
                return fs::create_directories(dirPath);
            }
            catch (...)
            {
                return false;
            }
        }

        bool FileHelper::deleteFile(const std::string &filePath)
        {
            try
            {
                if (fs::exists(filePath))
                {
                    return fs::remove(filePath);
                }
                return true; // File không tồn tại coi như đã xóa thành công
            }
            catch (...)
            {
                return false;
            }
        }

        bool FileHelper::copyFile(const std::string &sourcePath, const std::string &destPath)
        {
            try
            {
                // Đảm bảo thư mục đích tồn tại
                fs::path dest(destPath);
                if (dest.has_parent_path())
                {
                    fs::create_directories(dest.parent_path());
                }

                fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        // ==================== Backup Operations ====================

        std::string FileHelper::getBackupPath(const std::string &filePath)
        {
            fs::path originalPath(filePath);
            std::string filename = originalPath.filename().string();

            // Tạo timestamp an toàn cho tên file (thay : thành -)
            std::string timestamp = Utils::getCurrentDateTime();
            std::replace(timestamp.begin(), timestamp.end(), ':', '-');
            std::replace(timestamp.begin(), timestamp.end(), ' ', '_');

            // Kết quả: data/backup/Appointment.txt_2023-12-19_10-00-00.bak
            fs::path backupDir(Constants::BACKUP_DIR);
            return (backupDir / (filename + "_" + timestamp + ".bak")).string();
        }

        bool FileHelper::createBackup(const std::string &filePath)
        {
            if (!fileExists(filePath))
            {
                return false;
            }

            // Đảm bảo thư mục backup tồn tại
            createDirectoryIfNotExists(Constants::BACKUP_DIR);

            std::string backupPath = getBackupPath(filePath);
            return copyFile(filePath, backupPath);
        }

        bool FileHelper::restoreFromBackup(const std::string &filePath)
        {
            // Logic: Tìm file backup mới nhất khớp với tên file gốc
            try
            {
                if (!fs::exists(Constants::BACKUP_DIR))
                    return false;

                fs::path originalPath(filePath);
                std::string targetName = originalPath.filename().string();

                std::string latestBackup;

                for (const auto &entry : fs::directory_iterator(Constants::BACKUP_DIR))
                {
                    if (entry.is_regular_file())
                    {
                        std::string currentBackupName = entry.path().filename().string();

                        // Kiểm tra xem file backup có bắt đầu bằng tên file gốc không
                        if (currentBackupName.find(targetName) == 0)
                        {
                            // So sánh chuỗi để tìm file mới nhất (vì format timestamp tăng dần theo alphabet)
                            if (latestBackup.empty() || currentBackupName > fs::path(latestBackup).filename().string())
                            {
                                latestBackup = entry.path().string();
                            }
                        }
                    }
                }

                if (!latestBackup.empty())
                {
                    return copyFile(latestBackup, filePath);
                }
            }
            catch (...)
            {
                return false;
            }
            return false;
        }

        // ==================== Utility Methods ====================

        bool FileHelper::isComment(const std::string &line)
        {
            if (line.empty())
                return false;
            // Sử dụng hằng số từ Constants.h
            return line[0] == Constants::COMMENT_CHAR;
        }

        bool FileHelper::isEmpty(const std::string &line)
        {
            return line.empty() ||
                   std::all_of(line.begin(), line.end(), [](unsigned char c)
                               { return std::isspace(c); });
        }

        std::string FileHelper::getFileHeader(const std::string &fileType)
        {
            std::stringstream ss;
            ss << Constants::COMMENT_CHAR << " ==========================================\n";
            ss << Constants::COMMENT_CHAR << " " << fileType << " Data File\n";
            ss << Constants::COMMENT_CHAR << " Format: See documentation or source code\n";
            ss << Constants::COMMENT_CHAR << " Last Updated: " << Utils::getCurrentDateTime() << "\n";
            ss << Constants::COMMENT_CHAR << " ==========================================";
            return ss.str();
        }

    } // namespace DAL
} // namespace HMS