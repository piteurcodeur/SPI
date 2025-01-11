#include "MCP2210Interface.h"
#include <stdexcept>

MCP2210Interface::MCP2210Interface() {
    handle = InitMCP2210();
    if (!handle) {
        throw std::runtime_error("Impossible d'initialiser le MCP2210.");
    }
}

MCP2210Interface::~MCP2210Interface() {
    ReleaseMCP2210(handle);
}

std::vector<uint16_t> MCP2210Interface::readCurrentResistances() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x08);
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    SPIDataTransferStatusDef status = SPISendReceive(handle, commandFrames.data(), commandFrames.size(), commandFrames.size());
    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        throw std::runtime_error("Erreur lors de la lecture des résistances actuelles.");
    }

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[i * 2] << 8) | responseFrames[i * 2 + 1];
    }
    return resistances;
}

std::vector<uint16_t> MCP2210Interface::readMemoryResistances() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x14);
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    SPIDataTransferStatusDef status = SPISendReceive(handle, commandFrames.data(), commandFrames.size(), commandFrames.size());
    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        throw std::runtime_error("Erreur lors de la lecture des résistances en mémoire.");
    }

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[i * 2] << 8) | responseFrames[i * 2 + 1];
    }
    return resistances;
}

void MCP2210Interface::programResistances(const std::vector<uint16_t>& values) {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2);
    for (int i = 0; i < NUM_POTS; ++i) {
        commandFrames[i * 2] = 0x04 | ((values[i] >> 8) & 0x0F);
        commandFrames[i * 2 + 1] = values[i] & 0xFF;
    }

    SPIDataTransferStatusDef status = SPISendReceive(handle, commandFrames.data(), commandFrames.size(), commandFrames.size());
    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        throw std::runtime_error("Erreur lors de la programmation des résistances.");
    }
}

void MCP2210Interface::storeResistancesToMemory() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x0C);
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    SPIDataTransferStatusDef status = SPISendReceive(handle, commandFrames.data(), commandFrames.size(), commandFrames.size());
    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        throw std::runtime_error("Erreur lors du stockage des résistances en mémoire.");
    }
}
