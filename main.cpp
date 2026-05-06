#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <ctime>
#include <algorithm>
#include <stdexcept>
#include <cerrno>
#include <cstring>
using namespace std;

// ─────────────────────────────────────────────
//  CUSTOM EXCEPTION CLASSES: By Aditya Barnwal
// ─────────────────────────────────────────────

class BankingException : public runtime_error {
public:
    explicit BankingException(const string &msg)
        : runtime_error("[BankingException] " + msg) {}
};

class AccountNotFoundException : public BankingException {
public:
    explicit AccountNotFoundException(int accNo)
        : BankingException("Account not found: " + to_string(accNo)) {}
};

class InvalidPINException : public BankingException {
public:
    explicit InvalidPINException()
        : BankingException("Invalid PIN entered.") {}
};

class InsufficientFundsException : public BankingException {
public:
    explicit InsufficientFundsException(double balance, double requested)
        : BankingException(
              "Insufficient funds. Balance: ₹" + to_string(balance) +
              ", Requested: ₹" + to_string(requested)) {}
};

class InvalidAmountException : public BankingException {
public:
    explicit InvalidAmountException(const string &context = "")
        : BankingException("Invalid amount" + (context.empty() ? "." : " for " + context + ".")) {}
};

class DuplicateAccountException : public BankingException {
public:
    explicit DuplicateAccountException(int accNo)
        : BankingException("Account already exists: " + to_string(accNo)) {}
};

class FileIOException : public BankingException {
public:
    explicit FileIOException(const string &filename, const string &operation)
        : BankingException("File I/O error [" + operation + "] on '" + filename + "': " + strerror(errno)) {}
};

class InvalidInputException : public BankingException {
public:
    explicit InvalidInputException(const string &field)
        : BankingException("Invalid input for field: " + field) {}
};

class SameAccountTransferException : public BankingException {
public:
    explicit SameAccountTransferException()
        : BankingException("Cannot transfer to the same account.") {}
};

// ─────────────────────────────────────────────
//  TRANSACTION CLASS: By Krish Sharma
// ─────────────────────────────────────────────

class Transaction {
private:
    string type;
    double amount;
    double balanceAfter;
    string timestamp;
    string description;

public:
    Transaction() : amount(0), balanceAfter(0) {}

    Transaction(const string &t, double amt, double bal,
                const string &desc = "")
        : type(t), amount(amt), balanceAfter(bal), description(desc) {
        time_t now = time(nullptr);
        char buf[25];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        timestamp = string(buf);
    }

    // Getters
    string getType()        const { return type; }
    double getAmount()      const { return amount; }
    double getBalanceAfter()const { return balanceAfter; }
    string getTimestamp()   const { return timestamp; }
    string getDescription() const { return description; }

    void display() const {
        cout << left
             << setw(20) << timestamp
             << setw(12) << type
             << "₹" << setw(12) << fixed << setprecision(2) << amount
             << "Balance: ₹" << fixed << setprecision(2) << balanceAfter;
        if (!description.empty())
            cout << "  (" << description << ")";
        cout << endl;
    }

    // Serialize to string for file storage
    string serialize() const {
        return type + "~" + to_string(amount) + "~" +
               to_string(balanceAfter) + "~" + timestamp + "~" + description;
    }

    // Deserialize from string
    static Transaction deserialize(const string &line) {
        Transaction t;
        vector<string> parts;
        stringstream ss(line);
        string token;
        while (getline(ss, token, '~'))
            parts.push_back(token);

        if (parts.size() < 4)
            throw BankingException("Corrupted transaction record.");

        t.type         = parts[0];
        t.amount       = stod(parts[1]);
        t.balanceAfter = stod(parts[2]);
        t.timestamp    = parts[3];
        t.description  = (parts.size() > 4) ? parts[4] : "";
        return t;
    }
};

// ─────────────────────────────────────────────
//  ACCOUNT CLASS: By Raghav Bansal
// ─────────────────────────────────────────────

class Account {
private:
    int    accNo;
    string name;
    double balance;
    string pin;
    string accountType;   // "Savings" or "Current"
    vector<Transaction> history;

