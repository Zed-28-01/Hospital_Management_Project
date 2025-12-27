#include "bll/PrescriptionService.h"
#include "common/Utils.h"

#include <algorithm>

namespace HMS
{
namespace BLL
{

// ==================== Singleton ====================
std::unique_ptr<PrescriptionService> PrescriptionService::s_instance = nullptr;
std::mutex PrescriptionService::s_mutex;

// ==================== Constructor ====================
PrescriptionService::PrescriptionService()
    : m_prescriptionRepo(DAL::PrescriptionRepository::getInstance()),
      m_appointmentRepo(DAL::AppointmentRepository::getInstance()),
      m_medicineRepo(DAL::MedicineRepository::getInstance())
{
}

PrescriptionService* PrescriptionService::getInstance()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (s_instance == nullptr)
    {
        s_instance = std::unique_ptr<PrescriptionService>(new PrescriptionService());
    }
    return s_instance.get();
}

void PrescriptionService::resetInstance()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

PrescriptionService::~PrescriptionService() = default;

// ==================== Creation ====================
bool PrescriptionService::createPrescription(const Model::Prescription& prescription)
{
    if (!validatePrescription(prescription))
        return false;

    if (m_prescriptionRepo->exists(prescription.getPrescriptionID()))
        return false;

    return m_prescriptionRepo->add(prescription);
}

Result<Model::Prescription> PrescriptionService::createPrescription(
    const std::string& appointmentID,
    const std::string& diagnosis,
    const std::string& notes)
{
    auto aptOpt = m_appointmentRepo->getById(appointmentID);
    if (!aptOpt.has_value())
        return std::nullopt;

    if (appointmentHasPrescription(appointmentID))
        return std::nullopt;

    std::string newID = m_prescriptionRepo->getNextId();
    std::string date = Utils::getCurrentDate();

    Model::Prescription prescription(
        newID,
        appointmentID,
        aptOpt->getPatientUsername(),
        aptOpt->getDoctorID(),
        date);

    prescription.setDiagnosis(diagnosis);
    prescription.setNotes(notes);

    if (createPrescription(prescription))
        return prescription;

    return std::nullopt;
}

bool PrescriptionService::updatePrescription(const Model::Prescription& prescription)
{
    if (!validatePrescription(prescription))
        return false;

    if (!m_prescriptionRepo->exists(prescription.getPrescriptionID()))
        return false;

    return m_prescriptionRepo->update(prescription);
}

bool PrescriptionService::deletePrescription(const std::string& prescriptionID)
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
        return false;

    if (opt->isDispensed())
        return false;

    return m_prescriptionRepo->remove(prescriptionID);
}

// ==================== Item Management ====================
bool PrescriptionService::addPrescriptionItem(
    const std::string& prescriptionID,
    const Model::PrescriptionItem& item)
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value() || opt->isDispensed())
        return false;

    Model::Prescription prescription = opt.value();
    prescription.addItem(item);

    return m_prescriptionRepo->update(prescription);
}

bool PrescriptionService::addPrescriptionItem(
    const std::string& prescriptionID,
    const std::string& medicineID,
    int quantity,
    const std::string& dosage,
    const std::string& duration,
    const std::string& instructions)
{
    auto medOpt = m_medicineRepo->getById(medicineID);
    if (!medOpt.has_value())
        return false;

    Model::PrescriptionItem item{
        medicineID,
        medOpt->getName(),
        quantity,
        dosage,
        duration,
        instructions};

    return addPrescriptionItem(prescriptionID, item);
}

bool PrescriptionService::removePrescriptionItem(
    const std::string& prescriptionID,
    const std::string& medicineID)
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value() || opt->isDispensed())
        return false;

    Model::Prescription prescription = opt.value();
    prescription.removeItem(medicineID);

    return m_prescriptionRepo->update(prescription);
}

bool PrescriptionService::updatePrescriptionItem(
    const std::string& prescriptionID,
    const Model::PrescriptionItem& item)
{
    return addPrescriptionItem(prescriptionID, item);
}

bool PrescriptionService::clearPrescriptionItems(const std::string& prescriptionID)
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value() || opt->isDispensed())
        return false;

    Model::Prescription prescription = opt.value();
    prescription.clearItems();

    return m_prescriptionRepo->update(prescription);
}

// ==================== Queries ====================
Result<Model::Prescription> PrescriptionService::getPrescriptionByID(
    const std::string& prescriptionID)
{
    return m_prescriptionRepo->getById(prescriptionID);
}

Result<Model::Prescription> PrescriptionService::getPrescriptionByAppointment(
    const std::string& appointmentID)
{
    return m_prescriptionRepo->getByAppointment(appointmentID);
}

List<Model::Prescription> PrescriptionService::getAllPrescriptions()
{
    return m_prescriptionRepo->getAll();
}

List<Model::Prescription> PrescriptionService::getPatientPrescriptions(
    const std::string& patientUsername)
{
    return m_prescriptionRepo->getByPatient(patientUsername);
}

List<Model::Prescription> PrescriptionService::getDoctorPrescriptions(
    const std::string& doctorID)
{
    return m_prescriptionRepo->getByDoctor(doctorID);
}

List<Model::Prescription> PrescriptionService::getPrescriptionsByDateRange(
    const std::string& startDate,
    const std::string& endDate)
{
    return m_prescriptionRepo->getByDateRange(startDate, endDate);
}

