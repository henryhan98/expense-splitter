#include "storage.h"
#include <fstream>
#include "input.h"
using namespace std;
#include <string>
#include <vector>
#include "percentage.h"

bool saveToFile(const Group& g, const string& filename) {
    ofstream out(filename);
    if (!out) return false;
    
    out << "v2\n" << g.people.size() << "\n";
    for (const string& name : g.people) out << name << "\n";
    bool fixed = hasFixedPercent(g);
    out << (fixed ? "1" : "0") << "\n";
    if (fixed) {
        for (size_t i = 0; i < g.fixedWeight.size(); i++) {
            out << (i ? " " : "") << g.fixedWeight[i];
        }
        out << "\n";
    }
    out << g.budget << "\n" << g.expenses.size() << "\n";

    for (const Expense& e : g.expenses) {
        out << e.day << "\n";
        out << e.description << "\n";
        out << e.payer << " " << e.amount << " " << e.sharedBy.size();
        for (int p : e.sharedBy) out << " " << p;
        out << " " << e.weights.size();
        for (int w : e.weights) out << " " << w;
        out << "\n";
    }
    return true;
}

bool loadFromFile(Group& g, const string& filename) {
    ifstream in(filename);
    if (!in) return false;
    Group temp;

    string tag;
    if (!(in >> tag)) return false;
    bool v2 = (tag == "v2");

    int numPeople = v2 ? 0 : atoi(tag.c_str());
    if (v2 && !(in >> numPeople)) return false;
    if (numPeople < 1) return false;
    in.ignore(1000, '\n');
    for (int i = 0; i < numPeople; i++) {
        string name;
        if (!getline(in, name)) return false;
        temp.people.push_back(trimLine(name));
    }

    if (v2) {
        int hasFixed = 0;
        if (!(in >> hasFixed)) return false;
        if (hasFixed) {
            temp.fixedWeight.assign(numPeople, 0);
            for (int i = 0; i < numPeople; i++) {
                if (!(in >> temp.fixedWeight[i])) return false;
            }
        }
        if (!(in >> temp.budget)) return false;
    }

    int numExpenses;
    if (!(in >> numExpenses) || numExpenses < 0) return false;
    for (int i = 0; i < numExpenses; i++) {
        Expense e;
        int count;
        if (!(in >> e.day)) return false;
        in.ignore(1000, '\n');
        if (!getline(in, e.description)) return false;
        e.description = trimLine(e.description);
        if (!(in >> e.payer >> e.amount >> count)) return false;
        if (e.payer < 0 || e.payer >= numPeople || e.amount <= 0) return false;
        if (count < 1 || count > numPeople) return false;
        for (int k = 0; k < count; k++) {
            int p;
            if (!(in >> p) || p < 0 || p >= numPeople) return false;
            e.sharedBy.push_back(p);
        }
        if (v2) {
            int weightCount = 0;
            if (!(in >> weightCount)) return false;
            for (int k = 0; k < weightCount; k++) {
                int w;
                if (!(in >> w) || w < 0) return false;
                e.weights.push_back(w);
            }
            if (weightCount != count) e.weights.clear();
        }

        temp.expenses.push_back(e);
    }
    g = temp;
    return true;
}