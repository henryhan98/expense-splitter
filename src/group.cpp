#include "group.h"
#include <iostream>
#include <sstream>
#include "input.h"
using namespace std;

void setupPeople(Group& g) {
    g.people.clear();
    g.expenses.clear();

    int count = readInt("How many people are in the group? ", 1, 50);
    for (int i = 0; i < count; i++) {
        string name = readText("Name of person " + to_string(i + 1) + ": ");
        if (name == "") name = "Person " + to_string(i + 1);
        g.people.push_back(name);
    }
}

void printPeople(const Group& g) {
    for (int i = 0; i < (int)g.people.size(); i++) {
        cout << "  " << i + 1 << ". " << g.people[i] << "\n";
    }
}

static vector<int> readPeopleList(const Group& g) {
    int n = g.people.size();
    while (true) {
        string line = readText("Enter the numbers of the people sharing it (e.g. 1 3): ");
        stringstream ss(line);
        vector<bool> picked(n, false);
        int number;
        bool ok = true;

        while (ss >> number) {
            if (number < 1 || number > n) {
                ok = false;
                break;
            }
            picked[number - 1] = true;
        }
        if (!ss.eof()) ok = false;

        vector<int> result;
        for (int i = 0; i < n; i++) {
            if (picked[i]) result.push_back(i);
        }
        if (ok && !result.empty()) return result;
        cout << "Please enter numbers from 1 to " << n << ", separated by spaces.\n";
    }
}

void addExpense(Group& g) {
    Expense e;
    e.day = readInt("Day number (e.g. 1): ", 1, 365);
    e.description = readText("What was it for? ");
    if (e.description == "") e.description = "(no description)";

    printPeople(g);
    e.payer = readInt("Who paid? (enter their number): ", 1, g.people.size()) - 1;
    e.amount = readAmount("Amount: ");


    cout << "\nSelect category:\n";
    cout << "1. Food\n";
    cout << "2. Transport\n";
    cout << "3. Accommodation\n";
    cout << "4. Entertainment\n";
    cout << "5. Other\n";
    
    int categoryChoice = readInt("Category: ", 1, 5);
    
    e.category = static_cast<Category>(categoryChoice - 1);

    if (readYesNo("Split between everyone?")) {
        for (int i = 0; i < (int)g.people.size(); i++) e.sharedBy.push_back(i);
    } else {
        e.sharedBy = readPeopleList(g);
    }

    g.expenses.push_back(e);
    cout << "Expense added.\n";
}

void listExpenses(const Group& g) {
    if (g.expenses.empty()) {
        cout << "No expenses yet.\n";
        return;
    }
    int total = 0;
    for (int i = 0; i < (int)g.expenses.size(); i++) {
        const Expense& e = g.expenses[i];
        cout << "#" << i + 1 << "  Day " << e.day << "  " << e.description << "  - "
             << g.people[e.payer] << " paid " << money(e.amount) << "  (shared by: ";
        if (e.sharedBy.size() == g.people.size()) {
            cout << "everyone";
        } else {
            for (int k = 0; k < (int)e.sharedBy.size(); k++) {
                if (k > 0) cout << ", ";
                cout << g.people[e.sharedBy[k]];
            }
        }
        cout << ")\n";
        total += e.amount;
    }
    cout << "Total spent: " << money(total) << "\n";
}

void removeExpense(Group& g) {
    if (g.expenses.empty()) {
        cout << "No expenses to remove.\n";
        return;
    }
    listExpenses(g);
    int number = readInt("Which expense to remove? (0 = cancel): ", 0, g.expenses.size());
    if (number == 0) return;
    g.expenses.erase(g.expenses.begin() + (number - 1));
    cout << "Expense #" << number << " removed.\n";
}
