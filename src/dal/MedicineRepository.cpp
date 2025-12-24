#include "dal/MedicineRepository.h"
#include "dal/FileHelper.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <algorithm>
#include <format>
#include <sstream>
#include <filesystem>

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

        // ==================== CRUD Operations ====================
        std::vector<Model::Medicine> MedicineRepository::getAll()
        {
            if (!m_isLoaded)
            {
                load();
            }
            return m_medicines;
        }

        std::optional<Model::Medicine> MedicineRepository::getById(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

            // Check if medicine ID already exists
            if (exists(medicine.getMedicineID()))
            {
                return false;
            }

            std::string targetName = Utils::trim(Utils::toLower(medicine.getName()));
            std::string targetMfr = Utils::trim(Utils::toLower(medicine.getManufacturer()));

            auto duplicate = std::ranges::find_if(
                m_medicines,
                [&](const auto &med) {
                    return Utils::trim(Utils::toLower(med.getName())) == targetName &&
                            Utils::trim(Utils::toLower(med.getManufacturer())) == targetMfr;
                }
            );

            if (duplicate != m_medicines.end())
            {
                return false;
            }

            m_medicines.push_back(medicine);

            return save();
        }

        bool MedicineRepository::update(const Model::Medicine &medicine)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::ranges::find_if(
                m_medicines, [&medicine](const auto &med) {
                    return med.getMedicineID() == medicine.getMedicineID();
                }
            );

            if (it == m_medicines.end())
            {
                return false; // Not found
            }

            std::string targetName = Utils::trim(Utils::toLower(medicine.getName()));
            std::string targetMfr = Utils::trim(Utils::toLower(medicine.getManufacturer()));

            auto duplicate = std::ranges::find_if(
                m_medicines,
                [&](const auto &med) {
                    return med.getMedicineID() != medicine.getMedicineID() &&
                        Utils::trim(Utils::toLower(med.getName())) == targetName &&
                        Utils::trim(Utils::toLower(med.getManufacturer())) == targetMfr;
                }
            );

            if (duplicate != m_medicines.end())
            {
                return false;
            }

            *it = medicine;
            return save();
        }

        bool MedicineRepository::remove(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            return save();
        }

        // ==================== Persistence ====================
        bool MedicineRepository::save()
        {
            try
            {
                std::vector<std::string> lines;

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

        bool MedicineRepository::load() {
            try
            {
                std::filesystem::path path(m_filePath);
                FileHelper::createDirectoryIfNotExists(path.parent_path().string());

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
            if (!m_isLoaded)
            {
                const_cast<MedicineRepository*>(this)->load();
            }

            return m_medicines.size();
        }

        bool MedicineRepository::exists(const std::string &id) const
        {
            if (!m_isLoaded)
            {
                const_cast<MedicineRepository*>(this)->load();
            }

            return std::ranges::any_of(
                m_medicines, [&id](const auto &med) {
                    return med.getMedicineID() == id;
                }
            );
        }

        bool MedicineRepository::clear()
        {
            m_medicines.clear();
            m_isLoaded = true;
            return save();
        }

        // ==================== Medicine-Specific Queries ====================
        std::vector<Model::Medicine> MedicineRepository::getByCategory(const std::string &category)
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                    [](const auto &med) {
                        return med.isExpired();
                    }
            );
            return result;
        }

        std::vector<Model::Medicine> MedicineRepository::getExpiringSoon(int days)
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Model::Medicine> result;
            std::ranges::copy_if(
                m_medicines, std::back_inserter(result),
                [&keyword](const auto &med)
                {
                    return Utils::containsIgnoreCase(med.getName(), keyword) ||
                           Utils::containsIgnoreCase(med.getGenericName(), keyword) ||
                           Utils::containsIgnoreCase(med.getCategory(), keyword) ||
                           Utils::containsIgnoreCase(med.getManufacturer(), keyword);
                }
            );
            return result;
        }

        std::vector<std::string> MedicineRepository::getAllCategories()
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

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
            if (!m_isLoaded)
            {
                load();
            }

            if (m_medicines.empty())
            {
                return std::format("{}001", Constants::MEDICINE_ID_PREFIX);
            }

            // Find max ID number
            int maxNum = 0;
            for (const auto &med : m_medicines)
            {
                const auto &id = med.getMedicineID();
                if (id.starts_with(Constants::MEDICINE_ID_PREFIX))
                {
                    try
                    {
                        std::string prefix = Constants::MEDICINE_ID_PREFIX;
                        int num = std::stoi(id.substr(prefix.length()));
                        maxNum = std::max(maxNum, num);
                    }
                    catch (...)
                    {
                        // Skip invalid IDs
                    }
                }
            }

            return std::format("{}{:03d}", Constants::MEDICINE_ID_PREFIX, maxNum + 1);
        }

        // ==================== Stock Operations ====================
        bool MedicineRepository::updateStock(const std::string &id, int quantity)
        {
            if (!m_isLoaded)
            {
                load();
            }

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
            return save();
        }

        // ==================== File Path Management ====================
        void MedicineRepository::setFilePath(const std::string &filePath)
        {
            m_filePath = filePath;
            m_isLoaded = false;
        }

        std::string MedicineRepository::getFilePath() const
        {
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS