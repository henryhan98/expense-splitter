#ifndef EXPENSE_H
#define EXPENSE_H
#include <string>
#include <vector>

struct Expense {
    int day;
    std::string description;
    int payer;
    int amount;
    std::vector<int> sharedBy;
};

struct Group {
    std::vector<std::string> people;
    std::vector<Expense> expenses;
};

std::string money(int cents);

int shareOf(const Expense& e, int who);

#endif