    void recordTransaction(const string &type, double amt,
                           const string &desc = "") {
        history.push_back(Transaction(type, amt, balance, desc));
        if (history.size() > 50)          // keep last 50 transactions
            history.erase(history.begin());
    }

public:
    Account()
        : accNo(0), name(""), balance(0.0), pin(""), accountType("Savings") {}

    Account(int a, const string &n, double b,
            const string &p, const string &type = "Savings")
        : accNo(a), name(n), balance(b), pin(p), accountType(type) {}

    // ── Getters ──────────────────────────────
    int    getAccNo()      const { return accNo; }
    string getName()       const { return name; }
    double getBalance()    const { return balance; }
    string getPin()        const { return pin; }
    string getAccountType()const { return accountType; }
    const vector<Transaction> &getHistory() const { return history; }

    // ── Deposit ──────────────────────────────
    void deposit(double amt) {
        if (amt <= 0)
            throw InvalidAmountException("deposit");
        balance += amt;
        recordTransaction("DEPOSIT", amt);
        cout << "\n  ✔ Deposited: ₹" << fixed << setprecision(2) << amt
             << "  |  New Balance: ₹" << balance << endl;
    }

    // ── Withdraw ─────────────────────────────
    void withdraw(double amt) {
        if (amt <= 0)
            throw InvalidAmountException("withdrawal");
        if (balance < amt)
            throw InsufficientFundsException(balance, amt);
        balance -= amt;
        recordTransaction("WITHDRAWAL", amt);
        cout << "\n  ✔ Withdrawn: ₹" << fixed << setprecision(2) << amt
             << "  |  Remaining Balance: ₹" << balance << endl;
    }

    // ── Transfer (debit side) ─────────────────
    void debit(double amt, const string &recipientName, int recipientAccNo) {
        if (amt <= 0)
            throw InvalidAmountException("transfer");
        if (balance < amt)
            throw InsufficientFundsException(balance, amt);
        balance -= amt;
        recordTransaction("TRANSFER-OUT", amt,
                          "To: " + recipientName +
                          " [Acc# " + to_string(recipientAccNo) + "]");
    }

    // ── Transfer (credit side) ────────────────
    void credit(double amt, const string &senderName, int senderAccNo) {
        if (amt <= 0)
            throw InvalidAmountException("transfer credit");
        balance += amt;
        recordTransaction("TRANSFER-IN", amt,
                          "From: " + senderName +
                          " [Acc# " + to_string(senderAccNo) + "]");
    }

    // ── Update PIN ────────────────────────────
    void updatePin(const string &newPin) {
        if (newPin.length() != 4)
            throw InvalidInputException("PIN (must be exactly 4 digits)");
        for (char c : newPin)
            if (!isdigit(c))
                throw InvalidInputException("PIN (digits only)");
        pin = newPin;
        cout << "  ✔ PIN updated successfully." << endl;
    }

    // ── Display account details ───────────────
    void display() const {
        cout << "\n  ╔══════════════════════════════════╗" << endl;
        cout << "  ║       ACCOUNT DETAILS            ║" << endl;
        cout << "  ╠══════════════════════════════════╣" << endl;
        cout << "  ║  Account No  : " << left << setw(18) << accNo    << "║" << endl;
        cout << "  ║  Holder Name : " << left << setw(18) << name     << "║" << endl;
        cout << "  ║  Type        : " << left << setw(18) << accountType << "║" << endl;
        cout << "  ║  Balance     : ₹"
             << left << setw(17) << (to_string((int)balance) + "." +
                                     to_string((int)((balance - (int)balance) * 100)))
             << "║" << endl;
        cout << "  ╚══════════════════════════════════╝" << endl;
    }

    // ── Display last N transactions ───────────
    void displayHistory(int n = 10) const {
        cout << "\n  ──── Transaction History (last " << n << ") ────" << endl;
        if (history.empty()) {
            cout << "  No transactions found.\n";
            return;
        }
        int start = max(0, (int)history.size() - n);
        cout << left << setw(20) << "Timestamp"
             << setw(14) << "Type"
             << setw(14) << "Amount"
             << "Balance After" << endl;
        cout << string(65, '-') << endl;
        for (int i = start; i < (int)history.size(); i++)
            history[i].display();
        cout << string(65, '-') << endl;
    }

