#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <algorithm>
using namespace std;


const string ACCOUNT_FILE = "accounts.txt";
const string TRANSACTION_FILE = "transactions.txt";
const double MIN_SAVINGS = 0;
const double OVERDRAFT_LIMIT = 5000.0;

struct Account
{
    int number;
    string name;
    string pin;
    string phone;
    string address;
    string type;          // SAVINGS or CURRENT
    double balance;
    double extra;         // interest rate for savings / overdraft for current
    bool active;
    string dateOpened;
};

struct Transaction
{
    int id;
    int accountNumber;
    string type;
    double amount;
    double balance;
    string date;
    string description;
};

vector<Account> accounts;
vector<Transaction> transactions;

int nextAccountNumber = 1001;
int nextTransactionId = 1;


// ==================== BASIC HELPER FUNCTIONS ====================

string currentDateTime()
{
    time_t now = time(0);
    tm *t = localtime(&now);

    char date[30];
    strftime(date, sizeof(date), "%d/%m/%Y %H:%M", t);
    return date;
}

void clearInput()
{
    cin.clear();
    cin.ignore(10000, '\n');
}

void pauseScreen()
{
    cout << "\nPress Enter to continue...";
    cin.get();
}

void line()
{
    cout << "------------------------------------------------------------\n";
}

void title(string text)
{
    cout << "\n============================================================\n";
    cout << "                    " << text << "\n";
    cout << "============================================================\n";
}

int getInt(string message)
{
    int value;

    while (true)
    {
        cout << message;
        cin >> value;

        if (!cin.fail())
        {
            clearInput();
            return value;
        }

        clearInput();
        cout << "Please enter a valid number.\n";
    }
}

double getAmount(string message)
{
    double amount;

    while (true)
    {
        cout << message;
        cin >> amount;

        if (!cin.fail() && amount > 0)
        {
            clearInput();
            return amount;
        }

        clearInput();
        cout << "Please enter a positive amount.\n";
    }
}

string getText(string message, bool allowEmpty = false)
{
    string text;

    while (true)
    {
        cout << message;
        getline(cin, text);

        if (allowEmpty || !text.empty())
            return text;

        cout << "This field cannot be empty.\n";
    }
}


// ==================== ACCOUNT FUNCTIONS ====================

int findAccount(int number)
{
    for (int i = 0; i < (int)accounts.size(); i++)
    {
        if (accounts[i].number == number)
            return i;
    }

    return -1;
}

bool accountExists(int number)
{
    return findAccount(number) != -1;
}

void showAccount(Account a)
{
    line();
    cout << "Account Number : " << a.number << "\n";
    cout << "Name           : " << a.name << "\n";
    cout << "Account Type   : " << a.type << "\n";
    cout << "Balance        : " << fixed << setprecision(2) << a.balance << "\n";
    cout << "Phone          : " << a.phone << "\n";
    cout << "Address        : " << a.address << "\n";
    cout << "Date Opened    : " << a.dateOpened << "\n";
    cout << "Status         : " << (a.active ? "ACTIVE" : "CLOSED") << "\n";

    if (a.type == "SAVINGS")
        cout << "Interest Rate  : " << a.extra << "%\n";
    else
        cout << "Overdraft Limit: " << a.extra << "\n";

    line();
}

void showAccountRow(Account a)
{
    cout << left
         << setw(10) << a.number
         << setw(20) << a.name
         << setw(12) << a.type
         << right << setw(12) << fixed << setprecision(2) << a.balance
         << "   " << (a.active ? "ACTIVE" : "CLOSED") << "\n";
}


// ==================== FILE FUNCTIONS ====================

void saveAccounts()
{
    ofstream file(ACCOUNT_FILE);

    for (Account a : accounts)
    {
        file << a.number << "|"
             << a.name << "|"
             << a.pin << "|"
             << a.phone << "|"
             << a.address << "|"
             << a.type << "|"
             << a.balance << "|"
             << a.extra << "|"
             << a.active << "|"
             << a.dateOpened << "\n";
    }

    file.close();
}

