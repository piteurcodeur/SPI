#include "mcp2210.h"
#include <stdio.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_RESISTANCE 50000.0
#define NB_COMPO 10
#define FREQUENCY 10000
#define SPI_MODE 1
#define NUM_POTS 10
#define SPI_DATA_TRANSFER_SUCCESS 0

SPIDataTransferStatusDef _status;
hid_device* handle = NULL;

/// AD5270 commands
typedef enum {
  NO_OP = 0x00,               ///< No data
  NO_OP_cmd = 0x0000,         ///< 16 bit no data
  WRITE_RDAC = 0x04,          ///< Write to the RDAC Register
  READ_RDAC = 0x08,           ///< Read from the RDAC Register
  STORE_50TP = 0x0C,          ///< Store RDAC setting to 50-TP
  SW_RST = 0x10,              ///< Software reset to last memory location
  READ_50TP_CONTENTS = 0x14,  ///< Read the last memory contents
  READ_50TP_ADDRESS = 0x18,   ///< Read the last memory address
  WRITE_CTRL_REG = 0x1C,      ///< Write to the control Register
  READ_CTRL_REG = 0x20,       ///< Read from the control Register
  SW_SHUTDOWN = 0x24,         ///< Software shutdown (0) - Normal, (1) - Shutdown
  HI_Zupper = 0x80,           ///< Get the SDO line ready for High Z
  HI_Zlower = 0x01,           ///< Puts AD5270 into High Z mode
  HI_Z_Cmd = 0x8001           ///< Puts AD5270 into High Z mode*/
} AD5270Commands_t;

typedef enum {
  NORMAL_MODE = 0,
  SHUTDOWN_MODE = 1
} AD5270Modes_t;

typedef enum {
  PROGRAM_50TP_ENABLE = 1,
  RDAC_WRITE_PROTECT = 2,
  R_PERFORMANCE_ENABLE = 4,
  MEMORY_PROGRAM_SUCCESFUL = 8
} AD5270ControlRegisterBits_t;

void CheckSPIstatus(hid_device* handle, SPIDataTransferStatusDef _status)
{
    if (_status.ErrorCode != SPI_DATA_TRANSFER_SUCCESS)
    {
        std::cerr << "Erreur de transfert SPI : " << _status.ErrorCode << std::endl;
        return;
    }
}

uint16_t AD5270_ReadReg(hid_device* handle, uint8_t index, uint8_t command) {

    uint8_t data[2 * NB_COMPO];
    uint16_t result = 0;

    for (int i = 0; i < NB_COMPO; i++) {
        if (i == ((NB_COMPO - 1) - index)) {
        data[2 * i] = (command & 0x3C);
        data[2 * i + 1] = 0x00;
        } else {
        data[2 * i] = 0x00;
        data[2 * i + 1] = 0x00;
        }
    }

    SPIDataTransferStatusDef _status = SPIDataTransfer(handle, data, 2 * NB_COMPO);
    CheckSPIstatus(handle, _status);

    usleep(50);  // Délai minimum entre commandes

    memset(data, 0, 2 * NB_COMPO);
    _status = SPISendReceive(handle, data, 2 * NB_COMPO);
    CheckSPIstatus(handle, _status);

    // Utilisation de _status.DataReceived
    int idx = 2 * ((NB_COMPO - 1) - index);
    result = _status.DataReceived[idx];
    result = (result << 8) | _status.DataReceived[idx + 1];

    return result;
}

void AD5270_WriteReg(uint8_t index, uint8_t command, uint16_t value) {
    uint8_t data[2 * NB_COMPO];

    for (int i = 0; i < NB_COMPO; i++)
    {
        if (i == ((NB_COMPO - 1) - index))
        {
            data[2 * i] = (command & 0x3C);
            data[2 * i] |= (uint8_t)((value & 0x0300) >> 8);
            data[2 * i + 1] = (uint8_t)(value & 0x00FF);
        }
        else
        {
            data[2 * i] = 0x00;
            data[2 * i + 1] = 0x00;
        }
    }
    _status = SPIDataTransfer(handle, data, 2 * NB_COMPO);
    CheckSPIstatus(handle, _status);
}



void setup() {
    handle = InitMCP2210();
    if (!handle) {
        std::cerr << "Erreur : Impossible d'initialiser le MCP2210." << std::endl;
        exit(EXIT_FAILURE);
    }
    // Configuration des paramètres SPI
    SPITransferSettingsDef spiSettings;
    spiSettings.BitRate = FREQUENCY; // Vitesse d'horloge SPI
    spiSettings.SPIMode = SPI_MODE; // Mode SPI (0, 1, 2 ou 3)
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
        return;
    }

}

// Ecriture des valeurs sur 10 potentiomètres
void setPots(int index, uint16_t valeurs)
{
    if (valeurs > MAX_RESISTANCE) {
        std::cerr << "Erreur : valeur dépasse la résistance maximale." << std::endl;
        return;
    }

    uint16_t setValue;
    uint16_t RDAC_val = ((uint16_t)((valeurs / MAX_RESISTANCE) * 1024.0));

    AD5270_WriteReg(index, WRITE_CTRL_REG, 0x02);
    usleep(50);
    AD5270_WriteReg(index, WRITE_RDAC, RDAC_val);
}

// Lecture de la valeur RDAC d'un potentiomètre donné (index 0 = dernier de la chaîne)
uint16_t readPot(int index)
{
    uint16_t RDAC_val;

    RDAC_val = AD5270_ReadReg(handle, index, READ_RDAC);
    RDAC_val &= 0x03FF;

    return (uint16_t)(((float)(RDAC_val)*MAX_RESISTANCE) / 1024.0);
}

int main() {


    setup();
    
    uint16_t valeurs[10]  = {1000 , 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000};

    // Exemple : valeurs progressives
    for (int i = 0; i < 10; i++)
    {
        setPots(i, valeurs[i]); // Valeurs de 1000 à 10000
    }

    // Lecture et affichage
    std::cout << "Lecture des potentiomètres :" << std::endl;
    for (int i = 0; i < NB_COMPO; i++) {
        uint16_t lu = readPot(i);

        std::cout << "Pot " << i << " : " << lu << std::endl;

        usleep(10);  // Pour lisibilité console
    }
    std::cout << "-----------------------" << std::endl;
usleep(1000);

    return EXIT_SUCCESS;
}
