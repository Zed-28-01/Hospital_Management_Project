#include "bll/PrescriptionService.h"
#include "common/Constants.h"
#include "common/Utils.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace HMS
{
  namespace BLL
  {

    // ==================== Static Members Initialization ====================
    std::unique_ptr<PrescriptionService> PrescriptionService::s_instance = nullptr;
    std::mutex PrescriptionService::s_mutex;

    // ==================== Private Constructor ====================
    PrescriptionService::PrescriptionService()
    {
      m_prescriptionRepo = DAL::PrescriptionRepository::getInstance();
      m_appointmentRepo = DAL::AppointmentRepository::getInstance();
      m_medicineRepo = DAL::MedicineRepository::getInstance();
    }

    // ==================== Destructor ====================
    PrescriptionService::~PrescriptionService() = default;

    // ==================== Singleton Access ====================
    PrescriptionService *PrescriptionService::getInstance()
    {
      std::lock_guard<std::mutex> lock(s_mutex);
      if (!s_instance)
      {
        s_instance =
            std::unique_ptr<PrescriptionService>(new PrescriptionService());
      }
      return s_instance.get();
    }

    void PrescriptionService::resetInstance()
    {
      std::lock_guard<std::mutex> lock(s_mutex);
      s_instance.reset();
    }

    // ==================== Prescription Creation ====================

    std::string PrescriptionService::generateNextID()
    {
      return m_prescriptionRepo->getNextId();
    }

    bool PrescriptionService::createPrescription(
        const Model::Prescription &prescription)
    {
      if (!validatePrescription(prescription))
      {
        return false;
      }

      if (prescriptionExists(prescription.getPrescriptionID()))
      {
        return false;
      }

      // Check if appointment already has a prescription
      const std::string &appointmentID = prescription.getAppointmentID();
      if (!appointmentID.empty() && appointmentHasPrescription(appointmentID))
      {
        return false;
      }

      return m_prescriptionRepo->add(prescription);
    }

    Result<Model::Prescription>
    PrescriptionService::createPrescription(const std::string &appointmentID,
                                            const std::string &diagnosis,
                                            const std::string &notes)
    {
      // Validate appointment exists
      auto appointment = m_appointmentRepo->getById(appointmentID);
      if (!appointment)
      {
        return std::nullopt;
      }

      // Check if appointment already has a prescription
      if (appointmentHasPrescription(appointmentID))
      {
        return std::nullopt;
      }

      // Generate new ID
      std::string newID = m_prescriptionRepo->getNextId();
      std::string currentDate = Utils::getCurrentDate();

      // Create prescription with appointment details
      Model::Prescription prescription(newID, appointmentID,
                                       appointment->getPatientUsername(),
                                       appointment->getDoctorID(), currentDate);

      prescription.setDiagnosis(diagnosis);
      prescription.setNotes(notes);

      if (m_prescriptionRepo->add(prescription))
      {
        return prescription;
      }

      return std::nullopt;
    }

    bool PrescriptionService::updatePrescription(
        const Model::Prescription &prescription)
    {
      if (!prescriptionExists(prescription.getPrescriptionID()))
      {
        return false;
      }

      // Don't allow updating dispensed prescriptions
      auto existing = m_prescriptionRepo->getById(prescription.getPrescriptionID());
      if (existing && existing->isDispensed())
      {
        return false;
      }

      if (!validatePrescription(prescription))
      {
        return false;
      }

      return m_prescriptionRepo->update(prescription);
    }

    bool PrescriptionService::deletePrescription(
        const std::string &prescriptionID)
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return false;
      }

      // Don't allow deleting dispensed prescriptions
      if (prescription->isDispensed())
      {
        return false;
      }

      return m_prescriptionRepo->remove(prescriptionID);
    }

    // ==================== Item Management ====================

    bool PrescriptionService::addPrescriptionItem(
        const std::string &prescriptionID, const Model::PrescriptionItem &item)
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return false;
      }

      // Don't allow modifying dispensed prescriptions
      if (prescription->isDispensed())
      {
        return false;
      }

      // Validate medicine exists
      if (!m_medicineRepo->exists(item.medicineID))
      {
        return false;
      }

      prescription->addItem(item);
      return m_prescriptionRepo->update(*prescription);
    }

    bool PrescriptionService::addPrescriptionItem(const std::string &prescriptionID,
                                                  const std::string &medicineID,
                                                  int quantity,
                                                  const std::string &dosage,
                                                  const std::string &duration,
                                                  const std::string &instructions)
    {
      // Validate input
      if (quantity <= 0)
      {
        return false;
      }

      // Get medicine to populate name
      auto medicine = m_medicineRepo->getById(medicineID);
      if (!medicine)
      {
        return false;
      }

      Model::PrescriptionItem item;
      item.medicineID = medicineID;
      item.medicineName = medicine->getName();
      item.quantity = quantity;
      item.dosage = dosage;
      item.duration = duration;
      item.instructions = instructions;

      return addPrescriptionItem(prescriptionID, item);
    }

    bool PrescriptionService::removePrescriptionItem(
        const std::string &prescriptionID, const std::string &medicineID)
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return false;
      }

      // Don't allow modifying dispensed prescriptions
      if (prescription->isDispensed())
      {
        return false;
      }

      prescription->removeItem(medicineID);
      return m_prescriptionRepo->update(*prescription);
    }

    bool PrescriptionService::updatePrescriptionItem(
        const std::string &prescriptionID, const Model::PrescriptionItem &item)
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return false;
      }

      // Don't allow modifying dispensed prescriptions
      if (prescription->isDispensed())
      {
        return false;
      }

      // Remove existing and add updated (addItem handles duplicates)
      prescription->removeItem(item.medicineID);
      prescription->addItem(item);
      return m_prescriptionRepo->update(*prescription);
    }

    bool PrescriptionService::clearPrescriptionItems(
        const std::string &prescriptionID)
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return false;
      }

      // Don't allow modifying dispensed prescriptions
      if (prescription->isDispensed())
      {
        return false;
      }

      prescription->clearItems();
      return m_prescriptionRepo->update(*prescription);
    }

    // ==================== Query Operations ====================

    Result<Model::Prescription>
    PrescriptionService::getPrescriptionByID(const std::string &prescriptionID)
    {
      return m_prescriptionRepo->getById(prescriptionID);
    }

    Result<Model::Prescription> PrescriptionService::getPrescriptionByAppointment(
        const std::string &appointmentID)
    {
      return m_prescriptionRepo->getByAppointment(appointmentID);
    }

    List<Model::Prescription> PrescriptionService::getAllPrescriptions()
    {
      return m_prescriptionRepo->getAll();
    }

    List<Model::Prescription> PrescriptionService::getPatientPrescriptions(
        const std::string &patientUsername)
    {
      return m_prescriptionRepo->getByPatient(patientUsername);
    }

    List<Model::Prescription>
    PrescriptionService::getDoctorPrescriptions(const std::string &doctorID)
    {
      return m_prescriptionRepo->getByDoctor(doctorID);
    }

    List<Model::Prescription>
    PrescriptionService::getPrescriptionsByDateRange(const std::string &startDate,
                                                     const std::string &endDate)
    {
      // Validate dates
      if (!Utils::isValidDateInternal(startDate) || !Utils::isValidDateInternal(endDate))
      {
        return {};
      }

      return m_prescriptionRepo->getByDateRange(startDate, endDate);
    }

    List<PrescriptionSummary> PrescriptionService::getPrescriptionSummaries()
    {
      List<PrescriptionSummary> summaries;
      auto prescriptions = m_prescriptionRepo->getAll();

      for (const auto &presc : prescriptions)
      {
        PrescriptionSummary summary;
        summary.prescriptionID = presc.getPrescriptionID();
        summary.date = presc.getPrescriptionDate();
        summary.diagnosis = presc.getDiagnosis();
        summary.itemCount = static_cast<int>(presc.getItemCount());
        summary.isDispensed = presc.isDispensed();
        summary.totalCost = calculateItemsCost(presc.getItems());

        // Note: Patient and doctor names would require additional lookups
        // For now, we use IDs/usernames
        summary.patientName = presc.getPatientUsername();
        summary.doctorName = presc.getDoctorID();

        summaries.push_back(summary);
      }

      return summaries;
    }

    size_t PrescriptionService::getPrescriptionCount() const
    {
      return m_prescriptionRepo->count();
    }

    // ==================== Dispensing ====================

    DispenseResult
    PrescriptionService::dispensePrescription(const std::string &prescriptionID)
    {
      DispenseResult result;
      result.success = false;
      result.totalCost = 0.0;

      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        result.message = "Prescription not found";
        return result;
      }

      if (prescription->isDispensed())
      {
        result.message = "Prescription already dispensed";
        return result;
      }

      const auto &items = prescription->getItems();
      if (items.empty())
      {
        result.message = "Prescription has no items";
        return result;
      }

      // Check stock availability for all items first
      for (const auto &item : items)
      {
        auto medicine = m_medicineRepo->getById(item.medicineID);
        if (!medicine)
        {
          result.failedItems.push_back(item.medicineID);
          continue;
        }

        if (medicine->getQuantityInStock() < item.quantity)
        {
          result.failedItems.push_back(item.medicineID);
        }
      }

      if (!result.failedItems.empty())
      {
        result.message = "Insufficient stock for some items";
        return result;
      }

      // Deduct stock for all items and calculate total cost
      for (const auto &item : items)
      {
        auto medicine = m_medicineRepo->getById(item.medicineID);
        if (medicine)
        {
          // Deduct stock
          int newStock = medicine->getQuantityInStock() - item.quantity;
          m_medicineRepo->updateStock(item.medicineID, newStock);

          // Calculate cost
          result.totalCost += medicine->getUnitPrice() * item.quantity;
        }
      }

      // Mark as dispensed
      m_prescriptionRepo->markAsDispensed(prescriptionID);

      result.success = true;
      result.message = "Prescription dispensed successfully";
      return result;
    }

    bool PrescriptionService::canDispense(const std::string &prescriptionID) const
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription || prescription->isDispensed())
      {
        return false;
      }

      const auto &items = prescription->getItems();
      if (items.empty())
      {
        return false;
      }

      // Check stock availability for all items
      for (const auto &item : items)
      {
        auto medicine = m_medicineRepo->getById(item.medicineID);
        if (!medicine || medicine->getQuantityInStock() < item.quantity)
        {
          return false;
        }
      }

      return true;
    }

    List<std::string> PrescriptionService::getInsufficientStockItems(
        const std::string &prescriptionID) const
    {
      List<std::string> insufficientItems;

      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return insufficientItems;
      }

      const auto &items = prescription->getItems();
      for (const auto &item : items)
      {
        auto medicine = m_medicineRepo->getById(item.medicineID);
        if (!medicine || medicine->getQuantityInStock() < item.quantity)
        {
          insufficientItems.push_back(item.medicineID);
        }
      }

      return insufficientItems;
    }

    List<Model::Prescription> PrescriptionService::getUndispensedPrescriptions()
    {
      return m_prescriptionRepo->getUndispensed();
    }

    List<Model::Prescription> PrescriptionService::getDispensedPrescriptions()
    {
      return m_prescriptionRepo->getDispensed();
    }

    bool PrescriptionService::markAsUndispensed(const std::string &prescriptionID)
    {
      return m_prescriptionRepo->markAsUndispensed(prescriptionID);
    }

    // ==================== Cost Calculation ====================

    double PrescriptionService::calculatePrescriptionCost(
        const std::string &prescriptionID) const
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return -1.0;
      }

      return calculateItemsCost(prescription->getItems());
    }

    double PrescriptionService::calculateItemsCost(
        const List<Model::PrescriptionItem> &items) const
    {
      double total = 0.0;

      for (const auto &item : items)
      {
        auto medicine = m_medicineRepo->getById(item.medicineID);
        if (medicine)
        {
          total += medicine->getUnitPrice() * item.quantity;
        }
      }

      return total;
    }

    // ==================== Print/Export ====================

    std::string PrescriptionService::getPrescriptionPrintFormat(
        const std::string &prescriptionID) const
    {
      auto prescription = m_prescriptionRepo->getById(prescriptionID);
      if (!prescription)
      {
        return "Prescription not found";
      }

      // Use the model's built-in print format
      return prescription->toPrintFormat();
    }

    // ==================== Statistics ====================

    std::map<std::string, int>
    PrescriptionService::getPrescriptionStatistics() const
    {
      std::map<std::string, int> stats;
      auto prescriptions = m_prescriptionRepo->getAll();

      stats["total"] = static_cast<int>(prescriptions.size());
      stats["dispensed"] = 0;
      stats["undispensed"] = 0;
      stats["totalItems"] = 0;

      for (const auto &presc : prescriptions)
      {
        if (presc.isDispensed())
        {
          stats["dispensed"]++;
        }
        else
        {
          stats["undispensed"]++;
        }
        stats["totalItems"] += static_cast<int>(presc.getItemCount());
      }

      return stats;
    }

    List<std::pair<std::string, int>>
    PrescriptionService::getMostPrescribedMedicines(int limit) const
    {
      std::map<std::string, int> medicineCounts;
      auto prescriptions = m_prescriptionRepo->getAll();

      // Count occurrences of each medicine
      for (const auto &presc : prescriptions)
      {
        const auto &items = presc.getItems();
        for (const auto &item : items)
        {
          medicineCounts[item.medicineID] += item.quantity;
        }
      }

      // Convert to vector and sort by count
      List<std::pair<std::string, int>> result(medicineCounts.begin(),
                                               medicineCounts.end());
      std::ranges::sort(
          result, [](const auto &a, const auto &b)
          { return a.second > b.second; });

      // Limit results
      if (limit > 0 && static_cast<int>(result.size()) > limit)
      {
        result.resize(limit);
      }

      return result;
    }

    // ==================== Validation ====================

    bool PrescriptionService::validatePrescription(
        const Model::Prescription &prescription) const
    {
      // Check required fields
      if (prescription.getPrescriptionID().empty() ||
          prescription.getPatientUsername().empty() ||
          prescription.getDoctorID().empty())
      {
        return false;
      }

      // Validate date format if provided
      const std::string &date = prescription.getPrescriptionDate();
      if (!date.empty() && !Utils::isValidDateInternal(date))
      {
        return false;
      }

      // Validate each item
      const auto &items = prescription.getItems();
      for (const auto &item : items)
      {
        if (item.medicineID.empty())
        {
          return false;
        }
        if (item.quantity <= 0)
        {
          return false;
        }
      }

      return true;
    }

    bool PrescriptionService::prescriptionExists(
        const std::string &prescriptionID) const
    {
      return m_prescriptionRepo->exists(prescriptionID);
    }

    bool PrescriptionService::appointmentHasPrescription(
        const std::string &appointmentID) const
    {
      auto prescription = m_prescriptionRepo->getByAppointment(appointmentID);
      return prescription.has_value();
    }

    // ==================== Data Persistence ====================

    bool PrescriptionService::saveData() { return m_prescriptionRepo->save(); }

    bool PrescriptionService::loadData() { return m_prescriptionRepo->load(); }

  } // namespace BLL
} // namespace HMS
