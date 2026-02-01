use std::io::{self, Write};
use std::{thread, time};

// =============================================================
// 1. OUTILS VISUELS (UI & COULEURS)
// =============================================================
mod color {
    pub const RESET: &str = "\x1b[0m";
    pub const RED: &str = "\x1b[1;31m";
    pub const GREEN: &str = "\x1b[1;32m";
    pub const YELLOW: &str = "\x1b[1;33m";
    pub const BLUE: &str = "\x1b[1;34m";
    pub const MAGENTA: &str = "\x1b[1;35m";
    pub const CYAN: &str = "\x1b[1;36m";
    pub const GRAY: &str = "\x1b[1;90m";
}

fn clear_screen() {
    print!("\x1b[2J\x1b[1;1H");
}

fn draw_header() {
    println!("{}{}", color::GREEN, "╔═══════════════════════════════════════════════╗");
    println!("║         DISTRIBUTEUR AUTOMATIQUE 3000         ║");
    println!("╚═══════════════════════════════════════════════╝{}", color::RESET);
}

// =============================================================
// 2. TYPES DE DONNÉES (DATA)
// =============================================================

#[derive(Debug, Clone, PartialEq)]
enum Category {
    Drink,
    Snack,
    Tech,
}

#[derive(Clone)]
struct Product {
    id: i32,
    name: String,
    price: f32,
    stock: i32,
    category: Category,
}

impl Product {
    fn get_icon(&self) -> &str {
        match self.category {
            Category::Drink => "[Boisson]",
            Category::Snack => "[ Snack ]",
            Category::Tech => "[ Tech  ]",
        }
    }
}

// =============================================================
// 3. ABSTRACTION MATÉRIELLE (DRIVERS)
// =============================================================

struct Hardware;

impl Hardware {
    fn lcd_print(line1: &str, line2: &str) {
        println!("\n{}  [LCD] ┌───────────────────────────────────────┐{}", color::BLUE, color::RESET);
        println!("{}  [LCD] │ {}{: <37}{} │{}", color::BLUE, color::YELLOW, line1, color::BLUE, color::RESET);
        if !line2.is_empty() {
            println!("{}  [LCD] │ {}{: <37}{} │{}", color::BLUE, color::YELLOW, line2, color::BLUE, color::RESET);
        }
        println!("{}  [LCD] └───────────────────────────────────────┘{}\n", color::BLUE, color::RESET);
    }

    fn activate_motor(product_name: &str) {
        println!("{}  [MOTEUR] Livraison en cours : {}{}", color::MAGENTA, product_name, color::RESET);
        print!("  ");
        io::stdout().flush().unwrap();

        for _ in 0..20 {
            print!("▓");
            io::stdout().flush().unwrap();
            thread::sleep(time::Duration::from_millis(100));
        }
        println!(" OK!");
        thread::sleep(time::Duration::from_millis(500));
        println!("{}  >>> CLONG ! Le produit est tombé dans le bac. <<<{}", color::GREEN, color::RESET);
        thread::sleep(time::Duration::from_millis(1500));
    }

    fn coin_sound() {
        println!("{}  (Clink! Pièce acceptée...){}", color::GRAY, color::RESET);
        thread::sleep(time::Duration::from_millis(300));
    }
}

// =============================================================
// 4. LOGIQUE MÉTIER (CORE)
// =============================================================

struct VendingMachine {
    inventory: Vec<Product>,
    current_credit: f32,
}

impl VendingMachine {
    fn new() -> VendingMachine {
        VendingMachine {
            current_credit: 0.0,
            inventory: vec![
                Product { id: 10, name: String::from("Coca-Cola Zero"), price: 1.20, stock: 5, category: Category::Drink },
                Product { id: 11, name: String::from("Ice Tea Peche "), price: 1.40, stock: 4, category: Category::Drink },
                Product { id: 12, name: String::from("Eau Minerale  "), price: 0.80, stock: 8, category: Category::Drink },
                Product { id: 20, name: String::from("Kinder Bueno  "), price: 1.10, stock: 6, category: Category::Snack },
                Product { id: 21, name: String::from("M&Ms Peanut   "), price: 1.30, stock: 3, category: Category::Snack },
                Product { id: 22, name: String::from("Chips Nature  "), price: 1.00, stock: 2, category: Category::Snack },
                Product { id: 30, name: String::from("Cable USB-C   "), price: 5.50, stock: 2, category: Category::Tech },
                Product { id: 31, name: String::from("Ecouteurs     "), price: 8.00, stock: 1, category: Category::Tech },
            ],
        }
    }

