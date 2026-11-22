#include "report.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
using namespace std;

static void printRow(const string& name, int paid, int share) {
    int net = paid - share;
    string netText = money(net);
    if (net > 0) netText = "+" + netText;
    cout << "  " << left << setw(14) << name << right << setw(10) << money(paid) << setw(10)
         << money(share) << setw(10) << netText << "\n";
}

static void printHeader() {
    cout << "  " << left << setw(14) << "Name" << right << setw(10) << "Paid" << setw(10) << "Share"
         << setw(10) << "Net" << "\n";
}

void showByDay(const Group& g) {
    if (g.expenses.empty()) {
        cout << "No expenses yet.\n";
        return;
    }
    set<int> days;
    for (const Expense& e : g.expenses) days.insert(e.day);

    for (int day : days) {
        int dayTotal = 0;
        cout << "\n=== Day " << day << " ===\n";
        for (const Expense& e : g.expenses) {
            if (e.day == day) {
                cout << "  " << g.people[e.payer] << " paid " << money(e.amount) << " for "
                     << e.description << "\n";
                dayTotal += e.amount;
            }
        }
        cout << "  Total for the day: " << money(dayTotal) << "\n\n";

        printHeader();
        for (int p = 0; p < (int)g.people.size(); p++) {
            int paid = 0, share = 0;
            for (const Expense& e : g.expenses) {
                if (e.day != day) continue;
                if (e.payer == p) paid += e.amount;
                share += shareOf(e, p);
            }
            printRow(g.people[p], paid, share);
        }
    }
}

void showBalances(const Group& g) {
    if (g.expenses.empty()) {
        cout << "No expenses yet.\n";
        return;
    }
    cout << "\nNet + means they get money back, - means they owe money.\n";
    printHeader();
    for (int p = 0; p < (int)g.people.size(); p++) {
        int paid = 0, share = 0;
        for (const Expense& e : g.expenses) {
            if (e.payer == p) paid += e.amount;
            share += shareOf(e, p);
        }
        printRow(g.people[p], paid, share);
    }
}

void settleUp(const Group& g) {
    int n = g.people.size();
    vector<int> net(n, 0);
    for (const Expense& e : g.expenses) {
        net[e.payer] += e.amount;
        for (int p = 0; p < n; p++) net[p] -= shareOf(e, p);
    }
    bool anyPayments = false;
    while (true) {
        int mostOwed = 0, mostOwing = 0;
        for (int p = 0; p < n; p++) {
            if (net[p] > net[mostOwed]) mostOwed = p;
            if (net[p] < net[mostOwing]) mostOwing = p;
        }
        if (net[mostOwed] == 0) break;
        int amount = min(net[mostOwed], -net[mostOwing]);
        cout << "  " << g.people[mostOwing] << " pays " << g.people[mostOwed] << " " << money(amount)
             << "\n";
        net[mostOwed] -= amount;
        net[mostOwing] += amount;
        anyPayments = true;
    }
    if (!anyPayments) cout << "Everyone is square.\n";
}


void showCategoryTotals(const Group& g) {
    int totals[5] = {0};

    
    for (const Expense& e : g.expenses) {
        totals[e.category] += e.amount;
    }

    cout << "\n=== Category Totals ===\n";
    cout << "Food: " << money(totals[FOOD]) << "\n";
    
    cout << "Transport: " << money(totals[TRANSPORT]) << "\n";
    cout << "Accommodation: " << money(totals[ACCOMMODATION]) << "\n";
    cout << "Entertainment: " << money(totals[ENTERTAINMENT]) << "\n";
    cout << "Other: " << money(totals[OTHER]) << "\n";
}
