#!/bin/bash
# ============================================================
# Hospital Management System - Run with Sample Data
# ============================================================

echo "=========================================="
echo "  Hospital Management System"
echo "  Loading Sample Data..."
echo "=========================================="
echo ""

# 1. Copy sample data to main data directory
echo "[1/4] Copying sample data files..."
cp data/sample/Account_sample.txt data/Account.txt
cp data/sample/Doctor_sample.txt data/Doctor.txt
cp data/sample/Patient_sample.txt data/Patient.txt
cp data/sample/Appointment_sample.txt data/Appointment.txt
cp data/sample/Medicine_sample.txt data/Medicine.txt
cp data/sample/Department_sample.txt data/Department.txt
cp data/sample/Prescription_sample.txt data/Prescription.txt
echo "✓ Sample data copied successfully!"
echo ""

# 2. Build if needed
echo "[2/4] Building application..."
cmake --build build
echo "✓ Build completed!"
echo ""

# 3. Show available accounts
echo "[3/4] Sample accounts loaded:"
echo "=========================================="
echo "Admin:"
echo "  • Username: admin"
echo "  • Password: password123"
echo ""
echo "Doctors:"
echo "  • doctor001 / password123 (Cardiology)"
echo "  • doctor002 / password123 (Pediatrics)"
echo ""
echo "Patients:"
echo "  • patient001 / password123 (Nguyen Van An)"
echo "  • patient002 / password123 (Tran Thi Binh)"
echo "  • patient003 / password123 (Le Van Cuong)"
echo "=========================================="
echo ""

# 4. Count records
echo "[4/4] Data Summary:"
echo "  Accounts:      $(grep -v '^#' data/Account.txt | grep -c '^') records"
echo "  Doctors:       $(grep -v '^#' data/Doctor.txt | grep -c '^') records"
echo "  Patients:      $(grep -v '^#' data/Patient.txt | grep -c '^') records"
echo "  Appointments:  $(grep -v '^#' data/Appointment.txt | grep -c '^') records"
echo "  Medicines:     $(grep -v '^#' data/Medicine.txt | grep -c '^') records"
echo "  Departments:   $(grep -v '^#' data/Department.txt | grep -c '^') records"
echo "  Prescriptions: $(grep -v '^#' data/Prescription.txt | grep -c '^') records"
echo ""
echo "=========================================="
echo "Starting application..."
echo "=========================================="
echo ""

# 5. Run the application
./build/HospitalApp
