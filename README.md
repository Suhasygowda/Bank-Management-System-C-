# Smart Banking Management System

A robust, console-based banking application implemented in C++. This system provides a comprehensive set of banking features for both regular users and administrators, with persistent data storage using file I/O.

## 🚀 Features

### For Users
- **Account Creation**: Support for Savings and Current accounts with a minimum initial deposit.
- **Secure Login**: PIN-based authentication for account access.
- **Financial Transactions**:
    - **Deposit**: Easily add funds to your account.
    - **Withdrawal**: Securely withdraw funds (maintains a minimum balance of $100).
    - **Fund Transfer**: Transfer money to other accounts within the system.
- **Interest Calculation**: Automated monthly interest calculation for Savings accounts (4% annual rate).
- **Transaction History**: View the last 10 transactions with detailed notes and timestamps.
- **Account Management**: Change your 4-digit PIN at any time.

### For Administrators
- **Admin Panel**: Secure access using a master administrator PIN.
- **Account Oversight**: View all registered accounts, including balances and status.
- **Account Control**: Deactivate or reactivate user accounts.
- **Audit Logs**: View transaction history for any specific account.

## 🛠️ Technologies Used
- **Language**: C++
- **Persistence**: File-based storage (`accounts.dat`, `transactions.dat`)
- **Standard Library**: `<iostream>`, `<fstream>`, `<vector>`, `<iomanip>`, `<ctime>`, etc.

## 📂 Project Structure
```text
BankManagamentSystem/
├── main.cpp          # Core application logic
├── accounts.dat      # Persistent storage for account information
├── transactions.dat  # Persistent storage for transaction logs
└── README.md         # Project documentation
```

## ⚙️ Getting Started

### Prerequisites
- A C++ compiler (e.g., GCC, Clang, or MSVC).

### Compilation
Open your terminal or command prompt and navigate to the project directory. Run the following command:
```bash
g++ -o BankSystem main.cpp
```

### Execution
Run the compiled executable:
```bash
./BankSystem
```
*(On Windows, use `BankSystem.exe`)*

## 📖 Usage Guide

### Default Credentials
- **Admin PIN**: `admin123` (Defined in `main.cpp`)

### Basic Workflow
1. **Launch**: Start the application to see the main menu.
2. **Account Creation**: Choose "Create New Account" to set up your profile. Note down your generated Account Number.
3. **User Access**: Use your Account Number and 4-digit PIN to login.
4. **Transactions**: Perform deposits, withdrawals, or transfers from your dashboard.
5. **Admin Access**: Access the Admin Panel to manage the entire system.

## 💾 Data Persistence
The system uses custom delimited text files to ensure data is saved even after the application is closed:
- `accounts.dat`: Stores `AccountNumber|Name|PIN|Type|Balance|Status|CreatedAt`.
- `transactions.dat`: Stores `AccountNumber|Type|Amount|BalanceAfter|Timestamp|Note`.

## ⚠️ Security Note
*This is a demonstration project. In a real-world production environment, PINs should be hashed (e.g., using SHA-256) and never stored in plain text.*

---
Developed with ❤️ for C++ Learning.
