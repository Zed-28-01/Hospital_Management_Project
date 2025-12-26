#include "dal/PrescriptionRepository.h"
#include "dal/FileHelper.h"
#include "common/Constants.h"
#include "common/Utils.h"
#include "common/Types.h"

#include <algorithm>
#include <format>
#include <string>

namespace HMS
{
    namespace DAL
    {
        // Using declarations for cleaner code
        using HMS::Date;
        using HMS::ID;
        using HMS::Result;
        using Model::Prescription;
        using Model::PrescriptionItem;

        // ==================== Static Members Initialization ====================

        std::unique_ptr<PrescriptionRepository> PrescriptionRepository::s_instance = nullptr;
        std::mutex PrescriptionRepository::s_mutex;

        // ==================== Constructor / Destructor ====================

        PrescriptionRepository::PrescriptionRepository()
            : m_filePath(Constants::PRESCRIPTION_FILE), m_isLoaded(false)
        {
            FileHelper::createDirectoryIfNotExists(Constants::DATA_DIR);
            FileHelper::createFileIfNotExists(m_filePath);
            // Note: Lazy loading - data will be loaded on first access
        }

        PrescriptionRepository::~PrescriptionRepository()
        {
            // Do NOT save in destructor - let user explicitly save
            // or save will be called in resetInstance()
        }

        // ==================== Singleton Access ====================

