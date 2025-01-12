#include "MCP2210Interface.h"
#include <stdexcept>
#include <cstring>

MCP2210Interface::MCP2210Interface() {
    handle = InitMCP2210();
    if (!handle) {
        throw std::runtime_error("Impossible d'initialiser le MCP2210.");
    }
}

MCP2210Interface::~MCP2210Interface() {
    ReleaseMCP2210(handle);
}

void MCP2210Interface::sendSPICommand(const std::vector<uint8_t>& commandFrames, std::vector<uint8_t>& responseFrames) {
    size_t totalBytes = commandFrames.size() + NUM_POTS * 2; // Ajout de trames vides pour récupérer toutes les réponses.
    std::vector<uint8_t> extendedCommandFrames(totalBytes, 0x00); // Commandes + trames vides
    std::copy(commandFrames.begin(), commandFrames.end(), extendedCommandFrames.begin());

    uint8_t cmdBuffer[COMMAND_BUFFER_LENGTH] = {0};
    uint8_t responseBuffer[COMMAND_BUFFER_LENGTH] = {0};

    std::memcpy(cmdBuffer, extendedCommandFrames.data(), extendedCommandFrames.size());

    SPIDataTransferStatusDef status = SPISendReceive(handle, cmdBuffer, extendedCommandFrames.size(), extendedCommandFrames.size());

    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        throw std::runtime_error("Erreur lors du transfert SPI.");
    }

    responseFrames.assign(status.DataReceived, status.DataReceived + extendedCommandFrames.size());
}

std::vector<uint16_t> MCP2210Interface::readCurrentResistances() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x08); // Commande de lecture
    std::vector<uint8_t> responseFrames;

    sendSPICommand(commandFrames, responseFrames);

    // Décaler la réponse pour ignorer les trames initiales vides
    size_t offset = NUM_POTS * 2; // Nombre de trames vides initiales
    if (responseFrames.size() < offset + NUM_POTS * 2) {
        throw std::runtime_error("Erreur : trame SPI insuffisante en réponse.");
    }

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[offset + i * 2] << 8) | responseFrames[offset + i * 2 + 1];
    }

    return resistances;
}

std::vector<uint16_t> MCP2210Interface::readMemoryResistances() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x14); // Commande de lecture mémoire
    std::vector<uint8_t> responseFrames;

    sendSPICommand(commandFrames, responseFrames);

    // Décaler la réponse pour ignorer les trames initiales vides
    size_t offset = NUM_POTS * 2;
    if (responseFrames.size() < offset + NUM_POTS * 2) {
        throw std::runtime_error("Erreur : trame SPI insuffisante en réponse.");
    }

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[offset + i * 2] << 8) | responseFrames[offset + i * 2 + 1];
    }

    return resistances;
}

void MCP2210Interface::programResistances(const std::vector<uint16_t>& values) {
    if (values.size() != NUM_POTS) {
        throw std::runtime_error("Erreur : le nombre de valeurs ne correspond pas au nombre de potentiomètres.");
    }

    std::vector<uint8_t> commandFrames(NUM_POTS * 2);
    for (int i = 0; i < NUM_POTS; ++i) {
        commandFrames[i * 2] = 0x04 | ((values[i] >> 8) & 0x0F); // Commande d'écriture
        commandFrames[i * 2 + 1] = values[i] & 0xFF;             // Valeur faible
    }

    std::vector<uint8_t> responseFrames(NUM_POTS * 2);
    sendSPICommand(commandFrames, responseFrames);
}

void MCP2210Interface::storeResistancesToMemory() {
    std::vector<uint8_t> commandFrames(NUM_POTS * 2, 0x0C); // Commande de stockage
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    sendSPICommand(commandFrames, responseFrames);
}

