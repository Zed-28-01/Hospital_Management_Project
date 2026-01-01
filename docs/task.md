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
- [x] Create `include/dal/MedicineRepository.h`
  - [x] Declare class implementing `IRepository<Medicine>`
  - [x] Add singleton pattern (static instance, mutex, getInstance, resetInstance)
  - [x] Declare specialized queries: `getByCategory()`, `getLowStock()`, `getExpiringSoon()`, `searchByName()`
  - [x] Declare `getNextId()` for ID generation
- [x] Create `src/dal/MedicineRepository.cpp`
  - [x] Implement singleton pattern
  - [x] Implement all IRepository methods (getAll, getById, add, update, remove, save, load, count, exists, clear)
  - [x] Implement specialized queries
  - [x] Implement file I/O using FileHelper
- [x] Verify build

#### 2.2 DepartmentRepository
- [x] Create `include/dal/DepartmentRepository.h`
  - [x] Declare class implementing `IRepository<Department>`
  - [x] Add singleton pattern
  - [x] Declare specialized queries: `getByName()`, `getByHeadDoctor()`, `getDepartmentByDoctor()`
- [x] Create `src/dal/DepartmentRepository.cpp`
  - [x] Implement all methods
  - [x] Handle doctorIDs vector in serialization
- [x] Verify build

#### 2.3 PrescriptionRepository
- [x] Create `include/dal/PrescriptionRepository.h`
  - [x] Declare class implementing `IRepository<Prescription>`
  - [x] Add singleton pattern
  - [x] Declare specialized queries: `getByPatient()`, `getByDoctor()`, `getByAppointment()`, `getUndispensed()`
- [x] Create `src/dal/PrescriptionRepository.cpp`
  - [x] Implement all methods
  - [x] Handle complex items serialization
- [x] Verify build

---

### Phase 3: Business Logic Layer (BLL) ✅

#### 3.1 MedicineService
- [x] Create `include/bll/MedicineService.h`
  - [x] Declare singleton service
  - [x] Declare CRUD methods with validation
  - [x] Declare stock management: `addStock()`, `removeStock()`, `transferStock()`
  - [x] Declare alerts: `getLowStockAlerts()`, `getExpiryAlerts()`
  - [x] Declare search methods
- [x] Create `src/bll/MedicineService.cpp`
  - [x] Implement with MedicineRepository dependency
  - [x] Add validation logic (price > 0, quantity >= 0, valid dates)
  - [x] Implement alert logic
- [x] Verify build

#### 3.2 DepartmentService
- [x] Create `include/bll/DepartmentService.h`
  - [x] Declare singleton service
  - [x] Declare CRUD methods
  - [x] Declare doctor assignment: `assignDoctor()`, `unassignDoctor()`, `setDepartmentHead()`
  - [x] Declare statistics: `getDepartmentStats()`, `getDoctorDepartment()`
- [x] Create `src/bll/DepartmentService.cpp`
  - [x] Implement with DepartmentRepository + DoctorRepository dependencies
  - [x] Add validation (doctor exists before assignment, head must be in department)
- [x] Verify build

#### 3.3 PrescriptionService
- [x] Create `include/bll/PrescriptionService.h`
  - [x] Declare singleton service
  - [x] Declare creation: `createPrescription(appointmentID, diagnosis, notes)`
  - [x] Declare item management: `addPrescriptionItem()`, `removePrescriptionItem()`
  - [x] Declare dispensing: `dispensePrescription()` - updates medicine stock
  - [x] Declare queries: `getPatientPrescriptions()`, `getDoctorPrescriptions()`
- [x] Create `src/bll/PrescriptionService.cpp`
  - [x] Implement with PrescriptionRepository + AppointmentRepository + MedicineRepository dependencies
  - [x] Validate appointment exists before creating prescription
  - [x] Validate medicine exists and has sufficient stock before dispensing
  - [x] Update medicine inventory when dispensed
- [x] Verify build

