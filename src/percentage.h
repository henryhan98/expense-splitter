#ifndef PERCENTAGE_H
#define PERCENTAGE_H
#include <vector>
#include "expense.h"

bool hasFixedPercent(const Group& g);
std::vector<int> askPercentages(const Group& g);
void printPercentages(const Group& g);
void setFixedPercent(Group& g);
void applyFixedWeights(const Group& g, Expense& e);
void applyCustomWeights(const Group& g, Expense& e);

#endif