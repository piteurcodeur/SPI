#include <iostream>
#include <vector>
#include "mcp2210.h"

#define SPI_CLOCK_SPEED 1000000 // 1 MHz
#define SPI_MODE 0 // SPI mode 0 : CPOL = 0, CPHA = 0
#define NUM_POTS 10 // Nombre de potentiomètres en chaîne

// Envoi d'une trame SPI pour tous les potentiomètres
void sendSPIFrames(hid_device* handle, const std::vector<uint8_t>& commandFrames, std::vector<uint8_t>& responseFrames) {
    if (commandFrames.size() != NUM_POTS * 2) {
        std::cerr << "Erreur : la taille des commandes ne correspond pas au nombre de potentiomètres." << std::endl;
        return;
    }

    uint8_t cmdBuffer[COMMAND_BUFFER_LENGTH] = {0};
    uint8_t responseBuffer[RESPONSE_BUFFER_LENGTH] = {0};

    // Copier les trames de commande dans le buffer
    std::copy(commandFrames.begin(), commandFrames.end(), cmdBuffer);

    // Transfert SPI
    SPIDataTransferStatusDef status = SPISendReceive(handle, cmdBuffer, commandFrames.size(), RESPONSE_BUFFER_LENGTH);

    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        std::cerr << "Erreur SPI : " << status.ErrorCode << std::endl;
        return;
    }

    // Copier les réponses
    responseFrames.assign(status.DataReceived, status.DataReceived + commandFrames.size());
}

// Construction des trames pour une commande donnée
std::vector<uint8_t> buildCommandFrames(uint16_t command, const std::vector<uint16_t>& data) {
    std::vector<uint8_t> frames(NUM_POTS * 2, 0x00);
    for (int i = 0; i < NUM_POTS; ++i) {
        frames[i * 2] = (command & 0xF0) | ((data[i] >> 8) & 0x0F); // Commande et MSB des données
        frames[i * 2 + 1] = data[i] & 0xFF; // LSB des données
    }
    return frames;
}

// Lecture des résistances actuelles
std::vector<uint16_t> readCurrentResistances(hid_device* handle) {
    std::vector<uint8_t> commandFrames = buildCommandFrames(0x08, std::vector<uint16_t>(NUM_POTS, 0x00));
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    sendSPIFrames(handle, commandFrames, responseFrames);

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[i * 2] << 8) | responseFrames[i * 2 + 1];
        std::cout << "Potentiomètre #" << i + 1 << ": Résistance actuelle : " << resistances[i] << "\n";
    }
    return resistances;
}

// Lecture des résistances stockées en mémoire
std::vector<uint16_t> readMemoryResistances(hid_device* handle) {
    std::vector<uint8_t> commandFrames = buildCommandFrames(0x14, std::vector<uint16_t>(NUM_POTS, 0x00));
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    sendSPIFrames(handle, commandFrames, responseFrames);

    std::vector<uint16_t> resistances(NUM_POTS);
    for (int i = 0; i < NUM_POTS; ++i) {
        resistances[i] = (responseFrames[i * 2] << 8) | responseFrames[i * 2 + 1];
        std::cout << "Potentiomètre #" << i + 1 << ": Résistance mémoire : " << resistances[i] << "\n";
    }
    return resistances;
}

// Programmer de nouvelles résistances
void programResistances(hid_device* handle, const std::vector<uint16_t>& resistances) {
    std::vector<uint8_t> commandFrames = buildCommandFrames(0x04, resistances);
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    sendSPIFrames(handle, commandFrames, responseFrames);

    for (int i = 0; i < NUM_POTS; ++i) {
        std::cout << "Potentiomètre #" << i + 1 << ": Résistance programmée : " << resistances[i] << "\n";
    }
}

// Stocker les résistances en mémoire non volatile
void storeResistancesToMemory(hid_device* handle) {
    std::vector<uint8_t> commandFrames = buildCommandFrames(0x0C, std::vector<uint16_t>(NUM_POTS, 0x00));
    std::vector<uint8_t> responseFrames(NUM_POTS * 2);

    sendSPIFrames(handle, commandFrames, responseFrames);

    for (int i = 0; i < NUM_POTS; ++i) {
        std::cout << "Potentiomètre #" << i + 1 << ": Résistance stockée en mémoire.\n";
    }
}

int main() {
    // Initialisation du MCP2210
    hid_device* handle = InitMCP2210();
    if (!handle) {
        std::cerr << "Erreur : Impossible d'initialiser le MCP2210." << std::endl;
        return -1;
    }

    // Configuration des paramètres SPI
    SPITransferSettingsDef spiSettings;
    spiSettings.BitRate = SPI_CLOCK_SPEED;
    spiSettings.SPIMode = SPI_MODE;
    spiSettings.BytesPerSPITransfer = NUM_POTS * 2; // 2 octets par potentiomètre

    int result = SetSPITransferSettings(handle, spiSettings, true);
    if (result != OPERATION_SUCCESSFUL) {
        std::cerr << "Erreur lors de la configuration SPI : " << result << std::endl;
        ReleaseMCP2210(handle);
        return -1;
    }

    // Exemple : Valeurs de résistances à programmer pour chaque potentiomètre
    std::vector<uint16_t> resistances = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

    // Lecture des résistances actuelles
    readCurrentResistances(handle);

    // Lecture des résistances en mémoire
    readMemoryResistances(handle);

    // Programmer les nouvelles résistances
    programResistances(handle, resistances);

    // Stocker les résistances en mémoire non volatile
    storeResistancesToMemory(handle);

    // Libération des ressources
    ReleaseMCP2210(handle);
    return 0;
}
