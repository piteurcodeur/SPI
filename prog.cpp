#include <iostream>
#include <vector>
#include "mcp2210.h"

#define SPI_CLOCK_SPEED 1000000 // 1 MHz
#define SPI_MODE 0 // SPI mode 0 : CPOL = 0, CPHA = 0
#define NUM_POTS 10 // Nombre de potentiomètres en chaîne


void sendSPICommand(hid_device* handle, const std::vector<uint8_t>& command, std::vector<uint8_t>& response) {
    uint8_t cmdBuffer[COMMAND_BUFFER_LENGTH] = {0};
    uint8_t responseBuffer[RESPONSE_BUFFER_LENGTH] = {0};

    // Copier la commande dans le buffer
    std::copy(command.begin(), command.end(), cmdBuffer);

    // Transfert SPI
    SPIDataTransferStatusDef status = SPISendReceive(handle, cmdBuffer, command.size(), RESPONSE_BUFFER_LENGTH);

    if (status.ErrorCode != OPERATION_SUCCESSFUL) {
        std::cerr << "Erreur SPI : " << status.ErrorCode << std::endl;
        return;
    }

    // Copier la réponse
    std::copy(status.DataReceived, status.DataReceived + RESPONSE_BUFFER_LENGTH, response.begin());
}

void programPotentiometers(hid_device* handle) {
    std::vector<uint8_t> cmdEnableWrite = {0x1C, 0x03}; // Activer l'écriture dans RDAC
    std::vector<uint8_t> cmdWriteResistance = {0x05, 0x00}; // Programmer une résistance (1/4 échelle)
    std::vector<uint8_t> cmdReadResistance = {0x08, 0x00}; // Lire la résistance programmée
    std::vector<uint8_t> cmdStoreToMemory = {0x0C, 0x00}; // Stocker dans la mémoire 50-TP

    std::vector<uint8_t> response(RESPONSE_BUFFER_LENGTH);

    for (int potIndex = 0; potIndex < NUM_POTS; ++potIndex) {
        std::cout << "Potentiomètre #" << potIndex + 1 << " :" << std::endl;

        // Étape 1 : Activer l'écriture dans RDAC
        sendSPICommand(handle, cmdEnableWrite, response);
        std::cout << "  Écriture dans RDAC activée." << std::endl;

        // Étape 2 : Programmer une résistance
        sendSPICommand(handle, cmdWriteResistance, response);
        std::cout << "  Résistance programmée à 1/4 de l'échelle." << std::endl;

        // Étape 3 : Lire la résistance programmée
        sendSPICommand(handle, cmdReadResistance, response);
        uint16_t resistance = (response[0] << 8) | response[1];
        std::cout << "  Résistance lue : " << resistance << std::endl;

        // Étape 4 : Stocker dans la mémoire non volatile
        sendSPICommand(handle, cmdStoreToMemory, response);
        std::cout << "  Résistance stockée dans la mémoire non volatile." << std::endl;
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
    spiSettings.BytesPerSPITransfer = 2 * NUM_POTS; // 2 octets par potentiomètre

    int result = SetSPITransferSettings(handle, spiSettings, true);
    if (result != OPERATION_SUCCESSFUL) {
        std::cerr << "Erreur lors de la configuration SPI : " << result << std::endl;
        ReleaseMCP2210(handle);
        return -1;
    }

    // Exécution de la séquence de programmation
    programPotentiometers(handle);

    // Libération des ressources
    ReleaseMCP2210(handle);
    return 0;
}