#### 3.4 ReportGenerator
- [x] Create `src/bll/ReportGenerator.cpp` (header already exists)
  - [x] Implement singleton pattern with static instance initialization
  - [x] Implement `generateDailyReport(date)` - appointments, revenue for date
  - [x] Implement `generateWeeklyReport(startDate)` - week summary
  - [x] Implement `generateMonthlyReport(month, year)` - month summary
  - [x] Implement `generateRevenueReport(startDate, endDate)` - financial
  - [x] Implement `generatePatientReport()` - patient statistics
  - [x] Implement `generateDoctorPerformanceReport()` - doctor metrics
  - [x] Implement `generateAppointmentAnalysis()` - status breakdown
  - [x] Implement `exportToText()`, `exportToCSV()`, `exportToHTML()`
  - [x] Implement `exportReport()` - delegates to format-specific
  - [x] Implement `displayReport()` - console output
- [x] Verify build

---

### Phase 4: Common Layer Updates ✅

#### 4.1 Constants.h Updates
- [x] Add file path constants:
  ```cpp
  constexpr const char* DEPARTMENT_FILE = "data/Department.txt";
  constexpr const char* MEDICINE_FILE = "data/Medicine.txt";
  constexpr const char* PRESCRIPTION_FILE = "data/Prescription.txt";
  constexpr const char* REPORTS_DIR = "data/reports/";
  ```
- [x] Add ID prefixes:
  ```cpp
  constexpr const char* DEPARTMENT_ID_PREFIX = "DEP";
  constexpr const char* MEDICINE_ID_PREFIX = "MED";
  constexpr const char* PRESCRIPTION_ID_PREFIX = "PRE";
  ```
- [x] Add medicine constants:
  ```cpp
  constexpr int DEFAULT_REORDER_LEVEL = 10;
  constexpr int EXPIRY_WARNING_DAYS = 30;
  ```

#### 4.2 Types.h Updates
- [x] Add type aliases:
  ```cpp
  using DepartmentID = std::string;
  using MedicineID = std::string;
  using PrescriptionID = std::string;
  ```

---

### Phase 5: Data Files Creation ✅

#### 5.1 Create Data Files
- [x] Create `data/Department.txt` with header comment
- [x] Create `data/Medicine.txt` with header comment
- [x] Create `data/Prescription.txt` with header comment
- [x] Create `data/reports/` directory

#### 5.2 Create Sample Data
- [x] Add sample departments to `data/sample/Department_sample.txt`
- [x] Add sample medicines to `data/sample/Medicine_sample.txt`
- [x] Add sample prescriptions to `data/sample/Prescription_sample.txt`

---

### Phase 6: Testing ✅

#### 6.1 Model Tests
- [x] Create `test/model/DepartmentTest.cpp`
- [x] Create `test/model/MedicineTest.cpp`
- [x] Create `test/model/PrescriptionTest.cpp`

#### 6.2 DAL Tests
- [x] Create `test/dal/DepartmentRepositoryTest.cpp`
- [x] Create `test/dal/MedicineRepositoryTest.cpp`
- [x] Create `test/dal/PrescriptionRepositoryTest.cpp`

#### 6.3 BLL Tests
- [x] Create `test/bll/DepartmentServiceTest.cpp`
- [x] Create `test/bll/MedicineServiceTest.cpp`
- [x] Create `test/bll/PrescriptionServiceTest.cpp`
- [x] Create `test/bll/ReportGeneratorTest.cpp`

#### 6.4 Test Fixtures
- [x] Create `test/fixtures/Department_test.txt`
- [x] Create `test/fixtures/Medicine_test.txt`
- [x] Create `test/fixtures/Prescription_test.txt`

#### 6.5 Build & Run Tests
- [x] Build tests: `cmake --build build --target HospitalTests`
- [x] Run all tests: `./build/HospitalTests`
- [x] Run specific tests: `./build/HospitalTests --gtest_filter="*Department*:*Medicine*:*Prescription*:*Report*"`
- [x] **All 562 tests PASSED** (100% success rate)

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

**Phase:** ALL CORE IMPLEMENTATION COMPLETE ✅✅✅
**Completed:**
- ✅ Phase 1: Model Layer (3 models)
- ✅ Phase 2: DAL Layer (3 repositories)
- ✅ Phase 3: BLL Layer (3 services + ReportGenerator)
- ✅ Phase 4: Common Layer Updates (Constants.h, Types.h)
- ✅ Phase 5: Data Files Creation (all data + sample files)
- ✅ Phase 6: Testing (10 test files, 562 tests - ALL PASSING)

**Next Step:** Phase 7 - Documentation Updates (optional) OR UI Integration

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
