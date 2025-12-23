# Advanced Features Implementation - Hospital Management System

## Phase Overview
Implement the advanced features (Department, Medicine, Prescription, ReportGenerator) following the existing 3-tier architecture pattern.

---

## Task Tracking

### 1. Structural Analysis ✅
- [x] Read ARCHITECTURE_VI.md for architecture patterns
- [x] Read architecture diagrams (overview and detailed)
- [x] Scan include/advance/ for existing placeholder headers
- [x] Scan src/ for current implementation state
- [x] Review CMakeLists.txt for build configuration
- [x] Review test structure for testing patterns

### 2. Planning & Documentation ✅
- [x] Write comprehensive implementation_plan.md
- [x] Document structural recommendations
- [x] Define new directories and files needed
- [x] Outline integration approach
- [x] Request user review and approval

---

## Implementation Tasks

### Phase 1: Foundation - Model Layer
> **Estimated: 3 model implementations**

#### 1.1 Medicine Model (Independent - Start Here)
- [x] Create `src/model/Medicine.cpp`
  - [x] Implement constructors (default + parameterized)
  - [x] Implement all getters/setters
  - [x] Implement `addStock(int quantity)`
  - [x] Implement `removeStock(int quantity)` - returns false if insufficient
  - [x] Implement `isLowStock()` - compares with reorderLevel
  - [x] Implement `isExpired()` - compares with current date
  - [x] Implement `isExpiringSoon(int days)` - threshold check
  - [x] Implement `serialize()` - pipe-delimited format
  - [x] Implement `deserialize(const std::string&)` - parse and create
  - [x] Implement `displayInfo()` - console output
- [x] Verify build: `cmake --build build --target HospitalLib`

#### 1.2 Department Model (Independent)
- [x] Create `src/model/Department.cpp`
  - [x] Implement constructors
  - [x] Implement all getters/setters
  - [x] Implement `addDoctor(const std::string& doctorID)`
  - [x] Implement `removeDoctor(const std::string& doctorID)`
  - [x] Implement `hasDoctor(const std::string& doctorID)` - checks vector
  - [x] Implement `getDoctorCount()` - returns vector size
  - [x] Implement `serialize()` - handle doctorIDs as comma-separated
  - [x] Implement `deserialize(const std::string&)`
  - [x] Implement `displayInfo()`
- [x] Verify build

#### 1.3 Prescription Model (Depends on Medicine)
- [x] Create `src/model/Prescription.cpp`
  - [x] Implement PrescriptionItem serialization helper
  - [x] Implement constructors
  - [x] Implement all getters/setters
  - [x] Implement `addItem(const PrescriptionItem&)`
  - [x] Implement `removeItem(const std::string& medicineID)`
  - [x] Implement `clearItems()`
  - [x] Implement `getItemCount()`
  - [x] Implement `calculateTotalCost()` - sum medicine costs
  - [x] Implement `serialize()` - complex items format
  - [x] Implement `deserialize(const std::string&)` - parse items
  - [x] Implement `displayInfo()`
  - [x] Implement `toPrintFormat()` - patient-friendly output
- [x] Verify build

---

### Phase 2: Data Access Layer (DAL)

#### 2.1 MedicineRepository
- [ ] Create `include/dal/MedicineRepository.h`
  - [ ] Declare class implementing `IRepository<Medicine>`
  - [ ] Add singleton pattern (static instance, mutex, getInstance, resetInstance)
  - [ ] Declare specialized queries: `getByCategory()`, `getLowStock()`, `getExpiringSoon()`, `searchByName()`
  - [ ] Declare `getNextId()` for ID generation
- [ ] Create `src/dal/MedicineRepository.cpp`
  - [ ] Implement singleton pattern
  - [ ] Implement all IRepository methods (getAll, getById, add, update, remove, save, load, count, exists, clear)
  - [ ] Implement specialized queries
  - [ ] Implement file I/O using FileHelper
- [ ] Verify build

#### 2.2 DepartmentRepository
- [ ] Create `include/dal/DepartmentRepository.h`
  - [ ] Declare class implementing `IRepository<Department>`
  - [ ] Add singleton pattern
  - [ ] Declare specialized queries: `getByName()`, `getByHeadDoctor()`, `getDepartmentByDoctor()`
- [ ] Create `src/dal/DepartmentRepository.cpp`
  - [ ] Implement all methods
  - [ ] Handle doctorIDs vector in serialization