        PrescriptionRepository *PrescriptionRepository::getInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_instance)
            {
                s_instance = std::unique_ptr<PrescriptionRepository>(new PrescriptionRepository());
            }
            return s_instance.get();
        }

        void PrescriptionRepository::resetInstance()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (s_instance)
            {
                s_instance->save();
                s_instance.reset();
            }
        }

        // ==================== CRUD Operations ====================

        std::vector<Prescription> PrescriptionRepository::getAll()
        {
            if (!m_isLoaded)
            {
                load();
            }
            return m_prescriptions;
        }

        std::optional<Prescription> PrescriptionRepository::getById(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&id](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == id;
                                   });

            if (it != m_prescriptions.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        bool PrescriptionRepository::add(const Prescription &prescription)
        {
            if (!m_isLoaded)
            {
                load();
            }

            // Check if prescription ID already exists
            const std::string &prescriptionID = prescription.getPrescriptionID();
            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&prescriptionID](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == prescriptionID;
                                   });

            if (it != m_prescriptions.end())
            {
                return false; // Prescription ID already exists
            }

            // Check if appointment already has a prescription
            const std::string &appointmentID = prescription.getAppointmentID();
            if (!appointmentID.empty())
            {
                auto aptIt = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                          [&appointmentID](const Prescription &p)
                                          {
                                              return p.getAppointmentID() == appointmentID;
                                          });

                if (aptIt != m_prescriptions.end())
                {
                    return false; // Appointment already has a prescription
                }
            }

            m_prescriptions.push_back(prescription);
            return save();
        }

        bool PrescriptionRepository::update(const Prescription &prescription)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&prescription](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == prescription.getPrescriptionID();
                                   });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            *it = prescription;
            return save();
        }

        bool PrescriptionRepository::remove(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&id](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == id;
                                   });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            m_prescriptions.erase(it);
            return save();
        }

        // ==================== Persistence ====================

        bool PrescriptionRepository::save()
        {
            std::vector<std::string> lines;

            // Add header
            lines.push_back(FileHelper::getFileHeader("Prescription"));

            // Add each prescription
            for (const auto &prescription : m_prescriptions)
            {
                lines.push_back(prescription.serialize());
            }

            return FileHelper::writeLines(m_filePath, lines);
        }

        bool PrescriptionRepository::load()
        {
            m_prescriptions.clear();

            auto lines = FileHelper::readLines(m_filePath);

            for (const auto &line : lines)
            {
                auto prescriptionOpt = Prescription::deserialize(line);
                if (prescriptionOpt.has_value())
                {
                    m_prescriptions.push_back(prescriptionOpt.value());
                }
            }

            m_isLoaded = true;
            return true;
        }

        // ==================== Query Operations ====================

        size_t PrescriptionRepository::count() const
        {
            // Note: Does not lazy load. Call load() or getAll() first if needed.
            return m_prescriptions.size();
        }

        bool PrescriptionRepository::exists(const std::string &id) const
        {
            // Note: Does not lazy load. Call load() or getAll() first if needed.
            return std::any_of(m_prescriptions.begin(), m_prescriptions.end(),
                               [&id](const Prescription &p)
                               {
                                   return p.getPrescriptionID() == id;
                               });
        }

        bool PrescriptionRepository::clear()
        {
            m_prescriptions.clear();
            m_isLoaded = true; // Still loaded, just empty
            return save();
        }

        // ==================== Prescription-Specific Queries ====================

        std::optional<Prescription> PrescriptionRepository::getByAppointment(const std::string &appointmentID)
        {
            if (!m_isLoaded)
            {
                load();
            }

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&appointmentID](const Prescription &p)
                                   {
                                       return p.getAppointmentID() == appointmentID;
                                   });

            if (it != m_prescriptions.end())
            {
                return *it;
            }
            return std::nullopt;
        }

        std::vector<Prescription> PrescriptionRepository::getByPatient(const std::string &patientUsername)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [&patientUsername](const Prescription &p)
                         {
                             return p.getPatientUsername() == patientUsername;
                         });

            // Sort by date (most recent first)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0;
                      });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getByDoctor(const std::string &doctorID)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [&doctorID](const Prescription &p)
                         {
                             return p.getDoctorID() == doctorID;
                         });

            // Sort by date (most recent first)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0;
                      });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getUndispensed()
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [](const Prescription &p)
                         {
                             return !p.isDispensed();
                         });

            // Sort by date (oldest first - priority for dispensing)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) < 0;
                      });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getDispensed()
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [](const Prescription &p)
                         {
                             return p.isDispensed();
                         });

            // Sort by date (most recent first)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0;
                      });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getByDate(const std::string &date)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;
            const Date &targetDate = date; // Using Date type alias for clarity

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [&targetDate](const Prescription &p)
                         {
                             return p.getPrescriptionDate() == targetDate;
                         });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getByDateRange(const std::string &startDate,
                                                                         const std::string &endDate)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;
            const Date &start = startDate; // Using Date type alias
            const Date &end = endDate;

            std::copy_if(m_prescriptions.begin(), m_prescriptions.end(),
                         std::back_inserter(result),
                         [&start, &end](const Prescription &p)
                         {
                             const Date &prescDate = p.getPrescriptionDate();
                             return Utils::compareDates(prescDate, start) >= 0 &&
                                    Utils::compareDates(prescDate, end) <= 0;
                         });

            // Sort by date (most recent first)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0;
                      });

            return result;
        }

        std::vector<Prescription> PrescriptionRepository::getByMedicine(const std::string &medicineID)
        {
            if (!m_isLoaded)
            {
                load();
            }

            std::vector<Prescription> result;
            const ID &targetMedicineId = medicineID; // Using ID type alias

            for (const auto &prescription : m_prescriptions)
            {
                // Check if any item in prescription contains the medicine
                const auto &items = prescription.getItems();
                bool hasMedicine = std::any_of(items.begin(), items.end(),
                                               [&targetMedicineId](const PrescriptionItem &item)
                                               {
                                                   return item.medicineID == targetMedicineId;
                                               });

                if (hasMedicine)
                {
                    result.push_back(prescription);
                }
            }

            // Sort by date (most recent first)
            std::sort(result.begin(), result.end(),
                      [](const Prescription &a, const Prescription &b)
                      {
                          return Utils::compareDates(a.getPrescriptionDate(), b.getPrescriptionDate()) > 0;
                      });

            return result;
        }

        std::string PrescriptionRepository::getNextId()
        {
            if (!m_isLoaded)
            {
                load();
            }

            const std::string prefix(Constants::PRESCRIPTION_ID_PREFIX);
            const size_t prefixLen = prefix.length();

            // Find max ID number from existing prescriptions
            int maxId = 0;
            for (const auto &prescription : m_prescriptions)
            {
                const ID &currentId = prescription.getPrescriptionID();

                // Extract and validate numeric part (e.g., "PRE001" -> 1)
                if (currentId.length() > prefixLen && currentId.starts_with(prefix))
                {
                    try
                    {
                        int num = std::stoi(currentId.substr(prefixLen));
                        maxId = std::max(maxId, num);
                    }
                    catch (const std::invalid_argument &)
                    {
                        // Skip IDs with non-numeric suffix
                    }
                    catch (const std::out_of_range &)
                    {
                        // Skip IDs with number too large
                    }
                }
            }

            // Generate next ID using C++23 format (e.g., "PRE001", "PRE002", ...)
            return std::format("{}{:03d}", prefix, maxId + 1);
        }

        // ==================== Dispensing Operations ====================

        bool PrescriptionRepository::markAsDispensed(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            const ID &prescriptionId = id; // Using ID type alias for clarity

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&prescriptionId](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == prescriptionId;
                                   });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            // Modify directly via iterator
            it->setDispensed(true);

            return save();
        }

        bool PrescriptionRepository::markAsUndispensed(const std::string &id)
        {
            if (!m_isLoaded)
            {
                load();
            }

            const ID &prescriptionId = id; // Using ID type alias for clarity

            auto it = std::find_if(m_prescriptions.begin(), m_prescriptions.end(),
                                   [&prescriptionId](const Prescription &p)
                                   {
                                       return p.getPrescriptionID() == prescriptionId;
                                   });

            if (it == m_prescriptions.end())
            {
                return false;
            }

            // Modify directly via iterator
            it->setDispensed(false);

            return save();
        }

        // ==================== File Path Management ====================

        void PrescriptionRepository::setFilePath(const std::string &filePath)
        {
            m_filePath = filePath;
            m_isLoaded = false;
            load();
        }

        std::string PrescriptionRepository::getFilePath() const
        {
            return m_filePath;
        }

    } // namespace DAL
} // namespace HMS