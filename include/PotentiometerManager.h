#ifndef POTENTIOMETER_MANAGER_H
#define POTENTIOMETER_MANAGER_H

#include <vector>
#include "MCP2210Interface.h"

class PotentiometerManager {
public:
    PotentiometerManager();
    ~PotentiometerManager();

    std::vector<uint16_t> readCurrentResistances();
    std::vector<uint16_t> readMemoryResistances();
    void programResistances(const std::vector<uint16_t>& values);
    void storeResistancesToMemory();

private:
    MCP2210Interface mcpInterface;
};

#endif