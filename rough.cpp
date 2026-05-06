/*#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

class Account {
    private:
        int accNo;
        string name;
        double balance;
        string pin;
        
    public:
        Account() : accNo(0), name(""), balance(0), pin("") {}
        
        Account(int a, string n, double b, string p)
            : accNo(a), name(n), balance(b), pin(p) {}
        
        int getAccNo() const { return accNo; }
        string getName() const { return name; }
        double getBalance() const { return balance; }
        string getPin() const { return pin; }
        
        void deposit(double amt) {
            if (amt > 0) {
                balance += amt;
                cout << "Successfully deposited: $" << fixed << setprecision(2) << amt << endl;
            } else {
                cout << "Invalid amount!" << endl;
            }
        }
        
        bool withdraw(double amt) {
            if (amt <= 0) {
                cout << "Invalid amount!" << endl;
                return false;
            }
            if (balance >= amt) {
                balance -= amt;
                cout << "Successfully withdrawn: $" << fixed << setprecision(2) << amt << endl;
                return true;
            } else {
                cout << "Insufficient balance!" << endl;
                return false;
            }
        }
        
        bool transfer(double amt, Account &recipient) {
            if (this->withdraw(amt)) {
                recipient.deposit(amt);
                cout << "Successfully transferred $" << fixed << setprecision(2) << amt 
                     << " to " << recipient.getName() << endl;
                return true;
            }
            return false;
        }
        
        void display() const {
            cout << "\n========== Account Details ==========" << endl;
            cout << "Account Number: " << accNo << endl;
            cout << "Account Holder: " << name << endl;
            cout << "Balance: $" << fixed << setprecision(2) << balance << endl;
            cout << "=====================================" << endl;
        }
        
        string toFileString() const {
            return to_string(accNo) + "|" + name + "|" + to_string(balance) + "|" + pin;
        }
        
        static Account fromFileString(const string &line) {
            int accNo;
            string name, balanceStr, pin;
            double balance;
            
            size_t pos = 0;
            size_t found = line.find('|');
            accNo = stoi(line.substr(pos, found - pos));
            
            pos = found + 1;
            found = line.find('|', pos);
            name = line.substr(pos, found - pos);
            
            pos = found + 1;
            found = line.find('|', pos);
            balance = stod(line.substr(pos, found - pos));
            
            pos = found + 1;
            pin = line.substr(pos);
            
            return Account(accNo, name, balance, pin);
        }
};

class Bank {
    private:
        vector<Account> accounts;
        const string filename = "accounts.dat";
        
        bool verifyPin(const Account &acc, const string &pin) const {
            return acc.getPin() == pin;
        }
        
        int findAccount(int accNo) {
            for (int i = 0; i < accounts.size(); i++) {
                if (accounts[i].getAccNo() == accNo) {
                    return i;
                }
            }
            return -1;
        }
        
    public:
        Bank() {
            loadFromFile();
        }
        
        void saveToFile() const {
            ofstream file(filename);
            if (!file.is_open()) {
                cout << "Error: Could not open file for writing!" << endl;
                return;
            }
            for (const auto &acc : accounts) {
                file << acc.toFileString() << endl;
            }
            file.close();
            cout << "Data saved successfully!" << endl;
        }
        
        void loadFromFile() {
            ifstream file(filename);
            if (!file.is_open()) {
                cout << "No existing accounts file. Starting fresh." << endl;
                return;
            }
            
            string line;
            while (getline(file, line)) {
                if (!line.empty()) {
                    accounts.push_back(Account::fromFileString(line));
                }
            }
            file.close();
            cout << "Accounts loaded from file." << endl;
        }
        
        void createAccount() {
            int accNo;
            string name, pin;
            double initialBalance;
            
            cout << "\n========== Create New Account ==========" << endl;
            cout << "Enter Account Number: ";
            cin >> accNo;
            
            if (findAccount(accNo) != -1) {
                cout << "Account already exists!" << endl;
                return;
            }
            
            cin.ignore();
            cout << "Enter Name: ";
            getline(cin, name);
            
            cout << "Enter PIN (4 digits): ";
            cin >> pin;
            
            if (pin.length() != 4) {
                cout << "PIN must be 4 digits!" << endl;
                return;
            }
            
            cout << "Enter Initial Balance: $";
            cin >> initialBalance;
            
            if (initialBalance < 0) {
                cout << "Balance cannot be negative!" << endl;
                return;
            }
            
            accounts.push_back(Account(accNo, name, initialBalance, pin));
            cout << "Account created successfully!" << endl;
            saveToFile();
        }
        
        void depositMoney() {
            int accNo;
            string pin;
            double amt;
            
            cout << "\n========== Deposit Money ==========" << endl;
            cout << "Enter Account Number: ";
            cin >> accNo;
            
            int idx = findAccount(accNo);
            if (idx == -1) {
                cout << "Account not found!" << endl;
                return;
            }
            
            cout << "Enter PIN: ";
            cin >> pin;
            
            if (!verifyPin(accounts[idx], pin)) {
                cout << "Invalid PIN!" << endl;
                return;
            }
            
            cout << "Enter Amount to Deposit: $";
            cin >> amt;
            
            accounts[idx].deposit(amt);
            saveToFile();
        }
        
        void withdrawMoney() {
            int accNo;
            string pin;
            double amt;
            
            cout << "\n========== Withdraw Money ==========" << endl;
            cout << "Enter Account Number: ";
            cin >> accNo;
            
            int idx = findAccount(accNo);
            if (idx == -1) {
                cout << "Account not found!" << endl;
                return;
            }
            
            cout << "Enter PIN: ";
            cin >> pin;
            
            if (!verifyPin(accounts[idx], pin)) {
                cout << "Invalid PIN!" << endl;
                return;
            }
            
            cout << "Enter Amount to Withdraw: $";
            cin >> amt;
            
            accounts[idx].withdraw(amt);
            saveToFile();
        }
        
        void transferMoney() {
            int fromAccNo, toAccNo;
            string pin;
            double amt;
            
            cout << "\n========== Transfer Money ==========" << endl;
            cout << "Enter Your Account Number: ";
            cin >> fromAccNo;
            
            int fromIdx = findAccount(fromAccNo);
            if (fromIdx == -1) {
                cout << "Account not found!" << endl;
                return;
            }
            
            cout << "Enter PIN: ";
            cin >> pin;
            
            if (!verifyPin(accounts[fromIdx], pin)) {
                cout << "Invalid PIN!" << endl;
                return;
            }
            
            cout << "Enter Recipient Account Number: ";
            cin >> toAccNo;
            
            int toIdx = findAccount(toAccNo);
            if (toIdx == -1) {
                cout << "Recipient account not found!" << endl;
                return;
            }
            
            cout << "Enter Amount to Transfer: $";
            cin >> amt;
            
            accounts[fromIdx].transfer(amt, accounts[toIdx]);
            saveToFile();
        }
        
        void checkBalance() {
            int accNo;
            string pin;
            
            cout << "\n========== Check Balance ==========" << endl;
            cout << "Enter Account Number: ";
            cin >> accNo;
            
            int idx = findAccount(accNo);
            if (idx == -1) {
                cout << "Account not found!" << endl;
                return;
            }
            
            cout << "Enter PIN: ";
            cin >> pin;
            
            if (!verifyPin(accounts[idx], pin)) {
                cout << "Invalid PIN!" << endl;
                return;
            }
            
            accounts[idx].display();
        }
        
        void displayAllAccounts() {
            cout << "\n========== All Accounts ==========" << endl;
            if (accounts.empty()) {
                cout << "No accounts in the system." << endl;
                return;
            }
            for (const auto &acc : accounts) {
                cout << "Account No: " << acc.getAccNo() 
                     << " | Name: " << acc.getName() 
                     << " | Balance: $" << fixed << setprecision(2) << acc.getBalance() << endl;
            }
            cout << "==================================" << endl;
        }
};

int main() {
    Bank bank;
    int choice;
    
    cout << "\n╔════════════════════════════════════╗" << endl;
    cout << "║   Welcome to Banking System       ║" << endl;
    cout << "╚════════════════════════════════════╝\n" << endl;
    
    while (true) {
        cout << "\n========== Main Menu ==========" << endl;
        cout << "1. Create New Account" << endl;
        cout << "2. Deposit Money" << endl;
        cout << "3. Withdraw Money" << endl;
        cout << "4. Transfer Money" << endl;
        cout << "5. Check Balance" << endl;
        cout << "6. View All Accounts (Admin)" << endl;
        cout << "7. Exit" << endl;
        cout << "==============================" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        
        switch (choice) {
            case 1:
                bank.createAccount();
                break;
            case 2:
                bank.depositMoney();
                break;
            case 3:
                bank.withdrawMoney();
                break;
            case 4:
                bank.transferMoney();
                break;
            case 5:
                bank.checkBalance();
                break;
            case 6:
                bank.displayAllAccounts();
                break;
            case 7:
                cout << "\nThank you for using our banking system. Goodbye!" << endl;
                return 0;
            default:
                cout << "Invalid choice! Please try again." << endl;
        }
    }
    
    return 0;
}*/
/*
 * ============================================================
 *        BANKING MANAGEMENT SYSTEM - C++ Implementation
 *        Features: File Handling, Exception Handling,
 *                  Deposits, Withdrawals, Transfers,
 *                  Account Management, Transaction History
 * ============================================================
 */
