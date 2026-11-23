#ifndef REPORT_H
#define REPORT_H
#include "expense.h"

void showByDay(const Group& g);
void showBalances(const Group& g);
void settleUp(const Group& g);

void showCategoryTotals(const Group& g);
void showCategoryAnalytics(const Group& g);
void showCategoryRanking(const Group& g);

#endif
