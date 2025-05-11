#include "uuid.h"

#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>

namespace uuid
{
    std::string generate() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(0, 15);

        std::stringstream uuidStream;

        for (int i = 0; i < 8; ++i) {
            uuidStream << std::hex << dis(gen);
        }
        uuidStream << "-";
        for (int i = 0; i < 4; ++i) {
            uuidStream << std::hex << dis(gen);
        }
        uuidStream << "-";
        for (int i = 0; i < 4; ++i) {
            uuidStream << std::hex << dis(gen);
        }
        uuidStream << "-";
        for (int i = 0; i < 4; ++i) {
            uuidStream << std::hex << dis(gen);
        }
        uuidStream << "-";
        for (int i = 0; i < 12; ++i) {
            uuidStream << std::hex << dis(gen);
        }

        return uuidStream.str();
    }
}