#include "percentage.h"
#include <iostream>
using namespace std;
#include <cmath>
#include <vector>
#include "input.h"
#include <string>

static double readNumber(const string& prompt, double lo, double hi) {
    while (true) {
        string s = readText(prompt);
        try {
            size_t used = 0;
            double v = stod(s, &used);
            if (used == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {
        }
        cout << "Please enter a number from " << lo << " to " << hi << ".\n";
    }
}

bool hasFixedPercent(const Group& g) {
    return !g.people.empty() && g.fixedWeight.size() == g.people.size();
}

vector<int> askPercentages(const Group& g) {
    vector<int> w;
    for (const string& name : g.people) {
        w.push_back((int)llround(readNumber("  " + name + " share (%): ", 0, 100) * 100));
    }
    return w;
}

void printPercentages(const Group& g) {
    if (!hasFixedPercent(g)) {
        cout << "  (none set - even split)\n";
        return;
    }
    for (size_t i = 0; i < g.people.size(); i++) {
        cout << "  " << g.people[i] << ": " << g.fixedWeight[i] / 100.0 << "%\n";
    }
}

void setFixedPercent(Group& g) {
    if (g.people.empty()) return;
    cout << "\nCurrent fixed percentages:\n";
    printPercentages(g);
    if (!readYesNo("Set new ones?")) return;
    g.fixedWeight = askPercentages(g);
    cout << "Saved:\n";
    printPercentages(g);
}

void applyFixedWeights(const Group& g, Expense& e) {
    e.sharedBy.clear();
    for (int i = 0; i < (int)g.people.size(); i++) e.sharedBy.push_back(i);
    e.weights = g.fixedWeight;
}

void applyCustomWeights(const Group& g, Expense& e) {
    e.sharedBy.clear();
    for (int i = 0; i < (int)g.people.size(); i++) e.sharedBy.push_back(i);
    e.weights = askPercentages(g);
}