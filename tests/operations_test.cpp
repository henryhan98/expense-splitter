#include "operations.h"
#include "report.h"
#include "storage.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    assert(argc == 2);
    Group g;
    assert(settlementReport(g) == "Everyone is square.\n");
    assert(balanceReport(g) == "No expenses yet.\n");
    g.people = {"Alice", "Bob", "Chris"};
    Expense e = {2, "Dinner", 0, 1001, FOOD, {0, 1}};
    assert(appendExpense(g, e));
    assert(categoryTotalsReport(g).find("Food: 10.01") != std::string::npos);
    assert(categoryAnalyticsReport(g).find("Largest Category: Food") != std::string::npos);
    assert(categoryRankingReport(g).find("1. Food - 10.01") != std::string::npos);
    assert(shareOf(e, 0) == 501 && shareOf(e, 1) == 500 && shareOf(e, 2) == 0);
    assert(settlementReport(g) == "  Bob pays Alice 5.00\n");
    assert(dailyReport(g).find("Total for the day: 10.01") != std::string::npos);
    assert(matchesExpense(g, e, ExpenseFilter::Description, "INN"));
    assert(matchesExpense(g, e, ExpenseFilter::Any, "alice"));
    assert(matchesExpense(g, e, ExpenseFilter::Payer, "", 0));
    assert(matchesExpense(g, e, ExpenseFilter::Day, "", 2));
    assert(!matchesExpense(g, e, ExpenseFilter::Day, "", 1));
    Expense invalid = e;
    invalid.sharedBy = {0, 0};
    assert(!appendExpense(g, invalid));
    invalid.sharedBy.clear();
    assert(!replaceExpense(g, 0, invalid));
    invalid = e; invalid.payer = 3;
    assert(!appendExpense(g, invalid));
    invalid = e; invalid.amount = 0;
    assert(!appendExpense(g, invalid));
    assert(g.expenses.size() == 1 && g.expenses[0].amount == 1001);
    e.amount = 2001; e.description = "Lunch"; e.payer = 1;
    assert(replaceExpense(g, 0, e));
    assert(g.expenses[0].sharedBy == std::vector<int>({0, 1}));
    assert(settlementReport(g) == "  Alice pays Bob 10.01\n");
    assert(!replaceExpense(g, -1, e) && !eraseExpense(g, 10));
    assert(saveToFile(g, argv[1]));
    Group loaded;
    assert(loadFromFile(loaded, argv[1]));
    assert(balanceReport(loaded) == balanceReport(g));
    assert(loaded.expenses[0].sharedBy == e.sharedBy);
    assert(loaded.expenses[0].category == FOOD);
    { std::ofstream out(argv[1]); out << "2\nAlice\nBob\n1\n1\nLegacy\n0 1001 2 0 1\n"; }
    assert(loadFromFile(loaded, argv[1]) && loaded.expenses[0].category == OTHER);
    { std::ofstream out(argv[1]); out << "2\nAlice\nBob\n1\n1\nCategory\n0 1001 0 2 0 1\n"; }
    assert(loadFromFile(loaded, argv[1]) && loaded.expenses[0].category == FOOD);
    { std::ofstream out(argv[1]); out << "3\nAlice\nBob\nChris\n1\n1\nAmbiguous\n0 1001 2 1 2\n"; }
    assert(!loadFromFile(loaded, argv[1]));
    assert(loaded.expenses[0].category == FOOD);
    std::remove(argv[1]);
    assert(eraseExpense(g, 0) && g.expenses.empty());
    std::cout << "Shared operations, reports, custom splits, and storage checks passed.\n";
}
