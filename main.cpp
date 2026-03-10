#include <iostream>
#include <string>

using namespace std;

class Account
{
protected:
    int accNo;
    string name;
    string ifsc;
    double balance;

public:

    static const int MAX = 100;
    static Account accounts[MAX];
    static int totalAccounts;

    int findAccount(int number)
    {
        for(int i = 0; i < totalAccounts; i++)
        {
            if(accounts[i].accNo == number)
                return i;
        }
        return -1;
    }
};

Account Account::accounts[MAX];
int Account::totalAccounts = 0;

class Deposit : public Account
{
public:

    void depositMoney()
    {
        int number;
        double amount;

        cout << "\n--- DEPOSIT ---\n";
        cout << "Enter Account Number: ";
        cin >> number;

        int index = findAccount(number);

        if(index == -1)
        {
            cout << "Account not found.\n";
            return;
        }

        cout << "Enter Amount: ";
        cin >> amount;

        if(amount <= 0)
        {
            cout << "Invalid amount.\n";
            return;
        }

        accounts[index].balance += amount;

        cout << "[DEPOSIT SUCCESSFUL]\n";
        cout << "New Balance: " << accounts[index].balance << endl;
    }
};

class Withdrawal : public Account
{
public:

    bool withdrawMoney(int number, double amount)
    {
        int index = findAccount(number);

        if(index == -1)
        {
            cout << "Account not found.\n";
            return false;
        }

        if(amount <= 0)
        {
            cout << "Invalid withdrawal amount.\n";
            return false;
        }

        if(accounts[index].balance < amount)
        {
            cout << "[INSUFFICIENT BALANCE]\n";
            return false;
        }

        accounts[index].balance -= amount;

        cout << "[WITHDRAWAL SUCCESSFUL]\n";
        cout << "Remaining Balance: " << accounts[index].balance << endl;

        return true;
    }
};

class ChequeTransfer : public Deposit, public Withdrawal
{
public:

    void depositCheque()
    {
        int sender, receiver;
        string holderName, ifsc;
        double amount;

        cout << "\n--- CHEQUE TRANSFER ---\n";

        cout << "Enter Sender Account: ";
        cin >> sender;

        cout << "Enter Receiver Account: ";
        cin >> receiver;

        int senderIndex = findAccount(sender);
        int receiverIndex = findAccount(receiver);

        if(senderIndex == -1 || receiverIndex == -1)
        {
            cout << "Account verification failed.\n";
            return;
        }

        cout << "Enter Receiver Name: ";
        cin >> holderName;

        cout << "Enter IFSC Code: ";
        cin >> ifsc;

        if(accounts[receiverIndex].name != holderName ||
           accounts[receiverIndex].ifsc != ifsc)
        {
            cout << "[CHEQUE VERIFICATION FAILED]\n";
            return;
        }

        cout << "Enter Cheque Amount: ";
        cin >> amount;

        if(amount <= 0)
        {
            cout << "Invalid cheque amount.\n";
            return;
        }

        bool success = withdrawMoney(sender, amount);

        if(success)
        {
            accounts[receiverIndex].balance += amount;

            cout << "[CHEQUE DEPOSIT SUCCESSFUL]\n";
            cout << "Amount transferred successfully.\n";
        }
        else
        {
            cout << "[CHEQUE BOUNCED]\n";
        }
    }
};

int main()
{
    Deposit d;
    Withdrawal w;
    ChequeTransfer c;

    int choice;

    // Sample accounts for testing
    Account::accounts[0] = {101, "Rahul", "SBIN001", 5000};
    Account::accounts[1] = {102, "Amit", "SBIN001", 3000};
    Account::accounts[2] = {103, "Priya", "SBIN002", 7000};

    Account::totalAccounts = 3;

    do
    {
        cout << "\n===== BANKING SYSTEM MENU =====\n";
        cout << "1. Deposit Money\n";
        cout << "2. Withdraw Money\n";
        cout << "3. Cheque Transfer\n";
        cout << "4. Exit\n";
        cout << "Enter your choice: ";

        cin >> choice;

        switch(choice)
        {
            case 1:
                d.depositMoney();
                break;

            case 2:
            {
                int accNo;
                double amount;

                cout << "Enter Account Number: ";
                cin >> accNo;

                cout << "Enter Amount: ";
                cin >> amount;

                w.withdrawMoney(accNo, amount);
                break;
            }

            case 3:
                c.depositCheque();
                break;

            case 4:
                cout << "Exiting system...\n";
                break;

            default:
                cout << "Invalid choice.\n";
        }

    } while(choice != 4);

    return 0;
}
