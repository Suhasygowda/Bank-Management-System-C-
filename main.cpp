/*
 ============================================================
   SMART BANKING MANAGEMENT SYSTEM
   A console-based banking app in C++ (Single File)
   Features:
     - Account creation (Savings / Current)
     - PIN-based login
     - Deposit, Withdraw, Transfer
     - Transaction history
     - Interest calculation (Savings accounts)
     - Admin panel (view all accounts, delete accounts)
     - Data saved to files (accounts.dat, transactions.dat)
 ============================================================
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <limits>

using namespace std;

// ─────────────────────────────────────────────
//  CONSTANTS
// ─────────────────────────────────────────────
const string ACCOUNTS_FILE    = "accounts.dat";
const string TRANSACTIONS_FILE = "transactions.dat";
const string ADMIN_PIN        = "admin123";   // Admin password
const double SAVINGS_INTEREST = 0.04;         // 4% annual interest rate

// ─────────────────────────────────────────────
//  HELPER: Get current timestamp as string
// ─────────────────────────────────────────────
string getCurrentTime() {
    time_t now = time(0);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// ─────────────────────────────────────────────
//  HELPER: Generate a unique Account Number
//  Format: ACC + 6-digit number (e.g. ACC100001)
// ─────────────────────────────────────────────
string generateAccountNumber(int id) {
    stringstream ss;
    ss << "ACC" << setw(6) << setfill('0') << id;
    return ss.str();
}

// ─────────────────────────────────────────────
//  STRUCT: Transaction
//  Holds one transaction entry
// ─────────────────────────────────────────────
struct Transaction {
    string accountNumber;  // Which account
    string type;           // DEPOSIT / WITHDRAW / TRANSFER_IN / TRANSFER_OUT / INTEREST
    double amount;
    double balanceAfter;
    string timestamp;
    string note;           // Extra info (e.g. transfer to whom)
};

// ─────────────────────────────────────────────
//  STRUCT: Account
//  Holds all info for a bank account
// ─────────────────────────────────────────────
struct Account {
    string accountNumber;
    string holderName;
    string pin;            // Stored as plain string (in real apps: hashed)
    string accountType;    // "Savings" or "Current"
    double balance;
    bool   isActive;       // false = account deleted by admin
    string createdAt;
};

// ─────────────────────────────────────────────
//  FILE I/O: Save all accounts to file
// ─────────────────────────────────────────────
void saveAccounts(const vector<Account>& accounts) {
    ofstream file(ACCOUNTS_FILE);
    for (const auto& acc : accounts) {
        // Format: accountNumber|holderName|pin|type|balance|isActive|createdAt
        file << acc.accountNumber << "|"
             << acc.holderName    << "|"
             << acc.pin           << "|"
             << acc.accountType   << "|"
             << fixed << setprecision(2) << acc.balance << "|"
             << acc.isActive      << "|"
             << acc.createdAt     << "\n";
    }
}

// ─────────────────────────────────────────────
//  FILE I/O: Load accounts from file
// ─────────────────────────────────────────────
vector<Account> loadAccounts() {
    vector<Account> accounts;
    ifstream file(ACCOUNTS_FILE);
    if (!file.is_open()) return accounts;  // No file yet, return empty

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Account acc;
        string isActiveStr;

        getline(ss, acc.accountNumber, '|');
        getline(ss, acc.holderName,    '|');
        getline(ss, acc.pin,           '|');
        getline(ss, acc.accountType,   '|');
        ss >> acc.balance;
        ss.ignore();
        getline(ss, isActiveStr, '|');
        getline(ss, acc.createdAt);

        acc.isActive = (isActiveStr == "1");
        accounts.push_back(acc);
    }
    return accounts;
}

// ─────────────────────────────────────────────
//  FILE I/O: Save one transaction (append mode)
// ─────────────────────────────────────────────
void saveTransaction(const Transaction& txn) {
    ofstream file(TRANSACTIONS_FILE, ios::app);
    file << txn.accountNumber << "|"
         << txn.type          << "|"
         << fixed << setprecision(2) << txn.amount << "|"
         << txn.balanceAfter  << "|"
         << txn.timestamp     << "|"
         << txn.note          << "\n";
}

// ─────────────────────────────────────────────
//  FILE I/O: Load transactions for one account
// ─────────────────────────────────────────────
vector<Transaction> loadTransactions(const string& accountNumber) {
    vector<Transaction> result;
    ifstream file(TRANSACTIONS_FILE);
    if (!file.is_open()) return result;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        Transaction txn;

        getline(ss, txn.accountNumber, '|');
        getline(ss, txn.type,          '|');
        ss >> txn.amount;
        ss.ignore();
        ss >> txn.balanceAfter;
        ss.ignore();
        getline(ss, txn.timestamp, '|');
        getline(ss, txn.note);

        if (txn.accountNumber == accountNumber) {
            result.push_back(txn);
        }
    }
    return result;
}

// ─────────────────────────────────────────────
//  HELPER: Find account index by account number
//  Returns -1 if not found
// ─────────────────────────────────────────────
int findAccount(const vector<Account>& accounts, const string& accNum) {
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (accounts[i].accountNumber == accNum)
            return i;
    }
    return -1;
}

// ─────────────────────────────────────────────
//  HELPER: Print a divider line
// ─────────────────────────────────────────────
void printLine(char c = '-', int len = 55) {
    cout << string(len, c) << "\n";
}

// ─────────────────────────────────────────────
//  HELPER: Clear invalid cin input
// ─────────────────────────────────────────────
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ─────────────────────────────────────────────
//  HELPER: Read a double safely
// ─────────────────────────────────────────────
double readDouble(const string& prompt) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val && val > 0) {
            clearInput();
            return val;
        }
        cout << "  [!] Invalid amount. Please enter a positive number.\n";
        clearInput();
    }
}

// ─────────────────────────────────────────────
//  FEATURE: Create a new account
// ─────────────────────────────────────────────
void createAccount(vector<Account>& accounts) {
    printLine('=');
    cout << "           CREATE NEW ACCOUNT\n";
    printLine('=');

    Account acc;

    cout << "  Enter Full Name    : ";
    clearInput();
    getline(cin, acc.holderName);
    if (acc.holderName.empty()) {
        cout << "  [!] Name cannot be empty.\n";
        return;
    }

    cout << "  Account Type\n";
    cout << "    1. Savings (earns " << (SAVINGS_INTEREST * 100) << "% interest)\n";
    cout << "    2. Current\n";
    cout << "  Choice: ";
    int typeChoice;
    cin >> typeChoice;
    acc.accountType = (typeChoice == 1) ? "Savings" : "Current";

    cout << "  Set a 4-digit PIN  : ";
    cin >> acc.pin;
    if (acc.pin.length() != 4) {
        cout << "  [!] PIN must be exactly 4 digits.\n";
        clearInput();
        return;
    }

    double initialDeposit = readDouble("  Initial Deposit (min $100): ");
    if (initialDeposit < 100) {
        cout << "  [!] Minimum initial deposit is $100.\n";
        return;
    }

    // Assign account number based on how many accounts exist
    int newId = (int)accounts.size() + 100001;
    acc.accountNumber = generateAccountNumber(newId);
    acc.balance       = initialDeposit;
    acc.isActive      = true;
    acc.createdAt     = getCurrentTime();

    accounts.push_back(acc);
    saveAccounts(accounts);

    // Record the initial deposit as a transaction
    Transaction txn;
    txn.accountNumber = acc.accountNumber;
    txn.type          = "DEPOSIT";
    txn.amount        = initialDeposit;
    txn.balanceAfter  = initialDeposit;
    txn.timestamp     = acc.createdAt;
    txn.note          = "Initial deposit on account creation";
    saveTransaction(txn);

    printLine();
    cout << "  [✓] Account created successfully!\n";
    cout << "  Account Number : " << acc.accountNumber << "\n";
    cout << "  Account Type   : " << acc.accountType   << "\n";
    cout << "  Balance        : $" << fixed << setprecision(2) << acc.balance << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Show account details (dashboard)
// ─────────────────────────────────────────────
void showDashboard(const Account& acc) {
    printLine('=');
    cout << "         ACCOUNT DASHBOARD\n";
    printLine('=');
    cout << "  Name           : " << acc.holderName    << "\n";
    cout << "  Account Number : " << acc.accountNumber << "\n";
    cout << "  Account Type   : " << acc.accountType   << "\n";
    cout << "  Balance        : $" << fixed << setprecision(2) << acc.balance << "\n";
    cout << "  Member Since   : " << acc.createdAt    << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Deposit money
// ─────────────────────────────────────────────
void deposit(vector<Account>& accounts, int idx) {
    printLine('=');
    cout << "               DEPOSIT\n";
    printLine('=');
    cout << "  Current Balance: $" << fixed << setprecision(2) << accounts[idx].balance << "\n";

    double amount = readDouble("  Enter deposit amount: $");

    accounts[idx].balance += amount;
    saveAccounts(accounts);

    Transaction txn;
    txn.accountNumber = accounts[idx].accountNumber;
    txn.type          = "DEPOSIT";
    txn.amount        = amount;
    txn.balanceAfter  = accounts[idx].balance;
    txn.timestamp     = getCurrentTime();
    txn.note          = "Cash deposit";
    saveTransaction(txn);

    cout << "  [✓] Deposited $" << fixed << setprecision(2) << amount << " successfully.\n";
    cout << "  New Balance    : $" << accounts[idx].balance << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Withdraw money
// ─────────────────────────────────────────────
void withdraw(vector<Account>& accounts, int idx) {
    printLine('=');
    cout << "              WITHDRAWAL\n";
    printLine('=');
    cout << "  Current Balance: $" << fixed << setprecision(2) << accounts[idx].balance << "\n";

    double amount = readDouble("  Enter withdrawal amount: $");

    // Keep minimum $100 balance
    if (amount > accounts[idx].balance - 100) {
        cout << "  [!] Insufficient funds. (Minimum balance of $100 must be maintained)\n";
        return;
    }

    accounts[idx].balance -= amount;
    saveAccounts(accounts);

    Transaction txn;
    txn.accountNumber = accounts[idx].accountNumber;
    txn.type          = "WITHDRAW";
    txn.amount        = amount;
    txn.balanceAfter  = accounts[idx].balance;
    txn.timestamp     = getCurrentTime();
    txn.note          = "Cash withdrawal";
    saveTransaction(txn);

    cout << "  [✓] Withdrew $" << fixed << setprecision(2) << amount << " successfully.\n";
    cout << "  Remaining Balance: $" << accounts[idx].balance << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Transfer money to another account
// ─────────────────────────────────────────────
void transfer(vector<Account>& accounts, int senderIdx) {
    printLine('=');
    cout << "             FUND TRANSFER\n";
    printLine('=');
    cout << "  Your Balance: $" << fixed << setprecision(2) << accounts[senderIdx].balance << "\n";

    cout << "  Enter recipient account number: ";
    string recipientAccNum;
    cin >> recipientAccNum;

    int recipientIdx = findAccount(accounts, recipientAccNum);

    if (recipientIdx == -1) {
        cout << "  [!] Recipient account not found.\n";
        return;
    }
    if (recipientIdx == senderIdx) {
        cout << "  [!] Cannot transfer to your own account.\n";
        return;
    }
    if (!accounts[recipientIdx].isActive) {
        cout << "  [!] Recipient account is inactive.\n";
        return;
    }

    cout << "  Recipient Name : " << accounts[recipientIdx].holderName << "\n";

    double amount = readDouble("  Enter transfer amount: $");

    if (amount > accounts[senderIdx].balance - 100) {
        cout << "  [!] Insufficient funds. (Minimum balance of $100 must be maintained)\n";
        return;
    }

    string now = getCurrentTime();

    // Deduct from sender
    accounts[senderIdx].balance -= amount;
    Transaction txnOut;
    txnOut.accountNumber = accounts[senderIdx].accountNumber;
    txnOut.type          = "TRANSFER_OUT";
    txnOut.amount        = amount;
    txnOut.balanceAfter  = accounts[senderIdx].balance;
    txnOut.timestamp     = now;
    txnOut.note          = "Transfer to " + accounts[recipientIdx].accountNumber;
    saveTransaction(txnOut);

    // Add to recipient
    accounts[recipientIdx].balance += amount;
    Transaction txnIn;
    txnIn.accountNumber = accounts[recipientIdx].accountNumber;
    txnIn.type          = "TRANSFER_IN";
    txnIn.amount        = amount;
    txnIn.balanceAfter  = accounts[recipientIdx].balance;
    txnIn.timestamp     = now;
    txnIn.note          = "Transfer from " + accounts[senderIdx].accountNumber;
    saveTransaction(txnIn);

    saveAccounts(accounts);

    cout << "  [✓] Transferred $" << fixed << setprecision(2) << amount
         << " to " << accounts[recipientIdx].holderName << "\n";
    cout << "  Your Remaining Balance: $" << accounts[senderIdx].balance << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Apply monthly interest (Savings only)
//  Interest = balance * (annual rate / 12)
// ─────────────────────────────────────────────
void applyInterest(vector<Account>& accounts, int idx) {
    if (accounts[idx].accountType != "Savings") {
        cout << "  [!] Interest only applies to Savings accounts.\n";
        return;
    }

    double interest = accounts[idx].balance * (SAVINGS_INTEREST / 12.0);
    accounts[idx].balance += interest;
    saveAccounts(accounts);

    Transaction txn;
    txn.accountNumber = accounts[idx].accountNumber;
    txn.type          = "INTEREST";
    txn.amount        = interest;
    txn.balanceAfter  = accounts[idx].balance;
    txn.timestamp     = getCurrentTime();
    txn.note          = "Monthly interest credited";
    saveTransaction(txn);

    cout << "  [✓] Interest of $" << fixed << setprecision(2) << interest << " credited.\n";
    cout << "  New Balance: $" << accounts[idx].balance << "\n";
    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: View transaction history
// ─────────────────────────────────────────────
void viewTransactionHistory(const Account& acc) {
    printLine('=');
    cout << "        TRANSACTION HISTORY\n";
    cout << "  Account: " << acc.accountNumber << " (" << acc.holderName << ")\n";
    printLine('=');

    vector<Transaction> txns = loadTransactions(acc.accountNumber);

    if (txns.empty()) {
        cout << "  No transactions found.\n";
        printLine();
        return;
    }

    // Show last 10 transactions (most recent at top)
    int start = max(0, (int)txns.size() - 10);
    cout << left
         << setw(12) << "Type"
         << setw(12) << "Amount"
         << setw(14) << "Balance After"
         << "Timestamp\n";
    printLine();

    for (int i = (int)txns.size() - 1; i >= start; i--) {
        auto& t = txns[i];
        // Show + or - sign based on type
        string sign = (t.type == "DEPOSIT" || t.type == "TRANSFER_IN" || t.type == "INTEREST")
                      ? "+" : "-";
        cout << left
             << setw(12) << t.type
             << setw(2)  << sign << "$" << setw(9) << fixed << setprecision(2) << t.amount
             << "$" << setw(13) << t.balanceAfter
             << t.timestamp << "\n";
        if (!t.note.empty())
            cout << "             Note: " << t.note << "\n";
    }

    if (txns.size() > 10)
        cout << "\n  (Showing last 10 of " << txns.size() << " transactions)\n";

    printLine();
}

// ─────────────────────────────────────────────
//  FEATURE: Change PIN
// ─────────────────────────────────────────────
void changePin(vector<Account>& accounts, int idx) {
    printLine('=');
    cout << "              CHANGE PIN\n";
    printLine('=');

    cout << "  Enter current PIN : ";
    string currentPin;
    cin >> currentPin;

    if (currentPin != accounts[idx].pin) {
        cout << "  [!] Incorrect current PIN.\n";
        return;
    }

    cout << "  Enter new 4-digit PIN : ";
    string newPin;
    cin >> newPin;

    if (newPin.length() != 4) {
        cout << "  [!] PIN must be exactly 4 digits.\n";
        return;
    }

    accounts[idx].pin = newPin;
    saveAccounts(accounts);
    cout << "  [✓] PIN changed successfully.\n";
    printLine();
}

// ─────────────────────────────────────────────
//  MENU: Logged-in user menu
// ─────────────────────────────────────────────
void userMenu(vector<Account>& accounts, int idx) {
    int choice;

    while (true) {
        printLine('=');
        cout << "  Welcome, " << accounts[idx].holderName << "!\n";
        printLine('=');
        cout << "  1. View Dashboard (Balance & Info)\n";
        cout << "  2. Deposit Money\n";
        cout << "  3. Withdraw Money\n";
        cout << "  4. Transfer Money\n";
        cout << "  5. View Transaction History\n";
        cout << "  6. Apply Monthly Interest (Savings)\n";
        cout << "  7. Change PIN\n";
        cout << "  0. Logout\n";
        printLine();
        cout << "  Choice: ";
        cin >> choice;

        switch (choice) {
            case 1: showDashboard(accounts[idx]);          break;
            case 2: deposit(accounts, idx);                break;
            case 3: withdraw(accounts, idx);               break;
            case 4: transfer(accounts, idx);               break;
            case 5: viewTransactionHistory(accounts[idx]); break;
            case 6: applyInterest(accounts, idx);          break;
            case 7: changePin(accounts, idx);              break;
            case 0:
                cout << "  Logged out. Goodbye, " << accounts[idx].holderName << "!\n\n";
                return;
            default:
                cout << "  [!] Invalid choice.\n";
        }
    }
}

// ─────────────────────────────────────────────
//  FEATURE: Login to an existing account
// ─────────────────────────────────────────────
void login(vector<Account>& accounts) {
    printLine('=');
    cout << "               LOGIN\n";
    printLine('=');

    cout << "  Enter Account Number: ";
    string accNum;
    cin >> accNum;

    int idx = findAccount(accounts, accNum);

    if (idx == -1) {
        cout << "  [!] Account not found.\n";
        return;
    }
    if (!accounts[idx].isActive) {
        cout << "  [!] This account has been deactivated. Contact admin.\n";
        return;
    }

    cout << "  Enter PIN           : ";
    string pin;
    cin >> pin;

    if (pin != accounts[idx].pin) {
        cout << "  [!] Incorrect PIN.\n";
        return;
    }

    cout << "  [✓] Login successful!\n\n";
    userMenu(accounts, idx);
}

// ─────────────────────────────────────────────
//  FEATURE: Admin Panel
// ─────────────────────────────────────────────
void adminPanel(vector<Account>& accounts) {
    printLine('=');
    cout << "           ADMIN PANEL LOGIN\n";
    printLine('=');

    cout << "  Enter Admin Password: ";
    string pass;
    cin >> pass;

    if (pass != ADMIN_PIN) {
        cout << "  [!] Incorrect admin password.\n";
        return;
    }

    cout << "  [✓] Admin access granted.\n\n";

    int choice;
    while (true) {
        printLine('=');
        cout << "           ADMIN PANEL\n";
        printLine('=');
        cout << "  1. View All Accounts\n";
        cout << "  2. Deactivate an Account\n";
        cout << "  3. Reactivate an Account\n";
        cout << "  4. View All Transactions for an Account\n";
        cout << "  0. Exit Admin Panel\n";
        printLine();
        cout << "  Choice: ";
        cin >> choice;

        if (choice == 0) break;

        if (choice == 1) {
            // ── View all accounts ──
            printLine('=');
            cout << "           ALL ACCOUNTS\n";
            printLine('=');
            if (accounts.empty()) {
                cout << "  No accounts found.\n";
            } else {
                cout << left
                     << setw(12) << "Acc No."
                     << setw(20) << "Name"
                     << setw(10) << "Type"
                     << setw(12) << "Balance"
                     << "Status\n";
                printLine();
                for (const auto& acc : accounts) {
                    cout << left
                         << setw(12) << acc.accountNumber
                         << setw(20) << acc.holderName
                         << setw(10) << acc.accountType
                         << "$" << setw(11) << fixed << setprecision(2) << acc.balance
                         << (acc.isActive ? "Active" : "Inactive") << "\n";
                }
            }
            printLine();
        }

        else if (choice == 2) {
            // ── Deactivate account ──
            cout << "  Enter account number to deactivate: ";
            string accNum;
            cin >> accNum;
            int idx = findAccount(accounts, accNum);
            if (idx == -1) {
                cout << "  [!] Account not found.\n";
            } else if (!accounts[idx].isActive) {
                cout << "  [!] Account is already inactive.\n";
            } else {
                accounts[idx].isActive = false;
                saveAccounts(accounts);
                cout << "  [✓] Account " << accNum << " deactivated.\n";
            }
        }

        else if (choice == 3) {
            // ── Reactivate account ──
            cout << "  Enter account number to reactivate: ";
            string accNum;
            cin >> accNum;
            int idx = findAccount(accounts, accNum);
            if (idx == -1) {
                cout << "  [!] Account not found.\n";
            } else if (accounts[idx].isActive) {
                cout << "  [!] Account is already active.\n";
            } else {
                accounts[idx].isActive = true;
                saveAccounts(accounts);
                cout << "  [✓] Account " << accNum << " reactivated.\n";
            }
        }

        else if (choice == 4) {
            // ── View transactions for an account ──
            cout << "  Enter account number: ";
            string accNum;
            cin >> accNum;
            int idx = findAccount(accounts, accNum);
            if (idx == -1) {
                cout << "  [!] Account not found.\n";
            } else {
                viewTransactionHistory(accounts[idx]);
            }
        }

        else {
            cout << "  [!] Invalid choice.\n";
        }
    }
}

// ─────────────────────────────────────────────
//  MAIN: Main menu loop
// ─────────────────────────────────────────────
int main() {
    // Load existing accounts from file at startup
    vector<Account> accounts = loadAccounts();

    int choice;

    while (true) {
        printLine('=');
        cout << "    SMART BANKING MANAGEMENT SYSTEM\n";
        printLine('=');
        cout << "  1. Create New Account\n";
        cout << "  2. Login to Account\n";
        cout << "  3. Admin Panel\n";
        cout << "  0. Exit\n";
        printLine();
        cout << "  Choice: ";
        cin >> choice;

        switch (choice) {
            case 1: createAccount(accounts); break;
            case 2: login(accounts);         break;
            case 3: adminPanel(accounts);    break;
            case 0:
                cout << "\n  Thank you for using Smart Banking System. Goodbye!\n\n";
                return 0;
            default:
                cout << "  [!] Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
