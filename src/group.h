#ifndef GROUP_H
#define GROUP_H
#include "expense.h"

void setupPeople(Group& g);
void printPeople(const Group& g);
void addExpense(Group& g);
void listExpenses(const Group& g);
void removeExpense(Group& g);

#endif