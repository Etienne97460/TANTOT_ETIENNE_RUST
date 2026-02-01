#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip> // Pour l'alignement du texte
#include <limits>  // Pour vider le buffer cin

// =============================================================
// 1. OUTILS VISUELS (UI & COULEURS)
// =============================================================

namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[1;31m";
    const std::string GREEN   = "\033[1;32m";
    const std::string YELLOW  = "\033[1;33m";
    const std::string BLUE    = "\033[1;34m";
    const std::string CYAN    = "\033[1;36m";
    const std::string MAGENTA = "\033[1;35m";
    const std::string GRAY    = "\033[1;90m";
}

void clearScreen() {
    // Séquence ANSI pour effacer l'écran (fonctionne sur Linux/Mac et Win10+)
    std::cout << "\033[2J\033[1;1H";
}

void drawHeader() {
    std::cout << Color::GREEN;
    std::cout << "╔═══════════════════════════════════════════════╗\n";
    std::cout << "║         DISTRIBUTEUR AUTOMATIQUE 3000         ║\n";
    std::cout << "╚═══════════════════════════════════════════════╝\n";
    std::cout << Color::RESET;
}

// =============================================================
// 2. TYPES DE DONNÉES (DATA)
// =============================================================

// Une Enum pour les types de produits (utile pour le portage Rust !)
enum class Category { DRINK, SNACK, TECH };

struct Product {
    int id;
    std::string name;
    float price;
    int stock;
    Category type;

    // Helper pour afficher l'icône selon la catégorie
    std::string getIcon() const {
        switch(type) {
            case Category::DRINK: return "[Boisson]";
            case Category::SNACK: return "[ Snack ]";
            case Category::TECH:  return "[ Tech  ]";
            default: return "[ ? ]";
        }
    }
};

// =============================================================
// 3. ABSTRACTION MATÉRIELLE (DRIVERS)
// =============================================================

class Hardware {
public:
    // Simulation d'un écran LCD 2 lignes
    static void lcdPrint(const std::string& line1, const std::string& line2 = "") {
        std::cout << "\n" << Color::BLUE << "  [LCD] ┌───────────────────────────────────────┐" << Color::RESET << "\n";
        std::cout << Color::BLUE << "  [LCD] │ " << Color::YELLOW << std::left << std::setw(37) << line1 << Color::BLUE << " │" << Color::RESET << "\n";
        if (!line2.empty()) {
             std::cout << Color::BLUE << "  [LCD] │ " << Color::YELLOW << std::left << std::setw(37) << line2 << Color::BLUE << " │" << Color::RESET << "\n";
        }
        std::cout << Color::BLUE << "  [LCD] └───────────────────────────────────────┘" << Color::RESET << "\n\n";
    }