    // ── Serialization ─────────────────────────
    string toFileString() const {
        string s = to_string(accNo) + "|" + name + "|" +
                   to_string(balance) + "|" + pin + "|" + accountType + "|";
        for (size_t i = 0; i < history.size(); i++) {
            s += history[i].serialize();
            if (i + 1 < history.size()) s += "§";
        }
        return s;
    }

    static Account fromFileString(const string &line) {
        if (line.empty())
            throw BankingException("Empty line in accounts file.");

        vector<string> parts;
        stringstream ss(line);
        string token;
        while (getline(ss, token, '|'))
            parts.push_back(token);

        if (parts.size() < 5)
            throw BankingException("Corrupted account record: " + line);

        int    accNo   = stoi(parts[0]);
        string name    = parts[1];
        double balance = stod(parts[2]);
        string pin     = parts[3];
        string type    = parts[4];

        Account acc(accNo, name, balance, pin, type);

        // Parse transaction history if present
        if (parts.size() > 5 && !parts[5].empty()) {
            stringstream ths(parts[5]);
            string txn;
            while (getline(ths, txn, (char)0xC2)) {}   // dummy; use find instead
            // Manual split on § (multi-byte in some encodings; use sentinel)
            string raw = parts[5];
            size_t pos = 0, found;
            while ((found = raw.find("§", pos)) != string::npos) {
                string rec = raw.substr(pos, found - pos);
                if (!rec.empty()) {
                    try { acc.history.push_back(Transaction::deserialize(rec)); }
                    catch (...) {}
                }
                pos = found + 2; // § is 2 bytes in UTF-8
            }
            if (pos < raw.size()) {
                try { acc.history.push_back(Transaction::deserialize(raw.substr(pos))); }
                catch (...) {}
            }
        }
        return acc;
    }
};

// ─────────────────────────────────────────────
//  INPUT UTILITIES: By Aditya Shekhar Mishra
// ─────────────────────────────────────────────

namespace InputUtil {

    void clearInputBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int readInt(const string &prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) break;
            cout << "  ✘ Please enter a valid integer.\n";
            clearInputBuffer();
        }
        clearInputBuffer();
        return val;
    }

    double readDouble(const string &prompt) {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val && val >= 0) break;
            cout << "  ✘ Please enter a valid non-negative number.\n";
            clearInputBuffer();
        }
        clearInputBuffer();
        return val;
    }

    string readLine(const string &prompt) {
        string val;
        cout << prompt;
        getline(cin, val);
        return val;
    }

    string readWord(const string &prompt) {
        string val;
        cout << prompt;
        cin >> val;
        clearInputBuffer();
        return val;
    }

    string readPin(const string &prompt = "  Enter PIN: ") {
        string pin = readWord(prompt);
        if (pin.length() != 4)
            throw InvalidInputException("PIN (must be 4 digits)");
        for (char c : pin)
            if (!isdigit(c))
                throw InvalidInputException("PIN (must contain only digits)");
        return pin;
    }
}

// ─────────────────────────────────────────────
//  BANK CLASS: By Aditya Shekhar Mishra
// ─────────────────────────────────────────────

class Bank {
private:
    vector<Account> accounts;
    const string ACCOUNTS_FILE = "accounts.dat";
    const string BACKUP_FILE   = "accounts_backup.dat";
    const string LOG_FILE      = "bank_log.txt";

    // ── Internal helpers ─────────────────────

    int findAccountIndex(int accNo) const {
        for (int i = 0; i < (int)accounts.size(); i++)
            if (accounts[i].getAccNo() == accNo)
                return i;
        return -1;
    }

    Account &getAccountOrThrow(int accNo) {
        int idx = findAccountIndex(accNo);
        if (idx == -1)
            throw AccountNotFoundException(accNo);
        return accounts[idx];
    }

    void verifyPin(const Account &acc, const string &pin) const {
        if (acc.getPin() != pin)
            throw InvalidPINException();
    }

