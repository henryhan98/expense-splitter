#include "report.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
using namespace std;

static void printRow(ostream& out, const string& name, int paid, int share) {
    int net = paid - share;
    string netText = money(net);
    if (net > 0) netText = "+" + netText;
    out << "  " << left << setw(14) << name << right << setw(10) << money(paid) << setw(10)
         << money(share) << setw(10) << netText << "\n";
}

static void printHeader(ostream& out) {
    out << "  " << left << setw(14) << "Name" << right << setw(10) << "Paid" << setw(10) << "Share"
         << setw(10) << "Net" << "\n";
}

static void writeByDay(const Group& g, ostream& out) {
    if (g.expenses.empty()) {
        out << "No expenses yet.\n";
        return;
    }
    set<int> days;
    for (const Expense& e : g.expenses) days.insert(e.day);

    for (int day : days) {
        int dayTotal = 0;
        out << "\n=== Day " << day << " ===\n";
        for (const Expense& e : g.expenses) {
            if (e.day == day) {
                out << "  " << g.people[e.payer] << " paid " << money(e.amount) << " for "
                     << e.description << "\n";
                dayTotal += e.amount;
            }
        }
        out << "  Total for the day: " << money(dayTotal) << "\n\n";

        printHeader(out);
        for (int p = 0; p < (int)g.people.size(); p++) {
            int paid = 0, share = 0;
            for (const Expense& e : g.expenses) {
                if (e.day != day) continue;
                if (e.payer == p) paid += e.amount;
                share += shareOf(e, p);
            }
            printRow(out, g.people[p], paid, share);
        }
    }
}

static void writeBalances(const Group& g, ostream& out) {
    if (g.expenses.empty()) {
        out << "No expenses yet.\n";
        return;
    }
    out << "\nNet + means they get money back, - means they owe money.\n";
    printHeader(out);
    for (int p = 0; p < (int)g.people.size(); p++) {
        int paid = 0, share = 0;
        for (const Expense& e : g.expenses) {
            if (e.payer == p) paid += e.amount;
            share += shareOf(e, p);
        }
        printRow(out, g.people[p], paid, share);
    }
}

static void writeSettlement(const Group& g, ostream& out) {
    int n = g.people.size();
    if (n == 0) { out << "Everyone is square.\n"; return; }
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
        out << "  " << g.people[mostOwing] << " pays " << g.people[mostOwed] << " " << money(amount)
             << "\n";
        net[mostOwed] -= amount;
        net[mostOwing] += amount;
        anyPayments = true;
    }
    if (!anyPayments) out << "Everyone is square.\n";
}
std::string dailyReport(const Group& g) { ostringstream out; writeByDay(g, out); return out.str(); }
std::string balanceReport(const Group& g) { ostringstream out; writeBalances(g, out); return out.str(); }
std::string settlementReport(const Group& g) { ostringstream out; writeSettlement(g, out); return out.str(); }

void showByDay(const Group& g) { cout << dailyReport(g); }
void showBalances(const Group& g) { cout << balanceReport(g); }
void settleUp(const Group& g) { cout << settlementReport(g); }

static void writeCategoryTotals(const Group& g, ostream& out) {
    int totals[5] = {0};


    for (const Expense& e : g.expenses) {
        totals[e.category] += e.amount;
    }

    out << "\n=== Category Totals ===\n";
    out << "Food: " << money(totals[FOOD]) << "\n";

    out << "Transport: " << money(totals[TRANSPORT]) << "\n";
    out << "Accommodation: " << money(totals[ACCOMMODATION]) << "\n";
    out << "Entertainment: " << money(totals[ENTERTAINMENT]) << "\n";
    out << "Other: " << money(totals[OTHER]) << "\n";
}

static void writeCategoryAnalytics(const Group& g, ostream& out)
{
    int totals[5] = {0};

    for (const Expense& e : g.expenses) {
        totals[e.category] += e.amount;
    }

    int grandTotal = 0;

    for (int i = 0; i < 5; i++) {
        grandTotal += totals[i];
    }

    if (grandTotal == 0) {
        out << "No expenses yet.\n";
        return;
    }

    const string names[5] = {
        "Food",
        "Transport",
        "Accommodation",
        "Entertainment",
        "Other"
    };

    out << "\n=== Category Analytics ===\n";


    for (int i = 0; i < 5; i++) {
        double percentage =
            totals[i] * 100.0 / grandTotal;

        out << names[i]
             << ": "
             << money(totals[i])
             << " ("
             << percentage
             << "%)\n";

    }

    int largest = 0;
    for (int i = 1; i < 5; i++) {
        if (totals[i] > totals[largest]) {
            largest = i;
        }
    }
    out << "\nLargest Category: "
         << names[largest]
         << " ("
         << money(totals[largest])
         << ")\n";
}

static void writeCategoryRanking(const Group& g, ostream& out) {
    struct CategoryData {
        string name;
        int amount;
    };
    int totals[5] = {0};

    for (const Expense& e : g.expenses) {
        totals[e.category] += e.amount;
    }

    vector<CategoryData> categories = {
        {"Food", totals[FOOD]},
        {"Transport", totals[TRANSPORT]},
        {"Accommodation", totals[ACCOMMODATION]},
        {"Entertainment", totals[ENTERTAINMENT]},
        {"Other", totals[OTHER]}
    };

    sort(categories.begin(), categories.end(),
         [](const CategoryData& a, const CategoryData& b) {
             if (a.amount == 0 && b.amount != 0) return false;
             if (a.amount != 0 && b.amount == 0) return true;
             return a.amount > b.amount;
         });

    out << "\n=== Category Ranking ===\n\n";
    int rank = 1;

    for (const auto& category : categories) {


        if (category.amount == 0)
            continue;

        out << rank++ << ". "
             << category.name
             << " - "
             << money(category.amount)
             << "\n";

    }

}

std::string categoryTotalsReport(const Group& g) { ostringstream out; writeCategoryTotals(g, out); return out.str(); }
void showCategoryTotals(const Group& g) { cout << categoryTotalsReport(g); }

std::string categoryAnalyticsReport(const Group& g) { ostringstream out; writeCategoryAnalytics(g, out); return out.str(); }
void showCategoryAnalytics(const Group& g) { cout << categoryAnalyticsReport(g); }

std::string categoryRankingReport(const Group& g) { ostringstream out; writeCategoryRanking(g, out); return out.str(); }
void showCategoryRanking(const Group& g) { cout << categoryRankingReport(g); }
