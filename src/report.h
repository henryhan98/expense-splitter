#ifndef REPORT_H
#define REPORT_H
#include "expense.h"

std::string dailyReport(const Group& g);
std::string balanceReport(const Group& g);
std::string settlementReport(const Group& g);

void showByDay(const Group& g);
void showBalances(const Group& g);
void settleUp(const Group& g);

std::string categoryTotalsReport(const Group& g);
std::string categoryAnalyticsReport(const Group& g);
std::string categoryRankingReport(const Group& g);

void showCategoryTotals(const Group& g);
void showCategoryAnalytics(const Group& g);
void showCategoryRanking(const Group& g);

#endif
