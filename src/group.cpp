#include "group.h"
#include <iostream>
#include <sstream>
#include "input.h"
#include <string>
#include <vector>
#include "budget.h"
#include "percentage.h"
using namespace std;

void setupPeople(Group& g) {
    g.people.clear();
    g.expenses.clear();

    g.fixedWeight.clear();
    g.budget = -1;

    int count = readInt("How many people are in the group? ", 1, 50);
    for (int i = 0; i < count; i++) {
        string name = readText("Name of person " + to_string(i + 1) + ": ");
        if (name == "") name = "Person " + to_string(i + 1);
        g.people.push_back(name);
    }

    if (readYesNo("Set a budget now?")) {
        g.budget = readAmount("Budget total: ");
        showBudgetLine(g);
    }
    if (readYesNo("Set initial percentage split now?")) {
        g.fixedWeight = askPercentages(g);
        cout << "Fixed percentages saved:\n";
        printPercentages(g);
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

    bool useFixed = hasFixedPercent(g);
    cout << "How to split?  1. fixed %  (" << (useFixed ? "available" : "none set") << ")  2. even  3. custom %\n";
    int mode = readInt("Choose: ", 1, 3);
    if (mode == 1 && !useFixed) {
        cout << "No fixed percentages - using even split.\n";
        mode = 2;
    }
    if (mode == 1) {
        applyFixedWeights(g, e);
    } else if (mode == 2) {
        if (readYesNo("Split with everyone?")) {
            e.sharedBy.clear();
            for (int i = 0; i < (int)g.people.size(); i++) e.sharedBy.push_back(i);
        } else {
            e.sharedBy = readPeopleList(g);
        }
    } else {
        applyCustomWeights(g, e);
    }

    g.expenses.push_back(e);
    cout << "Split: ";
    for (size_t k = 0; k < e.sharedBy.size(); k++) {
        cout << (k ? ", " : "") << g.people[e.sharedBy[k]] << " " << money(shareOf(e, e.sharedBy[k]));
    }
    cout << "\nExpense added.\n";
    reportAfterExpense(g);
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
        if ((int)e.weights.size() == (int)e.sharedBy.size() && !e.weights.empty()) cout << " | %";
        cout << ")\n";
        total += e.amount;
    }
    cout << "Total spent: " << money(total) << "\n";
    showBudgetLine(g);
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

    showBudgetLine(g);
}