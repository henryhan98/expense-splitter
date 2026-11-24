#include "expense.h"
#include <cstdlib>
#include <algorithm>
#include <vector>

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

static int weightedShare(const Expense& e, int pos) {
    int n = (int)e.sharedBy.size();
    bool useW = ((int)e.weights.size() == n);
    long long total = 0;
    for (int k = 0; k < n; k++) {
        total += useW ? e.weights[k] : 1;
    }
    if (total <= 0) return 0;
    std::vector<long long> rem(n, 0);
    std::vector<int> base(n, 0);
    long long assigned = 0;
    for (int k = 0; k < n; k++) {
        long long num = (long long)e.amount * (useW ? e.weights[k] : 1);
        base[k] = (int)(num / total);
        rem[k] = num % total;
        assigned += base[k];
    }
    std::vector<int> order(n);
    for (int k = 0; k < n; k++) order[k] = k;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return rem[a] != rem[b] ? rem[a] > rem[b] : a < b;
    });
    int result = base[pos];
    for (long long i = 0; i < e.amount - assigned; i++) {
        if (order[i] == pos) result += 1;
    }
    return result;
}

int shareOf(const Expense& e, int who) {
    for (int k = 0; k < (int)e.sharedBy.size(); k++) {
        if (e.sharedBy[k] == who) {
            return weightedShare(e, k);
        }
    }
    return 0;
}