void loadAccounts()
{
    ifstream file(ACCOUNT_FILE);

    if (!file)
        return;

    string lineText;

    while (getline(file, lineText))
    {
        Account a;
        size_t p = 0;
        vector<string> data;

        while ((p = lineText.find('|')) != string::npos)
        {
            data.push_back(lineText.substr(0, p));
            lineText.erase(0, p + 1);
        }
        data.push_back(lineText);

        if (data.size() != 10)
            continue;

        a.number = stoi(data[0]);
        a.name = data[1];
        a.pin = data[2];
        a.phone = data[3];
        a.address = data[4];
        a.type = data[5];
        a.balance = stod(data[6]);
        a.extra = stod(data[7]);
        a.active = stoi(data[8]);
        a.dateOpened = data[9];

        accounts.push_back(a);

        if (a.number >= nextAccountNumber)
            nextAccountNumber = a.number + 1;
    }

    file.close();
}

void saveTransactions()
{
    ofstream file(TRANSACTION_FILE);

    for (Transaction t : transactions)
    {
        file << t.id << "|"
             << t.accountNumber << "|"
             << t.type << "|"
             << t.amount << "|"
             << t.balance << "|"
             << t.date << "|"
             << t.description << "\n";
    }

    file.close();
}

void loadTransactions()
{
    ifstream file(TRANSACTION_FILE);

    if (!file)
        return;

    string lineText;

    while (getline(file, lineText))
    {
        Transaction t;
        vector<string> data;
        size_t p = 0;

        while ((p = lineText.find('|')) != string::npos)
        {
            data.push_back(lineText.substr(0, p));
            lineText.erase(0, p + 1);
        }
        data.push_back(lineText);

        if (data.size() != 7)
            continue;

        t.id = stoi(data[0]);
        t.accountNumber = stoi(data[1]);
        t.type = data[2];
        t.amount = stod(data[3]);
        t.balance = stod(data[4]);
        t.date = data[5];
        t.description = data[6];

        transactions.push_back(t);

        if (t.id >= nextTransactionId)
            nextTransactionId = t.id + 1;
    }

    file.close();
}

void addTransaction(int accountNumber, string type,
                    double amount, double balance, string description)
{
    Transaction t;

    t.id = nextTransactionId++;
    t.accountNumber = accountNumber;
    t.type = type;
    t.amount = amount;
    t.balance = balance;
    t.date = currentDateTime();
    t.description = description;

    transactions.push_back(t);

    // Save immediately
    saveTransactions();
}


// ==================== ACCOUNT MANAGEMENT ====================

void createAccount()
{
    title("OPEN NEW ACCOUNT");

    Account a;

    a.number = nextAccountNumber++;

    a.name = getText("Enter full name: ");
    a.phone = getText("Enter phone number: ");
    a.address = getText("Enter address: ");

    while (true)
    {
        a.pin = getText("Create 4-digit PIN: ");

        if (a.pin.length() == 4 &&
            isdigit(a.pin[0]) &&
            isdigit(a.pin[1]) &&
            isdigit(a.pin[2]) &&
            isdigit(a.pin[3]))
            break;

        cout << "PIN must contain exactly 4 digits.\n";
    }

    int choice;

    do
    {
        cout << "\n1. Savings Account\n";
        cout << "2. Current Account\n";
        choice = getInt("Choose account type: ");

        if (choice != 1 && choice != 2)
            cout << "Please choose 1 or 2.\n";

    } while (choice != 1 && choice != 2);

    a.type = (choice == 1) ? "SAVINGS" : "CURRENT";

    if (a.type == "SAVINGS")
        a.extra = 4.0;
    else
        a.extra = OVERDRAFT_LIMIT;

    a.balance = getAmount("Enter opening deposit: ");

    if (a.type == "SAVINGS" && a.balance < MIN_SAVINGS)
    {
        cout << "Savings account needs at least NPR "
             << MIN_SAVINGS << ".\n";
        nextAccountNumber--;
        return;
    }

    a.active = true;
    a.dateOpened = currentDateTime();

    accounts.push_back(a);

    saveAccounts();
    addTransaction(a.number, "OPEN", a.balance,
                   a.balance, "Account opened");

    cout << "\nAccount created successfully!\n";
    cout << "Your account number is: " << a.number << "\n";
}

void showAllAccounts()
{
    title("ALL ACCOUNTS");

    if (accounts.empty())
    {
        cout << "No accounts found.\n";
        return;
    }

    cout << left
         << setw(10) << "Number"
         << setw(20) << "Name"
         << setw(12) << "Type"
         << right << setw(12) << "Balance"
         << "   Status\n";

    line();

    for (Account a : accounts)
        showAccountRow(a);

    line();
    cout << "Total accounts: " << accounts.size() << "\n";
}

void viewAccount()
{
    title("VIEW ACCOUNT");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    showAccount(accounts[index]);
}

