#include "../../include/model/Person.h"
namespace HMS {
    namespace Model {
// ============================= CONSTRUCTOR ==============================
        Person::Person(const std::string& name, const std::string& phone,
                        Gender gender, const std::string& dateOfBirth) {
            this->m_name = name;
            this->m_phone = phone;
            this->m_gender = gender;
            this->m_dateOfBirth = dateOfBirth;
        }
// ============================== GETTER ===============================
        std::string Person::getName() const {
            return this->m_name;
        }
        std::string Person::getPhone() const{
            return this->m_phone;
        }
        Gender Person::getGender() const {
            return this->m_gender;
        }
        std::string Person::getGenderString() const {
            return genderToString(this->m_gender);
        }
        std::string Person::getDateOfBirth() const {
            return this->m_dateOfBirth;
        }
// ============================= SETTER ==========================
        void Person::setName(const std::string& name) {
            this->m_name = name;
        }
        void Person::setPhone(const std::string& phone) {
            this->m_phone = phone;
        }
        void Person::setGender(Gender gender) {
            this->m_gender = gender;
        }
        void Person::setDateOfBirth(const std::string& dateOfBirth) {
            this->m_dateOfBirth = dateOfBirth;
        }
    }
}