- [ ] Verify build

#### 2.3 PrescriptionRepository
- [ ] Create `include/dal/PrescriptionRepository.h`
  - [ ] Declare class implementing `IRepository<Prescription>`
  - [ ] Add singleton pattern
  - [ ] Declare specialized queries: `getByPatient()`, `getByDoctor()`, `getByAppointment()`, `getUndispensed()`
- [ ] Create `src/dal/PrescriptionRepository.cpp`
  - [ ] Implement all methods
  - [ ] Handle complex items serialization
- [ ] Verify build

---

### Phase 3: Business Logic Layer (BLL)

#### 3.1 MedicineService
- [ ] Create `include/bll/MedicineService.h`
  - [ ] Declare singleton service
  - [ ] Declare CRUD methods with validation
  - [ ] Declare stock management: `addStock()`, `removeStock()`, `transferStock()`
  - [ ] Declare alerts: `getLowStockAlerts()`, `getExpiryAlerts()`
  - [ ] Declare search methods
- [ ] Create `src/bll/MedicineService.cpp`
  - [ ] Implement with MedicineRepository dependency
  - [ ] Add validation logic (price > 0, quantity >= 0, valid dates)
  - [ ] Implement alert logic
- [ ] Verify build

#### 3.2 DepartmentService
- [ ] Create `include/bll/DepartmentService.h`
  - [ ] Declare singleton service
  - [ ] Declare CRUD methods
  - [ ] Declare doctor assignment: `assignDoctor()`, `unassignDoctor()`, `setDepartmentHead()`
  - [ ] Declare statistics: `getDepartmentStats()`, `getDoctorDepartment()`
- [ ] Create `src/bll/DepartmentService.cpp`
  - [ ] Implement with DepartmentRepository + DoctorRepository dependencies
  - [ ] Add validation (doctor exists before assignment, head must be in department)
- [ ] Verify build

#### 3.3 PrescriptionService
- [ ] Create `include/bll/PrescriptionService.h`
  - [ ] Declare singleton service
  - [ ] Declare creation: `createPrescription(appointmentID, diagnosis, notes)`
  - [ ] Declare item management: `addPrescriptionItem()`, `removePrescriptionItem()`
  - [ ] Declare dispensing: `dispensePrescription()` - updates medicine stock
  - [ ] Declare queries: `getPatientPrescriptions()`, `getDoctorPrescriptions()`
- [ ] Create `src/bll/PrescriptionService.cpp`
  - [ ] Implement with PrescriptionRepository + AppointmentRepository + MedicineRepository dependencies
  - [ ] Validate appointment exists before creating prescription
  - [ ] Validate medicine exists and has sufficient stock before dispensing
  - [ ] Update medicine inventory when dispensed
- [ ] Verify build

#### 3.4 ReportGenerator
- [ ] Create `src/bll/ReportGenerator.cpp` (header already exists)
  - [ ] Implement singleton pattern with static instance initialization
  - [ ] Implement `generateDailyReport(date)` - appointments, revenue for date
  - [ ] Implement `generateWeeklyReport(startDate)` - week summary
  - [ ] Implement `generateMonthlyReport(month, year)` - month summary
  - [ ] Implement `generateRevenueReport(startDate, endDate)` - financial
  - [ ] Implement `generatePatientReport()` - patient statistics
  - [ ] Implement `generateDoctorPerformanceReport()` - doctor metrics
  - [ ] Implement `generateAppointmentAnalysis()` - status breakdown
  - [ ] Implement `exportToText()`, `exportToCSV()`, `exportToHTML()`
  - [ ] Implement `exportReport()` - delegates to format-specific
  - [ ] Implement `displayReport()` - console output
- [ ] Verify build

---

### Phase 4: Common Layer Updates

#### 4.1 Constants.h Updates
- [ ] Add file path constants:
  ```cpp
  constexpr const char* DEPARTMENT_FILE = "data/Department.txt";
  constexpr const char* MEDICINE_FILE = "data/Medicine.txt";
  constexpr const char* PRESCRIPTION_FILE = "data/Prescription.txt";
  constexpr const char* REPORTS_DIR = "data/reports/";
  ```
- [ ] Add ID prefixes:
  ```cpp
  constexpr const char* DEPARTMENT_ID_PREFIX = "DEP";
  constexpr const char* MEDICINE_ID_PREFIX = "MED";
  constexpr const char* PRESCRIPTION_ID_PREFIX = "PRE";
  ```
