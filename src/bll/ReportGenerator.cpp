#include "advance/ReportGenerator.h"

namespace HMS {
    namespace BLL {
// ======================================== SINGLETON INIT =========================================

        ReportGenerator *ReportGenerator::s_instance = nullptr;

        ReportGenerator *ReportGenerator::getInstance() {
            if (!s_instance) {
                s_instance = new ReportGenerator();

            }
            return s_instance;
        }

    }
}