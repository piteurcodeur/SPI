#ifndef MCP2210_INTERFACE_H
#define MCP2210_INTERFACE_H

#include <vector>
#include "mcp2210.h"

#define NUM_POTS 10

class MCP2210Interface {
public:
    MCP2210Interface();
    ~MCP2210Interface();

    std::vector<uint16_t> readCurrentResistances();
    std::vector<uint16_t> readMemoryResistances();
    void programResistances(const std::vector<uint16_t>& values);
    void storeResistancesToMemory();

private:
    hid_device* handle;
    void sendSPICommand(const std::vector<uint8_t>& commandFrames, std::vector<uint8_t>& responseFrames);
};

#endif