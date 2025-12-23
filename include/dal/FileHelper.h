#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <optional>

namespace HMS
{
    namespace DAL
    {

        /**
         * @class FileHelper
         * @brief Utility class for file I/O operations
         *
         * Provides low-level file operations used by repositories
         * for reading and writing data files.
         */
        class FileHelper
        {
        public:
            // ==================== Read Operations ====================

            /**
             * @brief Read all lines from a file
             * @param filePath Path to the file
             * @return Vector of lines (empty lines and comments excluded)
             */
            static std::vector<std::string> readLines(const std::string &filePath);

            /**
             * @brief Read all lines including empty lines and comments
             * @param filePath Path to the file
             * @return Vector of all lines
             */
            static std::vector<std::string> readAllLines(const std::string &filePath);

            /**
             * @brief Read entire file content as string
             * @param filePath Path to the file
             * @return File content or nullopt if read fails
             */
            static std::optional<std::string> readFile(const std::string &filePath);

            // ==================== Write Operations ====================

            /**
             * @brief Write lines to a file (overwrites existing content)
             * @param filePath Path to the file
             * @param lines Vector of lines to write
             * @return True if successful
             */
            static bool writeLines(const std::string &filePath,
                                   const std::vector<std::string> &lines);

            /**
             * @brief Write string content to a file
             * @param filePath Path to the file
             * @param content Content to write
             * @return True if successful
             */
            static bool writeFile(const std::string &filePath,
                                  const std::string &content);

            /**
             * @brief Append a single line to a file
             * @param filePath Path to the file
             * @param line Line to append
             * @return True if successful
             */
            static bool appendLine(const std::string &filePath,
                                   const std::string &line);

            /**
             * @brief Append multiple lines to a file
             * @param filePath Path to the file
             * @param lines Lines to append
             * @return True if successful
             */
            static bool appendLines(const std::string &filePath,
                                    const std::vector<std::string> &lines);

            // ==================== File Management ====================

            /**
             * @brief Check if a file exists
             * @param filePath Path to the file
             * @return True if file exists
             */
            static bool fileExists(const std::string &filePath);

            /**
             * @brief Create a file if it doesn't exist
             * @param filePath Path to the file
             * @return True if file exists or was created
             */
            static bool createFileIfNotExists(const std::string &filePath);

            /**
             * @brief Create a directory if it doesn't exist
             * @param dirPath Path to the directory
             * @return True if directory exists or was created
             */
            static bool createDirectoryIfNotExists(const std::string &dirPath);

            /**
             * @brief Delete a file
             * @param filePath Path to the file
             * @return True if file was deleted
             */
            static bool deleteFile(const std::string &filePath);

            /**
             * @brief Copy a file
             * @param sourcePath Source file path
             * @param destPath Destination file path
             * @return True if copy was successful
             */
            static bool copyFile(const std::string &sourcePath,
                                 const std::string &destPath);

            // ==================== Backup Operations ====================

            /**
             * @brief Create a backup of a file
             * @param filePath Path to the file to backup
             * @return True if backup was created
             *
             * Creates a copy with timestamp suffix in backup directory
             */
            static bool createBackup(const std::string &filePath);

            /**
             * @brief Restore from the latest backup
             * @param filePath Path to the file to restore
             * @return True if restore was successful
             */
            static bool restoreFromBackup(const std::string &filePath);

            /**
             * @brief Get backup file path for a given file
             * @param filePath Original file path
             * @return Backup file path with timestamp
             */
            static std::string getBackupPath(const std::string &filePath);

            // ==================== Utility Methods ====================

            /**
             * @brief Check if a line is a comment (starts with #)
             * @param line The line to check
             * @return True if line is a comment
             */
            static bool isComment(const std::string &line);

            /**
             * @brief Check if a line is empty or whitespace only
             * @param line The line to check
             * @return True if line is empty
             */
            static bool isEmpty(const std::string &line);

            /**
             * @brief Get the header comment for a data file
             * @param fileType Type of file (Account, Patient, Doctor, Appointment)
             * @return Header comment string describing file format
             */
            static std::string getFileHeader(const std::string &fileType);

        private:
            /**
             * @brief Private constructor to prevent instantiation
             */
            FileHelper() = default;
        };

    } // namespace DAL
} // namespace HMS