    void writeLog(const string &entry) const {
        ofstream log(LOG_FILE, ios::app);
        if (!log.is_open()) return;
        time_t now = time(nullptr);
        char buf[25];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        log << "[" << buf << "] " << entry << "\n";
        log.close();
    }

    void printDivider(char c = '=', int width = 45) const {
        cout << "  " << string(width, c) << endl;
    }

    void printHeader(const string &title) const {
        printDivider();
        int padding = (41 - (int)title.size()) / 2;
        cout << "  " << string(padding, ' ') << title << endl;
        printDivider();
    }

public:
    // ── Constructor ───────────────────────────
    Bank() {
        cout << "\n  Loading account data..." << endl;
        loadFromFile();
    }

    // ── Destructor: auto-save ─────────────────
    ~Bank() {
        try { saveToFile(); }
        catch (...) {}
    }

    // ── File: Save ────────────────────────────
    void saveToFile() const {
        // First write a backup of the existing file
        {
            ifstream src(ACCOUNTS_FILE, ios::binary);
            if (src.is_open()) {
                ofstream dst(BACKUP_FILE, ios::binary);
                dst << src.rdbuf();
            }
        }

        ofstream file(ACCOUNTS_FILE);
        if (!file.is_open())
            throw FileIOException(ACCOUNTS_FILE, "write");

        for (const auto &acc : accounts)
            file << acc.toFileString() << "\n";

        file.close();
        if (file.fail())
            throw FileIOException(ACCOUNTS_FILE, "flush");

        writeLog("Accounts saved (" + to_string(accounts.size()) + " records).");
    }

    // ── File: Load ────────────────────────────
    void loadFromFile() {
        ifstream file(ACCOUNTS_FILE);
        if (!file.is_open()) {
            cout << "  No accounts file found. Starting with empty system.\n";
            writeLog("No accounts file; started fresh.");
            return;
        }

        int loaded = 0, skipped = 0;
        string line;
        int lineNo = 0;
        while (getline(file, line)) {
            lineNo++;
            if (line.empty()) continue;
            try {
                accounts.push_back(Account::fromFileString(line));
                loaded++;
            } catch (const exception &e) {
                cerr << "  ⚠ Skipping corrupted record at line "
                     << lineNo << ": " << e.what() << "\n";
                skipped++;
            }
        }
        file.close();
        cout << "  ✔ Loaded " << loaded << " account(s)";
        if (skipped) cout << ", skipped " << skipped << " corrupted record(s)";
        cout << ".\n";
        writeLog("Loaded " + to_string(loaded) + " accounts.");
    }

