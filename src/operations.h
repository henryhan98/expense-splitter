#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "expense.h"

// UI-independent operations. Invalid mutations leave the group unchanged.
bool appendExpense(Group& g, const Expense& expense);
bool replaceExpense(Group& g, int index, const Expense& expense);
bool eraseExpense(Group& g, int index);
enum class ExpenseFilter { Any, Description, Payer, Day };
bool matchesExpense(const Group& g, const Expense& expense, ExpenseFilter filter,
                    const std::string& query = "", int value = -1);
#endif