void updateAccount()
{
    title("UPDATE ACCOUNT");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    Account &a = accounts[index];

    if (!a.active)
    {
        cout << "This account is closed.\n";
        return;
    }

    cout << "Leave a field empty to keep the old value.\n";

    string name = getText("New name: ", true);
    string phone = getText("New phone: ", true);
    string address = getText("New address: ", true);

    if (!name.empty())
        a.name = name;

    if (!phone.empty())
        a.phone = phone;

    if (!address.empty())
        a.address = address;

    saveAccounts();

    cout << "Account updated successfully.\n";
}

void closeAccount()
{
    title("CLOSE ACCOUNT");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    Account &a = accounts[index];

    if (!a.active)
    {
        cout << "Account is already closed.\n";
        return;
    }

    if (a.balance != 0)
    {
        cout << "The account balance is NPR " << a.balance << ".\n";
        cout << "Please withdraw or transfer the remaining money first.\n";
        return;
    }

    a.active = false;

    saveAccounts();

    addTransaction(a.number, "CLOSE", 0,
                   a.balance, "Account closed");

    cout << "Account closed successfully.\n";
}


// ==================== PIN AND MONEY FUNCTIONS ====================

bool checkPin(Account a)
{
    string pin = getText("Enter PIN: ");

    if (pin != a.pin)
    {
        cout << "Wrong PIN.\n";
        return false;
    }

    return true;
}

bool canWithdraw(Account a, double amount)
{
    if (amount <= 0)
        return false;

    if (a.type == "SAVINGS")
    {
        return a.balance - amount >= MIN_SAVINGS;
    }

    // Current account
    return a.balance - amount >= -a.extra;
}

void depositMoney()
{
    title("DEPOSIT MONEY");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1 || !accounts[index].active)
    {
        cout << "Account not found or closed.\n";
        return;
    }

    double amount = getAmount("Enter deposit amount: ");

    accounts[index].balance += amount;

    saveAccounts();

    addTransaction(number, "DEPOSIT", amount,
                   accounts[index].balance, "Cash deposit");

    cout << "Deposit successful.\n";
    cout << "New balance: NPR " << accounts[index].balance << "\n";
}

void withdrawMoney()
{
    title("WITHDRAW MONEY");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1 || !accounts[index].active)
    {
        cout << "Account not found or closed.\n";
        return;
    }

    if (!checkPin(accounts[index]))
        return;

    double amount = getAmount("Enter withdrawal amount: ");

    if (!canWithdraw(accounts[index], amount))
    {
        if (accounts[index].type == "SAVINGS")
            cout << "Savings account must keep at least NPR "
                 << MIN_SAVINGS << ".\n";
        else
            cout << "Withdrawal exceeds overdraft limit.\n";

        return;
    }

    accounts[index].balance -= amount;

    saveAccounts();

    addTransaction(number, "WITHDRAW", amount,
                   accounts[index].balance, "Cash withdrawal");

    cout << "Withdrawal successful.\n";
    cout << "New balance: NPR " << accounts[index].balance << "\n";
}

void transferMoney()
{
    title("TRANSFER MONEY");

    int from = getInt("Your account number: ");
    int fromIndex = findAccount(from);

    if (fromIndex == -1 || !accounts[fromIndex].active)
    {
        cout << "Your account was not found or is closed.\n";
        return;
    }

    if (!checkPin(accounts[fromIndex]))
        return;

    int to = getInt("Recipient account number: ");

    if (from == to)
    {
        cout << "You cannot transfer to the same account.\n";
        return;
    }

    int toIndex = findAccount(to);

    if (toIndex == -1 || !accounts[toIndex].active)
    {
        cout << "Recipient account not found or closed.\n";
        return;
    }

    double amount = getAmount("Enter transfer amount: ");

    if (!canWithdraw(accounts[fromIndex], amount))
    {
        cout << "Not enough available balance for this transfer.\n";
        return;
    }

    accounts[fromIndex].balance -= amount;
    accounts[toIndex].balance += amount;

    saveAccounts();

    addTransaction(from, "TRANSFER-OUT", amount,
                   accounts[fromIndex].balance,
                   "Transfer to account " + to_string(to));

    addTransaction(to, "TRANSFER-IN", amount,
                   accounts[toIndex].balance,
                   "Transfer from account " + to_string(from));

    cout << "Transfer successful.\n";
    cout << "Your new balance: NPR "
         << accounts[fromIndex].balance << "\n";
}