    // ── Feature 1: Create Account ─────────────
    void createAccount() {
        printHeader("CREATE NEW ACCOUNT");
        try {
            int accNo = InputUtil::readInt("  Enter Account Number   : ");
            if (findAccountIndex(accNo) != -1)
                throw DuplicateAccountException(accNo);

            string name = InputUtil::readLine("  Enter Full Name        : ");
            if (name.empty() || name.length() < 2)
                throw InvalidInputException("Name (must be at least 2 chars)");

            cout << "  Account Type (1=Savings, 2=Current): ";
            int typeChoice;
            cin >> typeChoice;
            InputUtil::clearInputBuffer();
            string accType = (typeChoice == 2) ? "Current" : "Savings";

            string pin = InputUtil::readPin("  Set 4-digit PIN        : ");
            string confirmPin = InputUtil::readPin("  Confirm PIN            : ");
            if (pin != confirmPin)
                throw InvalidInputException("PIN (confirmation mismatch)");

            double initialBalance = InputUtil::readDouble("  Initial Deposit (₹)    : ");

            accounts.push_back(Account(accNo, name, initialBalance, pin, accType));
            cout << "\n  ✔ Account #" << accNo << " created for " << name << ".\n";
            writeLog("Account created: #" + to_string(accNo) + " (" + name + ")");
            saveToFile();
        }
        catch (const DuplicateAccountException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidInputException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 2: Deposit ────────────────────
    void depositMoney() {
        printHeader("DEPOSIT MONEY");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            Account &acc = getAccountOrThrow(accNo);
            verifyPin(acc, InputUtil::readPin());

            double amt = InputUtil::readDouble("  Amount to Deposit (₹): ");
            acc.deposit(amt);

            writeLog("Deposit: ₹" + to_string(amt) + " -> Acc#" + to_string(accNo));
            saveToFile();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidAmountException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 3: Withdraw ───────────────────
    void withdrawMoney() {
        printHeader("WITHDRAW MONEY");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            Account &acc = getAccountOrThrow(accNo);
            verifyPin(acc, InputUtil::readPin());

            cout << "  Current Balance: ₹" << fixed << setprecision(2)
                 << acc.getBalance() << "\n";
            double amt = InputUtil::readDouble("  Amount to Withdraw (₹): ");
            acc.withdraw(amt);

            writeLog("Withdrawal: ₹" + to_string(amt) + " <- Acc#" + to_string(accNo));
            saveToFile();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InsufficientFundsException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidAmountException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 4: Transfer ───────────────────
    void transferMoney() {
        printHeader("TRANSFER MONEY");
        try {
            int fromAccNo = InputUtil::readInt("  Your Account Number        : ");
            Account &from = getAccountOrThrow(fromAccNo);
            verifyPin(from, InputUtil::readPin("  Your PIN                   : "));

            int toAccNo = InputUtil::readInt("  Recipient Account Number   : ");
            if (fromAccNo == toAccNo)
                throw SameAccountTransferException();

            Account &to = getAccountOrThrow(toAccNo);

            cout << "  Recipient: " << to.getName() << "\n";
            cout << "  Current Balance: ₹" << fixed << setprecision(2)
                 << from.getBalance() << "\n";
            double amt = InputUtil::readDouble("  Amount to Transfer (₹)     : ");

            // Confirm
            cout << "  Transfer ₹" << fixed << setprecision(2) << amt
                 << " to " << to.getName() << "? (1=Yes / 0=No): ";
            int confirm;
            cin >> confirm;
            InputUtil::clearInputBuffer();
            if (confirm != 1) {
                cout << "  Transfer cancelled.\n";
                return;
            }

            from.debit(amt, to.getName(), toAccNo);
            to.credit(amt, from.getName(), fromAccNo);

            cout << "\n  ✔ Transferred ₹" << fixed << setprecision(2) << amt
                 << " from Acc#" << fromAccNo
                 << " to Acc#" << toAccNo << " (" << to.getName() << ").\n";

            writeLog("Transfer: ₹" + to_string(amt) +
                     " Acc#" + to_string(fromAccNo) +
                     " -> Acc#" + to_string(toAccNo));
            saveToFile();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InsufficientFundsException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const SameAccountTransferException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidAmountException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 5: Check Balance ──────────────
    void checkBalance() {
        printHeader("CHECK BALANCE");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            const Account &acc = getAccountOrThrow(accNo);
            verifyPin(acc, InputUtil::readPin());
            acc.display();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 6: Transaction History ────────
    void viewTransactionHistory() {
        printHeader("TRANSACTION HISTORY");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            const Account &acc = getAccountOrThrow(accNo);
            verifyPin(acc, InputUtil::readPin());
            int n = InputUtil::readInt("  How many recent transactions? : ");
            if (n <= 0) throw InvalidInputException("number of transactions");
            acc.displayHistory(n);
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidInputException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 7: Change PIN ─────────────────
    void changePin() {
        printHeader("CHANGE PIN");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            Account &acc = getAccountOrThrow(accNo);
            verifyPin(acc, InputUtil::readPin("  Current PIN    : "));

            string newPin     = InputUtil::readPin("  New PIN        : ");
            string confirmPin = InputUtil::readPin("  Confirm New PIN: ");
            if (newPin != confirmPin)
                throw InvalidInputException("PIN (confirmation mismatch)");

            acc.updatePin(newPin);
            writeLog("PIN changed for Acc#" + to_string(accNo));
            saveToFile();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidInputException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 8: Delete Account ─────────────
    void deleteAccount() {
        printHeader("DELETE ACCOUNT");
        try {
            int accNo = InputUtil::readInt("  Account Number : ");
            int idx = findAccountIndex(accNo);
            if (idx == -1) throw AccountNotFoundException(accNo);

            verifyPin(accounts[idx], InputUtil::readPin("  Enter PIN to confirm: "));

            if (accounts[idx].getBalance() > 0) {
                cout << "  ⚠ Account has ₹"
                     << fixed << setprecision(2) << accounts[idx].getBalance()
                     << " remaining. Proceed? (1=Yes / 0=No): ";
                int confirm;
                cin >> confirm;
                InputUtil::clearInputBuffer();
                if (confirm != 1) {
                    cout << "  Deletion cancelled.\n";
                    return;
                }
            }

            string name = accounts[idx].getName();
            accounts.erase(accounts.begin() + idx);
            cout << "  ✔ Account #" << accNo << " (" << name << ") deleted.\n";
            writeLog("Account deleted: #" + to_string(accNo) + " (" + name + ")");
            saveToFile();
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidPINException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 9: Admin – View All Accounts ──
    void displayAllAccounts() const {
        printHeader("ALL ACCOUNTS  [ADMIN]");
        if (accounts.empty()) {
            cout << "  No accounts in the system.\n";
            return;
        }
        cout << left
             << setw(12) << "  Acc#"
             << setw(22) << "Name"
             << setw(12) << "Type"
             << "Balance" << "\n";
        printDivider('-');
        double total = 0;
        for (const auto &acc : accounts) {
            cout << "  " << left
                 << setw(12) << acc.getAccNo()
                 << setw(22) << acc.getName()
                 << setw(12) << acc.getAccountType()
                 << "₹" << fixed << setprecision(2) << acc.getBalance() << "\n";
            total += acc.getBalance();
        }
        printDivider('-');
        cout << "  Total Accounts : " << accounts.size() << "\n";
        cout << "  Total Deposits : ₹" << fixed << setprecision(2) << total << "\n";
        printDivider();
    }

    // ── Feature 10: Search Account ────────────
    void searchAccount() const {
        printHeader("SEARCH ACCOUNT");
        try {
            cout << "  Search by: (1) Account Number  (2) Name\n";
            int choice = InputUtil::readInt("  Choice: ");

            if (choice == 1) {
                int accNo = InputUtil::readInt("  Account Number: ");
                int idx = findAccountIndex(accNo);
                if (idx == -1) throw AccountNotFoundException(accNo);
                accounts[idx].display();
            } else if (choice == 2) {
                string query = InputUtil::readLine("  Enter name (partial match): ");
                transform(query.begin(), query.end(), query.begin(), ::tolower);
                bool found = false;
                for (const auto &acc : accounts) {
                    string lower = acc.getName();
                    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
                    if (lower.find(query) != string::npos) {
                        acc.display();
                        found = true;
                    }
                }
                if (!found) cout << "  No accounts matching \"" << query << "\".\n";
            } else {
                throw InvalidInputException("search choice");
            }
        }
        catch (const AccountNotFoundException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const InvalidInputException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }

    // ── Feature 11: Manual Save ───────────────
    void manualSave() {
        printHeader("SAVE DATA");
        try {
            saveToFile();
            cout << "  ✔ All data saved to '" << ACCOUNTS_FILE << "'.\n";
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error during save: " << e.what() << "\n";
        }
    }

    // ── Feature 12: Restore from Backup ───────
    void restoreFromBackup() {
        printHeader("RESTORE FROM BACKUP");
        try {
            ifstream backup(BACKUP_FILE);
            if (!backup.is_open())
                throw FileIOException(BACKUP_FILE, "read");
            backup.close();

            cout << "  ⚠ This will overwrite current data with the backup. Continue? (1=Yes): ";
            int confirm;
            cin >> confirm;
            InputUtil::clearInputBuffer();
            if (confirm != 1) { cout << "  Restore cancelled.\n"; return; }

            accounts.clear();
            ifstream file(BACKUP_FILE);
            string line;
            int loaded = 0;
            while (getline(file, line)) {
                if (!line.empty()) {
                    try {
                        accounts.push_back(Account::fromFileString(line));
                        loaded++;
                    } catch (...) {}
                }
            }
            file.close();
            cout << "  ✔ Restored " << loaded << " account(s) from backup.\n";
            writeLog("Restored from backup (" + to_string(loaded) + " records).");
            saveToFile();
        }
        catch (const FileIOException &e) {
            cerr << "\n  ✘ " << e.what() << "\n";
        }
        catch (const exception &e) {
            cerr << "\n  ✘ Unexpected error: " << e.what() << "\n";
        }
    }
};

// ─────────────────────────────────────────────
//  MENU HELPERS: By Aditya Barnwal
// ─────────────────────────────────────────────

void printWelcome() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════╗\n";
    cout << "  ║                                          ║\n";
    cout << "  ║     ██████╗  █████╗ ███╗  ██╗██╗  ██╗    ║\n";
    cout << "  ║     ██╔══██╗██╔══██╗████╗ ██║██║ ██╔╝    ║\n";
    cout << "  ║     ██████╦╝███████║██╔██╗██║█████╔╝     ║\n";
    cout << "  ║     ██╔══██╗██╔══██║██║╚████║██╔═██╗     ║\n";
    cout << "  ║     ██████╔╝██║  ██║██║ ╚███║██║  ██╗    ║\n";
    cout << "  ║     ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚══╝╚═╝  ╚═╝    ║\n";
    cout << "  ║        Banking Management System         ║\n";
    cout << "  ║                                          ║\n";
    cout << "  ╚══════════════════════════════════════════╝\n\n";
}

void printMenu() {
    cout << "\n  ╔══════════════════════════════════╗\n";
    cout << "  ║           MAIN  MENU             ║\n";
    cout << "  ╠══════════════════════════════════╣\n";
    cout << "  ║  1.  Create New Account          ║\n";
    cout << "  ║  2.  Deposit Money               ║\n";
    cout << "  ║  3.  Withdraw Money              ║\n";
    cout << "  ║  4.  Transfer Money              ║\n";
    cout << "  ║  5.  Check Balance               ║\n";
    cout << "  ║  6.  Transaction History         ║\n";
    cout << "  ║  7.  Change PIN                  ║\n";
    cout << "  ║  8.  Delete Account              ║\n";
    cout << "  ║  9.  Search Account              ║\n";
    cout << "  ║  10. View All Accounts [Admin]   ║\n";
    cout << "  ║  11. Save Data                   ║\n";
    cout << "  ║  12. Restore from Backup         ║\n";
    cout << "  ║  0.  Exit                        ║\n";
    cout << "  ╚══════════════════════════════════╝\n";
    cout << "  Choice: ";
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────

int main() {
    printWelcome();

    Bank bank;
    int choice;

    while (true) {
        printMenu();

        if (!(cin >> choice)) {
            InputUtil::clearInputBuffer();
            cerr << "  ✘ Invalid input. Please enter a number.\n";
            continue;
        }
        InputUtil::clearInputBuffer();

        try {
            switch (choice) {
                case  1: bank.createAccount();          break;
                case  2: bank.depositMoney();           break;
                case  3: bank.withdrawMoney();          break;
                case  4: bank.transferMoney();          break;
                case  5: bank.checkBalance();           break;
                case  6: bank.viewTransactionHistory(); break;
                case  7: bank.changePin();              break;
                case  8: bank.deleteAccount();          break;
                case  9: bank.searchAccount();          break;
                case 10: bank.displayAllAccounts();     break;
                case 11: bank.manualSave();             break;
                case 12: bank.restoreFromBackup();      break;
                case  0:
                    cout << "\n  Thank you for using the Banking System. Goodbye!\n\n";
                    return 0;
                default:
                    cout << "  ✘ Invalid choice. Please select 0–12.\n";
            }
        }
        catch (const exception &e) {
            cerr << "\n  [Unhandled] " << e.what() << "\n";
        }
        catch (...) {
            cerr << "\n  [Unknown error] Something went wrong.\n";
        }
    }

    return 0;
}