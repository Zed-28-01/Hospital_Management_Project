#include "model/Person.h"

namespace HMS {
namespace Model {

// ==================== Constructor ====================
Person::Person(const std::string& name,
               const std::string& phone,
               Gender gender,
               const std::string& dateOfBirth)
    : m_name(name),
      m_phone(phone),
      m_gender(gender),
      m_dateOfBirth(dateOfBirth) {}

// ==================== Getters ====================
std::string Person::getName() const {
    return m_name;
}

std::string Person::getPhone() const {
    return m_phone;
}

Gender Person::getGender() const {
    return m_gender;
}

std::string Person::getGenderString() const {
    return genderToString(m_gender);
}

std::string Person::getDateOfBirth() const {
    return m_dateOfBirth;
}

// ==================== Setters ====================
void Person::setName(const std::string& name) {
    m_name = name;
}

void Person::setPhone(const std::string& phone) {
    m_phone = phone;
}

void Person::setGender(Gender gender) {
    m_gender = gender;
}

void Person::setDateOfBirth(const std::string& dateOfBirth) {
    m_dateOfBirth = dateOfBirth;
}

} // namespace Model
} // namespace HMS