    fn run(&mut self) {
        loop {
            clear_screen();
            draw_header();
            self.display_inventory();
            self.display_controls();

            let credit_msg = format!("CREDIT : {:.2} EUR", self.current_credit);
            Hardware::lcd_print("Veuillez choisir un produit", &credit_msg);

            self.handle_user_action();
        }
    }

    fn display_inventory(&self) {
        println!("  ID  | TYPE      | NOM               | PRIX    | STOCK ");
        println!("  ----+-----------+-------------------+---------+-------");

        for item in &self.inventory {
            let (stock_color, stock_text) = if item.stock > 0 {
                (color::GREEN, item.stock.to_string())
            } else {
                (color::RED, "RUPTURE".to_string())
            };

            println!("  {}{:3}{} | {} | {: <15} | {}{:.2} €{} | {}{}{}",
                color::CYAN, item.id, color::RESET,
                item.get_icon(),
                item.name,
                color::YELLOW, item.price, color::RESET,
                stock_color, stock_text, color::RESET
            );
        }
        println!();
    }

    fn display_controls(&self) {
        println!("  [PIECES] : 1 = 0.10, 2 = 0.20, 3 = 0.50, 4 = 1.00, 5 = 2.00");
        println!("  [ACTION] : Tapez l'ID du produit pour acheter");
        println!("  [SYSTEM] : 99 pour Rendre monnaie / 0 pour Quitter");
        print!("{}  Votre choix > {}", color::GRAY, color::RESET);
        io::stdout().flush().unwrap();
    }

    fn handle_user_action(&mut self) {
        let mut input_str = String::new();
        
        // Lecture sécurisée
        if io::stdin().read_line(&mut input_str).is_err() {
            return;
        }

        // Parsing sécurisé avec gestion d'erreur (Result)
        let input: i32 = match input_str.trim().parse() {
            Ok(num) => num,
            Err(_) => return, // Si ce n'est pas un nombre, on ignore
        };

        match input {
            0 => std::process::exit(0),
            1..=5 => {
                let coin_value = match input {
                    1 => 0.10, 2 => 0.20, 3 => 0.50, 4 => 1.00, 5 => 2.00, _ => 0.0
                };
                Hardware::coin_sound();
                self.current_credit += coin_value;
            },
            99 => {
                if self.current_credit > 0.0 {
                    Hardware::lcd_print("RENDU MONNAIE...", &format!("{:.2} EUR", self.current_credit));
                    thread::sleep(time::Duration::from_millis(2000));
                    self.current_credit = 0.0;
                }
            },
            _ => self.process_purchase(input),
        }
    }

    fn process_purchase(&mut self, product_id: i32) {
        let index_opt = self.inventory.iter().position(|p| p.id == product_id);

        if let Some(index) = index_opt {
            let item = &mut self.inventory[index];

            if item.stock <= 0 {
                Hardware::lcd_print("RUPTURE DE STOCK", "Choisissez un autre");
                thread::sleep(time::Duration::from_millis(1500));
                return;
            }

            if self.current_credit < item.price {
                let missing = item.price - self.current_credit;
                Hardware::lcd_print("CREDIT INSUFFISANT", &format!("Manque : {:.2} EUR", missing));
                thread::sleep(time::Duration::from_millis(2000));
                return;
            }

            // Transaction et Clone pour éviter les soucis de Borrowing
            let product_name = item.name.clone(); 
            
            Hardware::lcd_print("DISTRIBUTION...", &product_name);
            
            item.stock -= 1;
            self.current_credit -= item.price;

            Hardware::activate_motor(&product_name);

        } else {
            Hardware::lcd_print("ERREUR", "ID Inconnu !");
            thread::sleep(time::Duration::from_millis(1500));
        }
    }
}

fn main() {
    let mut machine = VendingMachine::new();
    machine.run();
}