size_t PrescriptionService::getPrescriptionCount() const
{
    return m_prescriptionRepo->count();
}

// ==================== Dispensing ====================
DispenseResult PrescriptionService::dispensePrescription(
    const std::string& prescriptionID)
{
    DispenseResult result{false, "", 0.0, {}};

    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
    {
        result.message = "Prescription not found";
        return result;
    }

    Model::Prescription prescription = opt.value();
    if (prescription.isDispensed())
    {
        result.message = "Prescription already dispensed";
        return result;
    }

    for (const auto& item : prescription.getItems())
    {
        auto medOpt = m_medicineRepo->getById(item.medicineID);
        if (!medOpt.has_value() ||
            medOpt->getQuantityInStock() < item.quantity)
        {
            result.failedItems.push_back(item.medicineID);
        }
    }

    if (!result.failedItems.empty())
    {
        result.message = "Insufficient stock";
        return result;
    }

    for (const auto& item : prescription.getItems())
    {
        auto medOpt = m_medicineRepo->getById(item.medicineID);
        Model::Medicine med = medOpt.value();
        med.removeStock(item.quantity);
        m_medicineRepo->update(med);

        result.totalCost += med.getUnitPrice() * item.quantity;
    }

    m_prescriptionRepo->markAsDispensed(prescriptionID);

    result.success = true;
    result.message = "Dispensed successfully";
    return result;
}

bool PrescriptionService::canDispense(const std::string& prescriptionID) const
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
        return false;

    for (const auto& item : opt->getItems())
    {
        auto medOpt = m_medicineRepo->getById(item.medicineID);
        if (!medOpt.has_value() ||
            medOpt->getQuantityInStock() < item.quantity)
            return false;
    }

    return true;
}

List<std::string> PrescriptionService::getInsufficientStockItems(
    const std::string& prescriptionID) const
{
    List<std::string> result;
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
        return result;

    for (const auto& item : opt->getItems())
    {
        auto medOpt = m_medicineRepo->getById(item.medicineID);
        if (!medOpt.has_value() ||
            medOpt->getQuantityInStock() < item.quantity)
        {
            result.push_back(item.medicineID);
        }
    }
    return result;
}

List<Model::Prescription> PrescriptionService::getUndispensedPrescriptions()
{
    return m_prescriptionRepo->getUndispensed();
}

List<Model::Prescription> PrescriptionService::getDispensedPrescriptions()
{
    return m_prescriptionRepo->getDispensed();
}

bool PrescriptionService::markAsUndispensed(const std::string& prescriptionID)
{
    return m_prescriptionRepo->markAsUndispensed(prescriptionID);
}

// ==================== Cost ====================
double PrescriptionService::calculatePrescriptionCost(
    const std::string& prescriptionID) const
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
        return -1.0;

    return calculateItemsCost(opt->getItems());
}

double PrescriptionService::calculateItemsCost(
    const List<Model::PrescriptionItem>& items) const
{
    double total = 0.0;
    for (const auto& item : items)
    {
        auto medOpt = m_medicineRepo->getById(item.medicineID);
        if (medOpt.has_value())
            total += medOpt->getUnitPrice() * item.quantity;
    }
    return total;
}

// ==================== Print ====================
std::string PrescriptionService::getPrescriptionPrintFormat(
    const std::string& prescriptionID) const
{
    auto opt = m_prescriptionRepo->getById(prescriptionID);
    if (!opt.has_value())
        return "";

    return opt->toPrintFormat();
}

// ==================== Statistics ====================
std::map<std::string, int> PrescriptionService::getPrescriptionStatistics() const
{
    std::map<std::string, int> stats;
    stats["total"] = m_prescriptionRepo->count();
    stats["dispensed"] = m_prescriptionRepo->getDispensed().size();
    stats["undispensed"] = m_prescriptionRepo->getUndispensed().size();
    return stats;
}

List<std::pair<std::string, int>>
PrescriptionService::getMostPrescribedMedicines(int limit) const
{
    std::map<std::string, int> counter;

    for (const auto& p : m_prescriptionRepo->getAll())
        for (const auto& i : p.getItems())
            counter[i.medicineID]++;

    List<std::pair<std::string, int>> result(counter.begin(), counter.end());

    std::sort(result.begin(), result.end(),
              [](auto& a, auto& b) { return a.second > b.second; });

    if (result.size() > static_cast<size_t>(limit))
        result.resize(limit);

    return result;
}

// ==================== Validation ====================
bool PrescriptionService::validatePrescription(
    const Model::Prescription& prescription) const
{
    return !prescription.getPrescriptionID().empty() &&
           !prescription.getAppointmentID().empty();
}

bool PrescriptionService::prescriptionExists(const std::string& prescriptionID) const
{
    return m_prescriptionRepo->exists(prescriptionID);
}

bool PrescriptionService::appointmentHasPrescription(
    const std::string& appointmentID) const
{
    return m_prescriptionRepo->getByAppointment(appointmentID).has_value();
}

// ==================== Persistence ====================
bool PrescriptionService::saveData()
{
    return m_prescriptionRepo->save();
}

bool PrescriptionService::loadData()
{
    return m_prescriptionRepo->load();
}

} // namespace BLL
} // namespace HMS
