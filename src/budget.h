#ifndef BUDGET_H
#define BUDGET_H
#include "expense.h"

int totalSpent(const Group& g);
bool hasBudget(const Group& g);

void showBudgetLine(const Group& g);
void adjustBudget(Group& g);
void reportAfterExpense(const Group& g);

#endif