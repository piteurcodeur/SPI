#include <iostream>
#include <vector>
#include "mcp2210.h"


#define SPI_CLOCK_SPEED 31000 // Vitesse d'horloge SPI (en kHz)
#define SPI_MODE 1
#define NUM_POTS 10 // Nombre de potentiomètres en chaîne

// Envoi d'une commande SPI
void sendSPICommand(hid_device* handle, const std::vector<uint8_t>& command)
{
    
}


// Lire la valeur actuelle de la résistance
uint16_t readCurrentResistance(hid_device* handle, int potIndex) {
    std::vector<uint8_t> cmdReadResistance = {0x08, 0x00};
    std::vector<uint8_t> response(RESPONSE_BUFFER_LENGTH);
    sendSPICommand(handle, cmdReadResistance, response);
    uint16_t resistance = (response[0] << 8) | response[1];
    std::cout << "Potentiomètre #" << potIndex + 1 << ": Résistance actuelle : " << resistance << "\n";
    return resistance;
}

// Lire la valeur de résistance stockée en mémoire
uint16_t readMemoryResistance(hid_device* handle, int potIndex) {
    std::vector<uint8_t> cmdReadMemory = {0x14, 0x00};
    std::vector<uint8_t> response(RESPONSE_BUFFER_LENGTH);
    sendSPICommand(handle, cmdReadMemory, response);
    uint16_t resistance = (response[0] << 8) | response[1];
    std::cout << "Potentiomètre #" << potIndex + 1 << ": Résistance mémoire : " << resistance << "\n";
    return resistance;
}

// Programmer une nouvelle résistance
void programResistance(hid_device* handle, int potIndex, uint16_t resistance) {
    std::vector<uint8_t> cmdProgramResistance = {0x04, static_cast<uint8_t>(resistance >> 8), static_cast<uint8_t>(resistance & 0xFF)};
    std::vector<uint8_t> response(RESPONSE_BUFFER_LENGTH);
    sendSPICommand(handle, cmdProgramResistance, response);
    std::cout << "Potentiomètre #" << potIndex + 1 << ": Résistance programmée : " << resistance << "\n";
}

// Stocker la résistance programmée en mémoire non volatile
void storeResistanceToMemory(hid_device* handle, int potIndex) {
    std::vector<uint8_t> cmdStoreToMemory = {0x0C, 0x00};
    std::vector<uint8_t> response(RESPONSE_BUFFER_LENGTH);
    sendSPICommand(handle, cmdStoreToMemory, response);
    std::cout << "Potentiomètre #" << potIndex + 1 << ": Résistance stockée en mémoire.\n";
}

// Fonction principale pour programmer les potentiomètres avec les valeurs spécifiées
void processPotentiometers(hid_device* handle, const std::vector<uint16_t>& resistances) {
    if (resistances.size() != NUM_POTS) {
        std::cerr << "Erreur : le nombre de résistances ne correspond pas au nombre de potentiomètres.\n";
        return;
    }

    for (int potIndex = 0; potIndex < NUM_POTS; ++potIndex) {
        // Lire la résistance actuelle
        readCurrentResistance(handle, potIndex);

        // Lire la résistance en mémoire
        readMemoryResistance(handle, potIndex);

        // Programmer la nouvelle résistance
        programResistance(handle, potIndex, resistances[potIndex]);

        // Stocker la résistance en mémoire non volatile
        storeResistanceToMemory(handle, potIndex);
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
    spiSettings.IdleChipSelectValue = 0xffff; // Valeur de CS inactive
    spiSettings.ActiveChipSelectValue = 0xffef; // Valeur de CS active
    spiSettings.CSToDataDelay = 0; // Pas de délai entre CS et données
    spiSettings.LastDataByteToCSDelay = 0; // Pas de délai entre le dernier octet de données et CS
    spiSettings.SubsequentDataByteDelay = 0; // Pas de délai entre les octets de données

    int result = SetSPITransferSettings(handle, spiSettings, true);
    if (result != OPERATION_SUCCESSFUL) {
        std::cerr << "Erreur lors de la configuration SPI : " << result << std::endl;
        ReleaseMCP2210(handle);
        return -1;
    }

    // Valeurs de résistances à programmer pour chaque potentiomètre
    std::vector<uint16_t> resistances = {10000, 10000, 10000, 10000, 10000,
                                          10000, 10000, 10000, 10000, 10000};

    //initialisation des potentiomètres
    std::vector<uint8_t> cmdInit = {0x1C, 0x03};

   

    processPotentiometers(handle, resistances);

    // Libération des ressources
    ReleaseMCP2210(handle);
    return 0;
}
