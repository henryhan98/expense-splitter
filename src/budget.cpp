#include "budget.h"
#include <cmath>
#include <iostream>
#include <string>
#include "input.h"
using namespace std;

int totalSpent(const Group& g) {
    int t = 0;
    for (const Expense& e : g.expenses) {
        t += e.amount;
    }
    return t;
}

bool hasBudget(const Group& g) {
    return g.budget >= 0;
}

void showBudgetLine(const Group& g) {
    int spent = totalSpent(g);
    if (!hasBudget(g)) {
        cout << "Budget: not set  |  Spent: " << money(spent) << "\n";
        return;
    }
    int left = g.budget - spent;
    cout << "Budget: " << money(g.budget) << "  |  Spent: " << money(spent) << "  |  Left: " << money(left);
    if (left < 0) cout << "  *** OVER BUDGET by " << money(-left) << " ***";
    cout << "\n";
}

void adjustBudget(Group& g) {
    while (true) {
        string s = readText("Adjust budget by (e.g. +100.00 or -50.00): ");
        try {
            size_t used = 0;
            double v = stod(s, &used);
            if (used == s.size() && v > -1e7 && v < 1e7) {
                if (!hasBudget(g)) g.budget = 0;
                g.budget += (int)llround(v * 100);
                if (g.budget < 0) g.budget = 0;
                return;
            }
        } catch (...) {
        }
        cout << "Please enter a number, e.g. 200 or -50\n";
    }
}

void reportAfterExpense(const Group& g) {
    if (!hasBudget(g)) return;
    int left = g.budget - totalSpent(g);
    if (left < 0) {
        cout << "*** OVER BUDGET by " << money(-left) << " ***\n";
    } else {
        cout << "Budget left: " << money(left) << "\n";
    }
}