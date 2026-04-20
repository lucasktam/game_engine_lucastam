#include <iostream>

class ComponentDB {
public:
    static void CppLog(const std::string& message) {
        std::cout << message << std::endl;
    }
    static void CppLogError(const std::string& message) {
        std::cout << message << std::endl;
    }
};