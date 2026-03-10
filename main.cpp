#include <iostream>
#include <string>

using namespace std;

class Account
{
public:
    int accNo;
    string name;
    string ifsc;
    double balance;

    static const int MAX = 100;
    static Account accounts[MAX];
    static int totalAccounts;

    Account() {}

    Account(int a, string n, string i, double b)
    {
        accNo = a;
        name = n;
        ifsc = i;
        balance = b;
    }

    static int findAccount(int number)
    {
        for(int i=0;i<totalAccounts;i++)
        {
            if(accounts[i].accNo == number)
                return i;
        }
        return -1;
    }
};

Account Account::accounts[Account::MAX];
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

        int index = Account::findAccount(number);

        if(index == -1)
        {
            cout << "Account not found\n";
            return;
        }

        cout << "Enter Amount: ";
        cin >> amount;

        if(amount <= 0)
        {
            cout << "Invalid amount\n";
            return;
        }

        accounts[index].balance += amount;

        cout << "Deposit Successful\n";
        cout << "New Balance: " << accounts[index].balance << endl;
    }
};

class Withdrawal : public Account
{
public:

    bool withdrawMoney(int number,double amount)
    {
        int index = Account::findAccount(number);

        if(index == -1)
        {
            cout<<"Account not found\n";
            return false;
        }

        if(accounts[index].balance < amount)
        {
            cout<<"Insufficient Balance\n";
            return false;
        }

        accounts[index].balance -= amount;

        cout<<"Withdrawal Successful\n";
        cout<<"Remaining Balance: "<<accounts[index].balance<<endl;

        return true;
    }
};

class ChequeTransfer : public Deposit, public Withdrawal
{
public:

    void depositCheque()
    {
        int sender,receiver;
        string holderName,ifsc;
        double amount;

        cout<<"\n--- CHEQUE TRANSFER ---\n";

        cout<<"Enter Sender Account: ";
        cin>>sender;

        cout<<"Enter Receiver Account: ";
        cin>>receiver;

        int senderIndex = Account::findAccount(sender);
        int receiverIndex = Account::findAccount(receiver);

        if(senderIndex==-1 || receiverIndex==-1)
        {
            cout<<"Account verification failed\n";
            return;
        }

        cout<<"Enter Receiver Name: ";
        cin>>holderName;

        cout<<"Enter IFSC Code: ";
        cin>>ifsc;

        if(accounts[receiverIndex].name!=holderName ||
           accounts[receiverIndex].ifsc!=ifsc)
        {
            cout<<"Cheque verification failed\n";
            return;
        }

        cout<<"Enter Amount: ";
        cin>>amount;

        if(withdrawMoney(sender,amount))
        {
            accounts[receiverIndex].balance += amount;
            cout<<"Cheque Transfer Successful\n";
        }
        else
        {
            cout<<"Cheque Bounced\n";
        }
    }
};

int main()
{
    Deposit d;
    Withdrawal w;
    ChequeTransfer c;

    Account::accounts[0] = Account(101,"Rahul","SBIN001",5000);
    Account::accounts[1] = Account(102,"Amit","SBIN001",3000);
    Account::accounts[2] = Account(103,"Priya","SBIN002",7000);

    Account::totalAccounts = 3;

    int choice;

    do
    {
        cout<<"\n===== BANK MENU =====\n";
        cout<<"1 Deposit\n";
        cout<<"2 Withdraw\n";
        cout<<"3 Cheque Transfer\n";
        cout<<"4 Exit\n";

        cin>>choice;

        switch(choice)
        {
            case 1:
                d.depositMoney();
                break;

            case 2:
            {
                int acc;
                double amt;

                cout<<"Enter Account Number: ";
                cin>>acc;

                cout<<"Enter Amount: ";
                cin>>amt;

                w.withdrawMoney(acc,amt);
                break;
            }

            case 3:
                c.depositCheque();
                break;
        }

    }while(choice!=4);

    return 0;
}