void balanceInquiry()
{
    title("BALANCE INQUIRY");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    cout << "Account Holder : " << accounts[index].name << "\n";
    cout << "Balance        : NPR "
         << fixed << setprecision(2)
         << accounts[index].balance << "\n";
}


// ==================== TRANSACTION HISTORY ====================

void transactionHistory()
{
    title("TRANSACTION HISTORY");

    int number = getInt("Enter account number: ");

    if (findAccount(number) == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    bool found = false;

    cout << left
         << setw(5) << "ID"
         << setw(15) << "Type"
         << setw(12) << "Amount"
         << setw(12) << "Balance"
         << setw(18) << "Date"
         << "Description\n";

    line();

    for (Transaction t : transactions)
    {
        if (t.accountNumber == number)
        {
            cout << left
                 << setw(5) << t.id
                 << setw(15) << t.type
                 << setw(12) << fixed << setprecision(2) << t.amount
                 << setw(12) << t.balance
                 << setw(18) << t.date
                 << t.description << "\n";

            found = true;
        }
    }

    if (!found)
        cout << "No transactions found.\n";
}

void accountStatement()
{
    title("ACCOUNT STATEMENT");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
    {
        cout << "Account not found.\n";
        return;
    }

    showAccount(accounts[index]);

    cout << "\nTransactions:\n";
    line();

    for (Transaction t : transactions)
    {
        if (t.accountNumber == number)
        {
            cout << t.date << " | "
                 << t.type << " | NPR "
                 << fixed << setprecision(2) << t.amount
                 << " | Balance: " << t.balance
                 << " | " << t.description << "\n";
        }
    }
}


// ==================== SEARCH AND SORT ====================

void searchByNumber()
{
    title("SEARCH BY ACCOUNT NUMBER");

    int number = getInt("Enter account number: ");
    int index = findAccount(number);

    if (index == -1)
        cout << "Account not found.\n";
    else
        showAccount(accounts[index]);
}

void searchByName()
{
    title("SEARCH BY NAME");

    string keyword = getText("Enter name: ");
    bool found = false;

    for (Account a : accounts)
    {
        if (a.name.find(keyword) != string::npos)
        {
            showAccountRow(a);
            found = true;
        }
    }

    if (!found)
        cout << "No matching account found.\n";
}

void sortAccounts()
{
    title("SORT ACCOUNTS");

    cout << "1. Balance: High to Low\n";
    cout << "2. Balance: Low to High\n";
    cout << "3. Name: A to Z\n";
    cout << "4. Account Number\n";

    int choice = getInt("Choose: ");

    vector<Account> sorted = accounts;

    if (choice == 1)
    {
        sort(sorted.begin(), sorted.end(),
             [](Account a, Account b)
             {
                 return a.balance > b.balance;
             });
    }
    else if (choice == 2)
    {
        sort(sorted.begin(), sorted.end(),
             [](Account a, Account b)
             {
                 return a.balance < b.balance;
             });
    }
    else if (choice == 3)
    {
        sort(sorted.begin(), sorted.end(),
             [](Account a, Account b)
             {
                 return a.name < b.name;
             });
    }
    else if (choice == 4)
    {
        sort(sorted.begin(), sorted.end(),
             [](Account a, Account b)
             {
                 return a.number < b.number;
             });
    }
    else
    {
        cout << "Invalid choice.\n";
        return;
    }

    line();

    for (Account a : sorted)
        showAccountRow(a);
}


// ==================== REPORTS ====================

void bankSummary()
{
    title("BANK SUMMARY");

    int active = 0;
    int closed = 0;
    int savings = 0;
    int current = 0;

    double totalBalance = 0;

    for (Account a : accounts)
    {
        if (a.active)
        {
            active++;
            totalBalance += a.balance;

            if (a.type == "SAVINGS")
                savings++;
            else
                current++;
        }
        else
        {
            closed++;
        }
    }

    cout << "Total Accounts  : " << accounts.size() << "\n";
    cout << "Active Accounts : " << active << "\n";
    cout << "Closed Accounts : " << closed << "\n";
    cout << "Savings Accounts: " << savings << "\n";
    cout << "Current Accounts: " << current << "\n";
    cout << "Total Balance   : NPR "
         << fixed << setprecision(2)
         << totalBalance << "\n";
    cout << "Transactions    : " << transactions.size() << "\n";
}

void lowBalanceReport()
{
    title("LOW BALANCE REPORT");

    double limit = getAmount("Show accounts below: ");
    bool found = false;

    for (Account a : accounts)
    {
        if (a.active && a.balance < limit)
        {
            cout << a.number << " | "
                 << a.name << " | NPR "
                 << fixed << setprecision(2)
                 << a.balance << "\n";

            found = true;
        }
    }

    if (!found)
        cout << "No accounts found below this amount.\n";
}

void interestReport()
{
    title("MONTHLY INTEREST REPORT");

    bool found = false;
    double totalInterest = 0;

    for (Account a : accounts)
    {
        if (a.active && a.type == "SAVINGS")
        {
            double interest = a.balance * (a.extra / 100) / 12;

            cout << a.number << " | "
                 << a.name << " | Rate: "
                 << a.extra << "% | Interest: NPR "
                 << fixed << setprecision(2)
                 << interest << "\n";

            totalInterest += interest;
            found = true;
        }
    }

    if (!found)
    {
        cout << "No active savings accounts.\n";
        return;
    }

    line();
    cout << "Total monthly interest: NPR "
         << totalInterest << "\n";
}


// ==================== MENUS ====================

void accountMenu()
{
    int choice;

    do
    {
        title("ACCOUNT MANAGEMENT");

        cout << "1. Open Account\n";
        cout << "2. View All Accounts\n";
        cout << "3. View One Account\n";
        cout << "4. Update Account\n";
        cout << "5. Close Account\n";
        cout << "0. Back\n";

        choice = getInt("Choose: ");

        switch (choice)
        {
            case 1: createAccount(); break;
            case 2: showAllAccounts(); break;
            case 3: viewAccount(); break;
            case 4: updateAccount(); break;
            case 5: closeAccount(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

void transactionMenu()
{
    int choice;

    do
    {
        title("TRANSACTIONS");

        cout << "1. Deposit\n";
        cout << "2. Withdraw\n";
        cout << "3. Transfer\n";
        cout << "4. Balance Inquiry\n";
        cout << "5. Transaction History\n";
        cout << "6. Account Statement\n";
        cout << "0. Back\n";

        choice = getInt("Choose: ");

        switch (choice)
        {
            case 1: depositMoney(); break;
            case 2: withdrawMoney(); break;
            case 3: transferMoney(); break;
            case 4: balanceInquiry(); break;
            case 5: transactionHistory(); break;
            case 6: accountStatement(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

void searchMenu()
{
    int choice;

    do
    {
        title("SEARCH AND SORT");

        cout << "1. Search by Account Number\n";
        cout << "2. Search by Name\n";
        cout << "3. Sort Accounts\n";
        cout << "0. Back\n";

        choice = getInt("Choose: ");

        switch (choice)
        {
            case 1: searchByNumber(); break;
            case 2: searchByName(); break;
            case 3: sortAccounts(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}

void reportMenu()
{
    int choice;

    do
    {
        title("REPORTS");

        cout << "1. Bank Summary\n";
        cout << "2. Low Balance Report\n";
        cout << "3. Monthly Interest Report\n";
        cout << "0. Back\n";

        choice = getInt("Choose: ");

        switch (choice)
        {
            case 1: bankSummary(); break;
            case 2: lowBalanceReport(); break;
            case 3: interestReport(); break;
            case 0: break;
            default: cout << "Invalid choice.\n";
        }

        if (choice != 0)
            pauseScreen();

    } while (choice != 0);
}


// ==================== MAIN MENU ====================

void mainMenu()
{
    int choice;

    do
    {
        title("BANK ACCOUNT MANAGEMENT SYSTEM");

        cout << "1. Account Management\n";
        cout << "2. Transactions\n";
        cout << "3. Search and Sort\n";
        cout << "4. Reports\n";
        cout << "0. Exit\n";

        choice = getInt("Choose: ");

        switch (choice)
        {
            case 1: accountMenu(); break;
            case 2: transactionMenu(); break;
            case 3: searchMenu(); break;
            case 4: reportMenu(); break;

            case 0:
                saveAccounts();
                saveTransactions();
                cout << "\nThank you for using the Bank System!\n";
                break;

            default:
                cout << "Invalid choice.\n";
                pauseScreen();
        }

    } while (choice != 0);
}


// ==================== MAIN FUNCTION ====================

int main()
{
    cout << fixed << setprecision(2);

    loadAccounts();
    loadTransactions();

    cout << "============================================\n";
    cout << "       WELCOME TO BANK MANAGEMENT SYSTEM\n";
    cout << "============================================\n";
    cout << "Existing accounts: " << accounts.size() << "\n";

    pauseScreen();

    mainMenu();

    return 0;
}