- [ ] Add medicine constants:
  ```cpp
  constexpr int DEFAULT_REORDER_LEVEL = 10;
  constexpr int EXPIRY_WARNING_DAYS = 30;
  ```

#### 4.2 Types.h Updates
- [ ] Add type aliases:
  ```cpp
  using DepartmentID = std::string;
  using MedicineID = std::string;
  using PrescriptionID = std::string;
  ```

---

### Phase 5: Data Files Creation

#### 5.1 Create Data Files
- [ ] Create `data/Department.txt` with header comment
- [ ] Create `data/Medicine.txt` with header comment
- [ ] Create `data/Prescription.txt` with header comment
- [ ] Create `data/reports/` directory

#### 5.2 Create Sample Data
- [ ] Add sample departments to `data/sample/Department_sample.txt`
- [ ] Add sample medicines to `data/sample/Medicine_sample.txt`
- [ ] Add sample prescriptions to `data/sample/Prescription_sample.txt`

---

### Phase 6: Testing

#### 6.1 Model Tests
- [ ] Create `test/model/DepartmentTest.cpp`
- [ ] Create `test/model/MedicineTest.cpp`
- [ ] Create `test/model/PrescriptionTest.cpp`

#### 6.2 DAL Tests
- [ ] Create `test/dal/DepartmentRepositoryTest.cpp`
- [ ] Create `test/dal/MedicineRepositoryTest.cpp`
- [ ] Create `test/dal/PrescriptionRepositoryTest.cpp`

#### 6.3 BLL Tests
- [ ] Create `test/bll/DepartmentServiceTest.cpp`
- [ ] Create `test/bll/MedicineServiceTest.cpp`
- [ ] Create `test/bll/PrescriptionServiceTest.cpp`
- [ ] Create `test/bll/ReportGeneratorTest.cpp`

#### 6.4 Test Fixtures
- [ ] Create `test/fixtures/Department_test.txt`
- [ ] Create `test/fixtures/Medicine_test.txt`
- [ ] Create `test/fixtures/Prescription_test.txt`

#### 6.5 Build & Run Tests
- [ ] Build tests: `cmake --build build --target HospitalTests`
- [ ] Run all tests: `./build/HospitalTests`
- [ ] Run specific tests: `./build/HospitalTests --gtest_filter="*Department*:*Medicine*:*Prescription*:*Report*"`

---

### Phase 7: Documentation Updates

#### 7.1 Architecture Documentation
- [ ] Update `docs/ARCHITECTURE.md` with new components
- [ ] Update `docs/ARCHITECTURE_VI.md` with Vietnamese version
- [ ] Update `docs/diagrams/architecture-overview.md` with new services
- [ ] Update `docs/diagrams/architecture-detailed.md` with new class diagrams

#### 7.2 Build Documentation
- [ ] Update `docs/BUILD.md` if needed
- [ ] Update `docs/BUILD_vi.md` if needed

---

## Current Status

**Phase:** Ready for Implementation
**Next Step:** Start with Phase 1.1 - Medicine Model Implementation

---

## Dependencies Graph

```
Medicine (Independent)
    ↓
MedicineRepository
    ↓
MedicineService
    ↓
PrescriptionService ←── Prescription ←── PrescriptionRepository
    ↓
ReportGenerator

Department (Independent)
    ↓
DepartmentRepository
    ↓
DepartmentService
    ↓
ReportGenerator
```

---

## Notes

1. **CMakeLists.txt**: Uses `file(GLOB ...)` - new `.cpp` files are automatically included. No CMake changes needed.

2. **Build Verification**: After each implementation, run `cmake --build build --target HospitalLib` to catch errors early.

3. **Test Pattern**: Follow existing test patterns in `test/bll/AdminServiceTest.cpp`:
   - Use `SetUp()` to reset singletons and redirect to test files
   - Use `TearDown()` to cleanup
   - Create helper methods for test data

4. **Singleton Pattern**: All services and repositories use the same pattern:
   ```cpp
   static std::unique_ptr<ClassName> s_instance;
   static std::mutex s_mutex;
   static ClassName* getInstance();
   static void resetInstance();
   ```

5. **Error Handling**: Use `std::optional<T>` (aliased as `Result<T>`) for nullable returns.
