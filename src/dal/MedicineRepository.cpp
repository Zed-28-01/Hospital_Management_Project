#include "dal/MedicineRepository.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "dal/FileHelper.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <sstream>

namespace HMS
{
    namespace DAL
    {
        // ==================== Static Members Initialization ====================
        std::unique_ptr<MedicineRepository> MedicineRepository::s_instance = nullptr;
        std::mutex MedicineRepository::s_mutex;

        // ==================== Private Constructor ====================
        MedicineRepository::MedicineRepository()
            : m_filePath(Constants::MEDICINE_FILE), m_isLoaded(false)
        {
        }

        // ==================== Singleton Access ====================
        MedicineRepository *MedicineRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<MedicineRepository>(new MedicineRepository());
            }

            return s_instance.get();
        }

        void MedicineRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Destructor ====================
        MedicineRepository::~MedicineRepository() = default;

        // ==================== Private Helper ====================
        void MedicineRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<MedicineRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Medicine> MedicineRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_medicines;
        }

        std::optional<Model::Medicine> MedicineRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_medicines, [&id](const auto &med) {
                    return med.getMedicineID() == id;
                }
            );

            if (it != m_medicines.end())
            {
                return *it;
            }

            return std::nullopt;
        }

        bool MedicineRepository::add(const Model::Medicine &medicine)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // DAL only checks for duplicate ID - business rules are in BLL
            bool idExists = std::ranges::any_of(
                m_medicines, [&medicine](const auto &med)
                { return med.getMedicineID() == medicine.getMedicineID(); });

            if (idExists)
            {
                return false;
            }

            m_medicines.push_back(medicine);
            return saveInternal();
        }

        bool MedicineRepository::update(const Model::Medicine &medicine)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // DAL only does CRUD - business rules are in BLL
            auto it = std::ranges::find_if(
                m_medicines, [&medicine](const auto &med)
                { return med.getMedicineID() == medicine.getMedicineID(); });

            if (it == m_medicines.end())
            {
                return false; // Not found
            }

            *it = medicine;
            return saveInternal();
        }

        bool MedicineRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(
                m_medicines, [&id](const auto &med) {
                    return med.getMedicineID() == id;
                }
            );

            if (it == m_medicines.end())
            {
                return false;
            }

            m_medicines.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool MedicineRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool MedicineRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Medicine");
                std::stringstream hss(header);
                std::string headerLine;
                while (std::getline(hss, headerLine))
                {
                    if (!headerLine.empty())
                    {
                        lines.push_back(headerLine);
                    }
                }

                // Add data
                for (const auto &medicine : m_medicines)
                {
                    lines.push_back(medicine.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool MedicineRepository::load()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool MedicineRepository::loadInternal()
        {
            try
            {
                // Create parent directory of the file if it doesn't exist
                std::filesystem::path filePath(m_filePath);
                std::filesystem::path parentDir = filePath.parent_path();

                if (!parentDir.empty())
                {
                    FileHelper::createDirectoryIfNotExists(parentDir.string());
                }

                FileHelper::createFileIfNotExists(m_filePath);

                std::vector<std::string> lines = FileHelper::readLines(m_filePath);

                m_medicines.clear();

                for (const auto &line : lines)
                {
                    auto medicine = Model::Medicine::deserialize(line);
                    if (medicine)
                    {
                        m_medicines.push_back(medicine.value());
                    }
                }

                m_isLoaded = true;
                return true;
            }
            catch (...)
            {
                m_isLoaded = false;
                return false;
            }
        }

        // ==================== Query Operations ====================
        size_t MedicineRepository::count() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_medicines.size();
        }

        bool MedicineRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(
                m_medicines, [&id](const auto &med) {
                    return med.getMedicineID() == id;
                }
            );
        }

        bool MedicineRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_medicines.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Medicine-Specific Queries ====================
        std::vector<Model::Medicine> MedicineRepository::getByCategory(const std::string &category)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [&category](const auto &med)
                {
                    return Utils::containsIgnoreCase(med.getCategory(), category);
                }
            );

            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::getLowStock()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [](const auto &med)
                {
                    return med.isLowStock();
                }
            );

            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::getExpired()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [](const auto &med)
                {
                    return med.isExpired();
                }
            );

            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::getExpiringSoon(int days)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [days](const auto &med)
                {
                    return med.isExpiringSoon(days);
                }
            );

            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::searchByName(const std::string &name)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [&name](const auto &med)
                {
                    return Utils::containsIgnoreCase(med.getName(), name) ||
                        Utils::containsIgnoreCase(med.getGenericName(), name);
                }
            );

            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::search(const std::string &keyword)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [&keyword](const auto &med)
                {
                    return Utils::containsIgnoreCase(med.getMedicineID(), keyword) ||
                           Utils::containsIgnoreCase(med.getName(), keyword) ||
                           Utils::containsIgnoreCase(med.getGenericName(), keyword) ||
                           Utils::containsIgnoreCase(med.getCategory(), keyword) ||
                           Utils::containsIgnoreCase(med.getManufacturer(), keyword);
                }
            );

            return result;
        }

        std::vector<std::string> MedicineRepository::getAllCategories()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<std::string> categories;
            for (const auto &med : m_medicines)
            {
                const auto &cat = med.getCategory();
                if (!cat.empty() &&
                    std::ranges::find(categories, cat) == categories.end())
                {
                    categories.push_back(cat);
                }
            }

            std::ranges::sort(categories);
            return categories;
        }

        std::vector<std::string> MedicineRepository::getAllManufacturers()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<std::string> manufacturers;
            for (const auto &med : m_medicines)
            {
                const auto &mfr = med.getManufacturer();
                if (!mfr.empty() &&
                    std::ranges::find(manufacturers, mfr) == manufacturers.end())
                {
                    manufacturers.push_back(mfr);
                }
            }

            std::ranges::sort(manufacturers);
            return manufacturers;
        }

        std::string MedicineRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_medicines.empty())
            {
                return std::format("{}001", Constants::MEDICINE_ID_PREFIX);
            }

            // Find max ID number
            int maxNum = 0;
            const std::string prefix = Constants::MEDICINE_ID_PREFIX;

            for (const auto &med : m_medicines)
            {
                const auto &id = med.getMedicineID();

                // Only process valid format: prefix + digits
                if (id.length() > prefix.length() &&
                    id.starts_with(prefix))
                {
                    std::string numPart = id.substr(prefix.length());

                    // Validate numeric before parsing
                    if (Utils::isNumeric(numPart))
                    {
                        try
                        {
                            int num = std::stoi(numPart);
                            maxNum = std::max(maxNum, num);
                        }
                        catch (const std::exception &)
                        {
                            // Skip invalid IDs
                        }
                    }
                }
            }

            return std::format("{}{:03d}", Constants::MEDICINE_ID_PREFIX, maxNum + 1);
        }

        // ==================== Stock Operations ====================
        bool MedicineRepository::updateStock(const std::string &id, int quantity)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (quantity < 0)
            {
                return false;  // The quantity must be non-negative
            }

            auto it = std::ranges::find_if(
                m_medicines,
                [&id](const auto &med) {
                    return med.getMedicineID() == id;
                }
            );

            if (it == m_medicines.end())
            {
                return false; // Not found
            }

            it->setQuantityInStock(quantity);
            return saveInternal();
        }

        // ==================== File Path Management ====================
        void MedicineRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false; // Force reload with new file
        }

        std::string MedicineRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS