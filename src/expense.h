#ifndef EXPENSE_H
#define EXPENSE_H
#include <string>
#include <vector>

enum Category {
    FOOD,
    TRANSPORT,
    ACCOMMODATION,
    ENTERTAINMENT,
    OTHER
};

struct Expense {
    int day;
    std::string description;
    int payer;
    int amount;
    Category category;
    std::vector<int> sharedBy;

    std::vector<int> weights;
};

struct Group {
    std::vector<std::string> people;
    std::vector<Expense> expenses;

    std::vector<int> fixedWeight;
    int budget = -1;
};

std::string money(int cents);

int shareOf(const Expense& e, int who);

#endif
