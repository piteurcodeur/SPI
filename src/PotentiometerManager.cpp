#include "PotentiometerManager.h"
#include <stdexcept>

PotentiometerManager::PotentiometerManager() {}

PotentiometerManager::~PotentiometerManager() {}

std::vector<uint16_t> PotentiometerManager::readCurrentResistances() {
    return mcpInterface.readCurrentResistances();
}

std::vector<uint16_t> PotentiometerManager::readMemoryResistances() {
    return mcpInterface.readMemoryResistances();
}

void PotentiometerManager::programResistances(const std::vector<uint16_t>& values) {
    if (values.size() != NUM_POTS) {
        throw std::runtime_error("Le nombre de résistances ne correspond pas au nombre de potentiomètres.");
    }
    mcpInterface.programResistances(values);
}

void PotentiometerManager::storeResistancesToMemory() {
    mcpInterface.storeResistancesToMemory();
}