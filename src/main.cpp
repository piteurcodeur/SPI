#include <iostream>
#include <vector>
#include "PotentiometerManager.h"
#include <string>

void printHelp() {
    std::cout << "Usage: mcp2210_cli [options]\n"
              << "Options:\n"
              << "  --read-current         Lire les résistances actuelles\n"
              << "  --read-memory          Lire les résistances stockées en mémoire\n"
              << "  --set [values...]      Programmer des résistances (valeurs séparées par des espaces)\n"
              << "  --store                Stocker les résistances programmées en mémoire\n"
              << "  --help                 Afficher l'aide\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    std::string command = argv[1];
    PotentiometerManager manager;

    try {
        if (command == "--read-current") {
            auto resistances = manager.readCurrentResistances();
            for (size_t i = 0; i < resistances.size(); ++i) {
                std::cout << "Potentiomètre #" << i + 1 << ": " << resistances[i] << " ohms\n";
            }
        } else if (command == "--read-memory") {
            auto resistances = manager.readMemoryResistances();
            for (size_t i = 0; i < resistances.size(); ++i) {
                std::cout << "Potentiomètre #" << i + 1 << ": " << resistances[i] << " ohms\n";
            }
        } else if (command == "--set") {
            if (argc < 3) {
                std::cerr << "Erreur : aucune valeur fournie pour --set\n";
                return 1;
            }
            std::vector<uint16_t> values;
            for (int i = 2; i < argc; ++i) {
                values.push_back(static_cast<uint16_t>(std::stoi(argv[i])));
            }
            manager.programResistances(values);
        } else if (command == "--store") {
            manager.storeResistancesToMemory();
        } else if (command == "--help") {
            printHelp();
        } else {
            std::cerr << "Erreur : commande inconnue \"" << command << "\"\n";
            printHelp();
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << "\n";
        return 1;
    }

    return 0;
}