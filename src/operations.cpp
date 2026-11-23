#include "operations.h"
#include <cctype>
#include <set>

static bool valid(const Group& g, const Expense& e) {
    if (e.category < FOOD || e.category > OTHER) return false;
    if (e.day < 1 || e.day > 365 || e.amount <= 0 || e.payer < 0
        || e.payer >= static_cast<int>(g.people.size()) || e.sharedBy.empty()) return false;
    std::set<int> seen;
    for (int person : e.sharedBy)
        if (person < 0 || person >= static_cast<int>(g.people.size()) || !seen.insert(person).second) return false;
    return true;
}
bool appendExpense(Group& g, const Expense& e) {
    if (!valid(g, e)) return false;
    g.expenses.push_back(e);
    return true;
}
bool replaceExpense(Group& g, int index, const Expense& e) {
    if (index < 0 || index >= static_cast<int>(g.expenses.size()) || !valid(g, e)) return false;
    g.expenses[index] = e;
    return true;
}
bool eraseExpense(Group& g, int index) {
    if (index < 0 || index >= static_cast<int>(g.expenses.size())) return false;
    g.expenses.erase(g.expenses.begin() + index);
    return true;
}
static std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}
bool matchesExpense(const Group& g, const Expense& e, ExpenseFilter filter,
                    const std::string& query, int value) {
    if (filter == ExpenseFilter::Payer) return e.payer == value;
    if (filter == ExpenseFilter::Day) return e.day == value;
    std::string haystack = e.description;
    if (filter == ExpenseFilter::Any)
        haystack += " " + g.people[e.payer] + " " + std::to_string(e.day);
    return lower(haystack).find(lower(query)) != std::string::npos;
}
