#!/bin/bash
# ============================================================
# Hospital Management System - Reset Data to Empty State
# ============================================================

echo "=========================================="
echo "  Resetting Data to Empty State"
echo "=========================================="
echo ""

# Ask for confirmation
read -p "This will delete all data and keep only admin account. Continue? (y/n): " confirm

if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Operation cancelled."
    exit 0
fi

echo ""
echo "Resetting all data files..."

# Reset all data files (keep headers only)
for file in data/*.txt; do
    if [ -f "$file" ]; then
        grep '^#' "$file" > "$file.tmp" && mv "$file.tmp" "$file"
    fi
done

# Add back admin account
echo "admin|HASH_admin123|admin|1|2024-01-01" >> data/Account.txt

echo "✓ Data reset completed!"
echo ""
echo "Admin account restored:"
echo "  Username: admin"
echo "  Password: admin123"
echo ""
echo "=========================================="
