#include "expense.h"
#include <cstdlib>

std::string money(int cents) {
    std::string sign = "";
    if (cents < 0) {
        sign = "-";
        cents = -cents;
    }
    std::string decimals = std::to_string(cents % 100);
    if (decimals.size() == 1) decimals = "0" + decimals;
    return sign + std::to_string(cents / 100) + "." + decimals;
}

int shareOf(const Expense& e, int who) {
    int n = e.sharedBy.size();
    int base = e.amount / n;
    int extra = e.amount % n;

    for (int k = 0; k < n; k++) {
        if (e.sharedBy[k] == who) {
            if (k < extra) return base + 1;
            return base;
        }
    }
    return 0;
}