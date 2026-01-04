# Hospital Management System - Build Guide

## ⚠️ IMPORTANT TIMEZONE NOTICE ⚠️

**The application REQUIRES Vietnam timezone (Asia/Ho_Chi_Minh) for appointment scheduling features to work correctly!**

### Timezone Setup (REQUIRED)

```bash
# RECOMMENDED: Set timezone permanently
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc
```

**Without timezone configuration, appointment scheduling and time validation features will not work correctly!**

---

## Prerequisites

- **CMake** >= 3.10
- **GCC** >= 14 (with C++23 support)
- **Google Test** (libgtest-dev)

The devcontainer configuration includes all prerequisites.

---

## Quick Start

```bash
# Step 1: Set timezone (REQUIRED - only once)
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc

# Step 2: Build application
cd /workspaces/Hospital_Management_Project
mkdir -p build && cd build
cmake ..
make HospitalApp

# Step 3: Run application from project ROOT directory
cd ..
./build/HospitalApp
```

**⚠️ NOTE:** You must run `./build/HospitalApp` from the project root directory, NOT from the `build/` directory

---

## Build Commands

### Configure Project

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake (Debug mode - default)
cmake ..

# Configure with Release mode
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Build Targets

```bash
# Build main application only
make HospitalApp

# Build static library (used by tests)
make HospitalLib

# Build tests only
make HospitalTests

# Build everything
make all

# Fast build with multiple CPU cores
make -j$(nproc)

# Clean build artifacts
make clean
```

### Run Application

```bash
# ⚠️ IMPORTANT: Run from project ROOT directory, NOT from build/
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

**Why run from root directory?**
- Application uses `data/` path to store data
- If run from `build/`, application won't find data files

### Run Tests

```bash
# Run from build directory
cd /workspaces/Hospital_Management_Project/build

# Run all tests
./HospitalTests

# Run tests with verbose output
./HospitalTests --gtest_output=xml:test_results.xml

# Run specific test suite
./HospitalTests --gtest_filter=PatientTest.*

# Run with CTest
ctest --verbose
```

---

## Build Configuration Summary

After running `cmake ..`, you'll see:

```
=== Hospital Management System Build Configuration ===
CMake version:     <version>
C++ Standard:      23
Build type:        Debug
Compiler:          GNU <version>
Project version:   1.0.0

Source directories:
  Model:           src/model/
  DAL:             src/dal/
  BLL:             src/bll/
  UI:              src/ui/
  Common:          src/common/

Build targets:
  HospitalApp      - Main application
  HospitalLib      - Static library
  HospitalTests    - Unit tests (GTest)
======================================================
```

---

## Development Workflow

### Rebuild After Code Changes

```bash
cd /workspaces/Hospital_Management_Project/build
make HospitalApp
cd ..
./build/HospitalApp
```

### Clean Build (When Encountering Errors)

```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
cd ..
./build/HospitalApp
```

### Directory Structure for Source Files

```
src/
├── model/              # Entity implementations
│   ├── Person.cpp
│   ├── Patient.cpp
│   ├── Doctor.cpp
│   ├── Admin.cpp
│   ├── Account.cpp
│   ├── Appointment.cpp
│   ├── Medicine.cpp
│   ├── Department.cpp
│   ├── Prescription.cpp
│   └── Report.cpp
│
├── dal/                # Data Access Layer
│   ├── AccountRepository.cpp
│   ├── PatientRepository.cpp
│   ├── DoctorRepository.cpp
│   ├── AppointmentRepository.cpp
│   ├── MedicineRepository.cpp
│   ├── DepartmentRepository.cpp
│   ├── PrescriptionRepository.cpp
│   └── FileHelper.cpp
│
├── bll/                # Business Logic Layer
│   ├── AuthService.cpp
│   ├── PatientService.cpp
│   ├── DoctorService.cpp
│   ├── AppointmentService.cpp
│   ├── AdminService.cpp
│   ├── MedicineService.cpp
│   ├── DepartmentService.cpp
│   ├── PrescriptionService.cpp
│   └── ReportGenerator.cpp
│
├── ui/                 # Presentation Layer
│   ├── HMSFacade.cpp
│   ├── ConsoleUI.cpp
│   ├── InputValidator.cpp
│   └── DisplayHelper.cpp
│
├── common/             # Utilities
│   └── Utils.cpp
│
└── main.cpp            # Entry point
```

### Directory Structure for Data Files

```
data/
├── Account.txt          # User accounts
├── Patient.txt          # Patient information
├── Doctor.txt           # Doctor information
├── Appointment.txt      # Appointment schedules
├── Medicine.txt         # Medicine catalog
├── Department.txt       # Department catalog
├── Prescription.txt     # Prescriptions
└── Report.txt           # System reports
```

**Note:** CMake no longer copies data files to `build/` directory. The application accesses the `data/` directory in the project root directly to ensure data persistence.

---

## Common Issues

### 1. "Could not open file" error when running application

**Cause:** Running application from wrong directory.

**Solution:** Must run from project root directory:
```bash
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

### 2. Appointment scheduling features not working correctly

**Cause:** Vietnam timezone not configured.

**Solution:**
```bash
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc
```

### 3. "CMake Error: The source directory ... is a file"

**Cause:** Using `cmake ./HospitalApp` instead of `make HospitalApp`.

**Solution:**
```bash
cd build
cmake ..          # Run once to configure
make HospitalApp  # Use this command to build
```

### 4. Build errors after code changes

**Solution:** Clean build:
```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
```

---

## Quick Reference - Common Commands

### Initial Setup (Run once)
```bash
# Set timezone permanently
export TZ='Asia/Ho_Chi_Minh'
echo "export TZ='Asia/Ho_Chi_Minh'" >> ~/.bashrc
source ~/.bashrc

# First build
cd /workspaces/Hospital_Management_Project
mkdir -p build && cd build
cmake ..
make HospitalApp
```

### Run Application
```bash
# Always run from project root directory
cd /workspaces/Hospital_Management_Project
./build/HospitalApp
```

### Rebuild After Code Changes
```bash
cd /workspaces/Hospital_Management_Project/build
make HospitalApp
cd ..
./build/HospitalApp
```

### Run Tests
```bash
cd /workspaces/Hospital_Management_Project/build
./HospitalTests
```

### Clean Build (When encountering errors)
```bash
cd /workspaces/Hospital_Management_Project
rm -rf build
mkdir build && cd build
cmake ..
make HospitalApp
cd ..
./build/HospitalApp
```

### Parallel Build (Faster)
```bash
# Automatically use all CPU cores
make -j$(nproc)
```

---

## Quick System Check

```bash
# Check timezone
echo $TZ  # Should be: Asia/Ho_Chi_Minh

# Check compiler
g++ --version  # Must be >= 14

# Check CMake
cmake --version  # Must be >= 3.10

# Check Google Test
dpkg -l | grep libgtest
```

---

**For more information about system architecture and design, see [ARCHITECTURE.md](ARCHITECTURE.md)**