    // Simulation du moteur avec barre de progression
    static void activateMotor(const std::string& productName) {
        std::cout << Color::MAGENTA << "  [MOTEUR] Livraison en cours : " << productName << Color::RESET << "\n  ";
        
        for (int i = 0; i <= 20; ++i) {
            std::cout << "▓";
            std::cout.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Vitesse moteur
        }
        std::cout << " OK!\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << Color::GREEN << "  >>> CLONG ! Le produit est tombé dans le bac. <<<" << Color::RESET << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    // Simulation du monnayeur (Accepteur de pièces)
    static void coinSound() {
        std::cout << Color::GRAY << "  (Clink! Pièce acceptée...)" << Color::RESET << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
};

// =============================================================
// 4. LOGIQUE MÉTIER (CORE)
// =============================================================

class VendingMachine {
private:
    std::vector<Product> inventory;
    float currentCredit = 0.0f;

public:
    VendingMachine() {
        // Initialisation de l'inventaire complet
        inventory = {
            {10, "Coca-Cola Zero", 1.20f, 5, Category::DRINK},
            {11, "Ice Tea Peche ", 1.40f, 4, Category::DRINK},
            {12, "Eau Minerale  ", 0.80f, 8, Category::DRINK},
            {20, "Kinder Bueno  ", 1.10f, 6, Category::SNACK},
            {21, "M&Ms Peanut   ", 1.30f, 3, Category::SNACK},
            {22, "Chips Nature  ", 1.00f, 2, Category::SNACK},
            {30, "Cable USB-C   ", 5.50f, 2, Category::TECH},
            {31, "Ecouteurs     ", 8.00f, 1, Category::TECH}
        };
    }

    void run() {
        while (true) {
            clearScreen();
            drawHeader();
            displayInventory();
            displayControls();
            
            // Affichage du crédit actuel sur le LCD
            std::string creditMsg = "CREDIT : " + formatMoney(currentCredit);
            Hardware::lcdPrint("Veuillez choisir un produit", creditMsg);

            handleUserAction();
        }
    }

private:
    std::string formatMoney(float amount) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << amount << " EUR";
        return ss.str();
    }

    void displayInventory() {
        std::cout << "  ID  | TYPE      | NOM             | PRIX    | STOCK \n";
        std::cout << "  ----+-----------+-----------------+---------+-------\n";
        
        for (const auto& item : inventory) {
            // Choix de la couleur selon le stock
            std::string stockColor = (item.stock > 0) ? Color::GREEN : Color::RED;
            std::string stockText  = (item.stock > 0) ? std::to_string(item.stock) : "RUPTURE";

            std::cout << "  " << Color::CYAN << std::setw(3) << item.id << Color::RESET << " | "
                      << item.getIcon() << " | "
                      << std::left << std::setw(15) << item.name << " | "
                      << Color::YELLOW << std::fixed << std::setprecision(2) << item.price << " €" << Color::RESET << " | "
                      << stockColor << stockText << Color::RESET << "\n";
        }
        std::cout << "\n";
    }

    void displayControls() {
        std::cout << "  [PIECES] : 1 = 0.10 EUR, 2 = 0.20 EUR, 3 = 0.50 EUR, 4 = 1.00 EUR, 5 = 2.00 EUR\n";
        std::cout << "  [ACTION] : Tapez l'ID du produit pour acheter\n";
        std::cout << "  [SYSTEM] : 99 pour Rendre monnaie / 0 pour Quitter\n";
        std::cout << Color::GRAY << "  Votre choix > " << Color::RESET;
    }

    void handleUserAction() {
        int input;
        std::cin >> input;

        // Gestion des erreurs de saisie (si l'utilisateur tape des lettres)
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return;
        }

        // --- GESTION DES PIÈCES ---
        if (input >= 1 && input <= 5) {
            float coinValue = 0.0f;
            switch(input) {
                case 1: coinValue = 0.10f; break;
                case 2: coinValue = 0.20f; break;
                case 3: coinValue = 0.50f; break;
                case 4: coinValue = 1.00f; break;
                case 5: coinValue = 2.00f; break;
            }
            Hardware::coinSound();
            currentCredit += coinValue;
            return;
        }

        // --- GESTION SYSTÈME ---
        if (input == 0) exit(0);
        
        if (input == 99) {
            if (currentCredit > 0) {
                Hardware::lcdPrint("RENDU MONNAIE...", formatMoney(currentCredit));
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                currentCredit = 0.0f;
            }
            return;
        }

        // --- GESTION ACHAT ---
        processPurchase(input);
    }

    void processPurchase(int productId) {
        // Recherche du produit
        Product* selected = nullptr;
        for (auto& item : inventory) {
            if (item.id == productId) {
                selected = &item;
                break;
            }
        }

        if (selected == nullptr) {
            Hardware::lcdPrint("ERREUR", "ID Inconnu !");
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return;
        }

        if (selected->stock <= 0) {
            Hardware::lcdPrint("RUPTURE DE STOCK", "Choisissez un autre");
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return;
        }

        if (currentCredit < selected->price) {
            float missing = selected->price - currentCredit;
            Hardware::lcdPrint("CREDIT INSUFFISANT", "Manque : " + formatMoney(missing));
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            return;
        }

        // Tout est OK : Transaction
        Hardware::lcdPrint("DISTRIBUTION...", selected->name);
        
        // Logique Transactionnelle
        currentCredit -= selected->price;
        selected->stock--;

        // Activation Hardware
        Hardware::activateMotor(selected->name);
        
        // Rendu monnaie automatique si on veut (optionnel)
        // Ici on garde le crédit pour un autre achat
    }
};

// =============================================================
// 5. POINT D'ENTRÉE
// =============================================================

int main() {
    // Config console pour afficher les caractères spéciaux si besoin (sur Windows vieux)
    // system("chcp 65001"); 
    
    VendingMachine machine;
    machine.run();
    return 0;
}