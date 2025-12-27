#include "dal/PrescriptionRepository.h"
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
        std::unique_ptr<PrescriptionRepository> PrescriptionRepository::s_instance =
            nullptr;
        std::mutex PrescriptionRepository::s_mutex;

        // ==================== Private Constructor ====================
        PrescriptionRepository::PrescriptionRepository()
            : m_filePath(Constants::PRESCRIPTION_FILE), m_isLoaded(false) {}

        // ==================== Singleton Access ====================
        PrescriptionRepository *PrescriptionRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance =
                    std::unique_ptr<PrescriptionRepository>(new PrescriptionRepository());
            }
            return s_instance.get();
        }

        void PrescriptionRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_instance.reset();
        }

        // ==================== Destructor ====================
        PrescriptionRepository::~PrescriptionRepository() = default;

        // ==================== Private Helper ====================
        void PrescriptionRepository::ensureLoaded() const
        {
            if (!m_isLoaded)
            {
                const_cast<PrescriptionRepository *>(this)->loadInternal();
            }
        }

        // ==================== CRUD Operations ====================
        std::vector<Model::Prescription> PrescriptionRepository::getAll()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_prescriptions;
        }

        std::optional<Model::Prescription>
        PrescriptionRepository::getById(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(m_prescriptions, [&id](const auto &p)
                                           { return p.getPrescriptionID() == id; });

            if (it != m_prescriptions.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool PrescriptionRepository::add(const Model::Prescription &prescription)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            // Check if prescription ID already exists
            bool idExists = std::ranges::any_of(
                m_prescriptions, [&prescription](const auto &p)
                { return p.getPrescriptionID() == prescription.getPrescriptionID(); });

            if (idExists)
            {
                return false;
            }

            // Check if appointment ID already has a prescription (unless empty)
            const std::string &appointmentID = prescription.getAppointmentID();
            if (!appointmentID.empty())
            {
                bool appointmentExists = std::ranges::any_of(
                    m_prescriptions, [&appointmentID](const auto &p)
                    { return p.getAppointmentID() == appointmentID; });

                if (appointmentExists)
                {
                    return false;
                }
            }

            m_prescriptions.push_back(prescription);
            return saveInternal();
        }

        bool PrescriptionRepository::update(const Model::Prescription &prescription)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it =
                std::ranges::find_if(m_prescriptions, [&prescription](const auto &p)
                                     { return p.getPrescriptionID() == prescription.getPrescriptionID(); });

            if (it != m_prescriptions.end())
            {
                *it = prescription;
                return saveInternal();
            }
            return false;
        }

        bool PrescriptionRepository::remove(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(m_prescriptions, [&id](const auto &p)
                                           { return p.getPrescriptionID() == id; });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            m_prescriptions.erase(it);
            return saveInternal();
        }

        // ==================== Persistence ====================
        bool PrescriptionRepository::save()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return saveInternal();
        }

        bool PrescriptionRepository::saveInternal()
        {
            try
            {
                std::vector<std::string> lines;

                // Add header
                auto header = FileHelper::getFileHeader("Prescription");
                std::stringstream hss(header);
                std::string headerLine;
                while (std::getline(hss, headerLine))
                {
                    if (!headerLine.empty())
                    {
                        lines.push_back(headerLine);
                    }
                }

                // Add data - complex items serialization is handled by
                // Prescription::serialize()
                for (const auto &prescription : m_prescriptions)
                {
                    lines.push_back(prescription.serialize());
                }

                FileHelper::createBackup(m_filePath);
                return FileHelper::writeLines(m_filePath, lines);
            }
            catch (...)
            {
                return false;
            }
        }

        bool PrescriptionRepository::load()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return loadInternal();
        }

        bool PrescriptionRepository::loadInternal()
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

                m_prescriptions.clear();

                for (const auto &line : lines)
                {
                    // Prescription::deserialize() handles complex items parsing
                    auto prescription = Model::Prescription::deserialize(line);
                    if (prescription)
                    {
                        m_prescriptions.push_back(prescription.value());
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
        size_t PrescriptionRepository::count() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();
            return m_prescriptions.size();
        }

        bool PrescriptionRepository::exists(const std::string &id) const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            return std::ranges::any_of(m_prescriptions, [&id](const auto &p)
                                       { return p.getPrescriptionID() == id; });
        }

        bool PrescriptionRepository::clear()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_prescriptions.clear();
            m_isLoaded = true;
            return saveInternal();
        }

        // ==================== Prescription-Specific Queries ====================
        std::optional<Model::Prescription>
        PrescriptionRepository::getByAppointment(const std::string &appointmentID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it =
                std::ranges::find_if(m_prescriptions, [&appointmentID](const auto &p)
                                     { return p.getAppointmentID() == appointmentID; });

            if (it != m_prescriptions.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Model::Prescription>
        PrescriptionRepository::getByPatient(const std::string &patientUsername)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(m_prescriptions, std::back_inserter(results),
                                 [&patientUsername](const auto &p)
                                 {
                                     return p.getPatientUsername() == patientUsername;
                                 });

            // Sort by date descending (most recent first)
            std::ranges::sort(results, [](const auto &a, const auto &b)
                              { return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0; });

            return results;
        }

        std::vector<Model::Prescription>
        PrescriptionRepository::getByDoctor(const std::string &doctorID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(
                m_prescriptions, std::back_inserter(results),
                [&doctorID](const auto &p)
                { return p.getDoctorID() == doctorID; });

            // Sort by date descending (most recent first)
            std::ranges::sort(results, [](const auto &a, const auto &b)
                              { return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0; });

            return results;
        }

        std::vector<Model::Prescription> PrescriptionRepository::getUndispensed()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(m_prescriptions, std::back_inserter(results),
                                 [](const auto &p)
                                 { return !p.isDispensed(); });

            // Sort by date ascending (oldest first - priority for dispensing)
            std::ranges::sort(results, [](const auto &a, const auto &b)
                              { return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) < 0; });

            return results;
        }

        std::vector<Model::Prescription> PrescriptionRepository::getDispensed()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(m_prescriptions, std::back_inserter(results),
                                 [](const auto &p)
                                 { return p.isDispensed(); });

            // Sort by date descending (most recent first)
            std::ranges::sort(results, [](const auto &a, const auto &b)
                              { return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0; });

            return results;
        }

        std::vector<Model::Prescription>
        PrescriptionRepository::getByDate(const std::string &date)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(
                m_prescriptions, std::back_inserter(results),
                [&date](const auto &p)
                { return p.getPrescriptionDate() == date; });

            return results;
        }

        std::vector<Model::Prescription>
        PrescriptionRepository::getByDateRange(const std::string &startDate,
                                               const std::string &endDate)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            std::ranges::copy_if(m_prescriptions, std::back_inserter(results),
                                 [&startDate, &endDate](const auto &p)
                                 {
                                     const std::string &date = p.getPrescriptionDate();
                                     return Utils::compareDates(date, startDate) >= 0 &&
                                            Utils::compareDates(date, endDate) <= 0;
                                 });

            // Sort by date descending (most recent first)
            std::ranges::sort(results, [](const auto &a, const auto &b)
                              { return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0; });

            return results;
        }

        std::vector<Model::Prescription>
        PrescriptionRepository::getByMedicine(const std::string &medicineID)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            std::vector<Model::Prescription> results;
            for (const auto &prescription : m_prescriptions)
            {
                const auto &items = prescription.getItems();
                // Check if any item in this prescription contains the medicineID
                bool hasMedicine =
                    std::ranges::any_of(items, [&medicineID](const auto &item)
                                        { return item.medicineID == medicineID; });

                if (hasMedicine)
                {
                    results.push_back(prescription);
                }
            }

            return results;
        }

        std::string PrescriptionRepository::getNextId()
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            if (m_prescriptions.empty())
            {
                return std::format("{}001", Constants::PRESCRIPTION_ID_PREFIX);
            }

            // Find max ID number
            int maxID = 0;
            const std::string prefix = Constants::PRESCRIPTION_ID_PREFIX;

            for (const auto &prescription : m_prescriptions)
            {
                const std::string &prescriptionID = prescription.getPrescriptionID();

                // Only process valid format: prefix + digits
                if (prescriptionID.length() > prefix.length() &&
                    prescriptionID.starts_with(prefix))
                {
                    std::string numPart = prescriptionID.substr(prefix.length());

                    // Validate numeric before parsing
                    if (Utils::isNumeric(numPart))
                    {
                        try
                        {
                            int idNum = std::stoi(numPart);
                            maxID = std::max(maxID, idNum);
                        }
                        catch (const std::exception &)
                        {
                            // Ignore parse errors
                        }
                    }
                }
            }

            return std::format("{}{:03d}", prefix, maxID + 1);
        }

        // ==================== Dispensing Operations ====================
        bool PrescriptionRepository::markAsDispensed(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(m_prescriptions, [&id](const auto &p)
                                           { return p.getPrescriptionID() == id; });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            // Check if already dispensed
            if (it->isDispensed())
            {
                return true; // Already dispensed, consider it a success
            }

            it->setDispensed(true);
            return saveInternal();
        }

        bool PrescriptionRepository::markAsUndispensed(const std::string &id)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            ensureLoaded();

            auto it = std::ranges::find_if(m_prescriptions, [&id](const auto &p)
                                           { return p.getPrescriptionID() == id; });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            // Check if already undispensed
            if (!it->isDispensed())
            {
                return true; // Already undispensed, consider it a success
            }

            it->setDispensed(false);
            return saveInternal();
        }

        // ==================== File Path Management ====================
        void PrescriptionRepository::setFilePath(const std::string &filePath)
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            m_filePath = filePath;
            m_isLoaded = false; // Force reload with new file
        }

        std::string PrescriptionRepository::getFilePath() const
        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS