# Data Directory

This directory contains all data files for the Hospital Management System.

## 📁 Directory Structure

```
data/
├── Account.txt         # User accounts (admin, doctor, patient)
├── Patient.txt         # Patient records
├── Doctor.txt          # Doctor profiles
├── Appointment.txt     # Appointment bookings
├── Medicine.txt        # Pharmacy inventory
├── Department.txt      # Hospital departments
├── Prescription.txt    # Medical prescriptions
├── backup/             # Automatic backups
├── reports/            # Generated reports
└── sample/             # Sample data for testing/demo
```

## 📋 File Formats

All data files use **pipe-delimited (|)** format with the following structure:

```
# Header comments (lines starting with #)
field1|field2|field3|...
```

### Account.txt
```
Format: username|passwordHash|role|isActive|createdDate
Example: admin|HASH_admin123|admin|1|2024-01-01
```

### Patient.txt
```
Format: patientID|username|name|phone|gender|dateOfBirth|address|medicalHistory
Example: P001|patient001|Nguyen Van A|0901234567|Male|1990-01-15|Ha Noi|None
```

### Doctor.txt
```
Format: doctorID|username|name|phone|gender|dateOfBirth|specialization|schedule|consultationFee
Example: D001|doctor001|Dr. Tran B|0902345678|Female|1980-05-20|Cardiology|Mon-Fri 9AM-5PM|500000
```

### Appointment.txt
```
Format: appointmentID|patientUsername|doctorID|date|time|disease|price|isPaid|status|notes
Example: APT001|patient001|D001|2024-03-15|09:00|Fever|500000|1|completed|Follow-up needed
```

### Medicine.txt
```
Format: medicineID|name|genericName|category|manufacturer|description|unitPrice|quantity|reorderLevel|expiryDate|dosageForm|strength
Example: MED001|Paracetamol 500mg|Acetaminophen|Pain Relief|PharmaCorp|For pain and fever|5000|500|100|2025-12-31|Tablet|500mg
```

### Department.txt
```
Format: departmentID|name|description|headDoctorID|doctorIDs|location|phone
Example: DEP001|Cardiology|Heart care|D001|D001,D005,D012|Building A|0281234567
```

### Prescription.txt
```
Format: prescriptionID|appointmentID|patientUsername|doctorID|date|diagnosis|notes|isDispensed|items
Items: medicineID:quantity:dosage:duration:instructions;...
Example: PRE001|APT001|patient001|D001|2024-03-15|Viral Fever|Rest|1|MED001:20:2 tablets 3 times daily:5 days:After meals
```

## 🔑 Default Credentials

**Admin Account:**
- Username: `admin`
- Password: `admin123`

## 📦 Sample Data

The `sample/` directory contains pre-populated data for testing and demonstration purposes. To use sample data:

```bash
# Copy sample data to main data directory
cp data/sample/*.txt data/
```

Sample accounts include:
- 1 admin account
- 2 doctor accounts
- 3 patient accounts

All sample accounts use password: `password123`

## 💾 Backup

The `backup/` directory is used by the system for automatic data backups. Backups are created:
- Before major operations
- On system startup (optional)
- Manually through admin interface

## 📊 Reports

The `reports/` directory stores generated reports:
- Daily reports
- Weekly reports
- Monthly reports
- Revenue reports
- Department statistics

Reports are generated in TXT, CSV, and HTML formats.

## 🔒 Data Security

**Important Notes:**
1. **Password Storage:** Passwords are hashed using a simple prefix-based hash for demonstration. In production, use proper hashing (bcrypt, Argon2).
2. **File Permissions:** Ensure data files have appropriate read/write permissions.
3. **Backup Regularly:** The system provides backup functionality - use it!
4. **Version Control:** Data files are included in .gitignore by default (except headers).

## 🛠️ Maintenance

### Clear All Data
```bash
# Remove all data (keeps headers)
for file in data/*.txt; do
  head -10 "$file" > "$file.tmp" && mv "$file.tmp" "$file"
done
```

### Reset to Sample Data
```bash
# Copy sample data
cp data/sample/Account_sample.txt data/Account.txt
cp data/sample/Doctor_sample.txt data/Doctor.txt
cp data/sample/Patient_sample.txt data/Patient.txt
# ... etc
```

### View Data Statistics
```bash
# Count records in each file
for file in data/*.txt; do
  echo "$(basename $file): $(grep -v '^#' $file | grep -c '^')"
done
```

## 📝 Data Validation Rules

### IDs
- Patient ID: `P` + 3 digits (e.g., P001, P002)
- Doctor ID: `D` + 3 digits (e.g., D001, D002)
- Department ID: `DEP` + 3 digits (e.g., DEP001)
- Medicine ID: `MED` + 3 digits (e.g., MED001)
- Appointment ID: `APT` + 3 digits (e.g., APT001)
- Prescription ID: `PRE` + 3 digits (e.g., PRE001)

### Date Format
- `YYYY-MM-DD` (e.g., 2024-03-15)

### Time Format
- `HH:MM` 24-hour format (e.g., 09:00, 14:30)

### Gender Values
- Male, Female, Other, Unknown

### Appointment Status
- scheduled, completed, cancelled, no_show

### Boolean Values
- `1` = true
- `0` = false

## 🚀 Getting Started

1. **First Run:** Start with only the admin account
   ```bash
   ./build/HospitalApp
   # Login: admin / admin123
   ```

2. **Demo Mode:** Copy sample data for testing
   ```bash
   cp data/sample/*.txt data/
   ./build/HospitalApp
   ```

3. **Production:** Keep files empty and let users register

## 📞 Support

For issues or questions about data management, refer to:
- Main README.md
- Documentation in docs/
- Code comments in DAL layer (src/dal/)
