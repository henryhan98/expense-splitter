#include "storage.h"
#include <fstream>
#include "input.h"
using namespace std;

bool saveToFile(const Group& g, const string& filename) {
    ofstream out(filename);
    if (!out) return false;
    out << g.people.size() << "\n";
    for (const string& name : g.people) out << name << "\n";
    out << g.expenses.size() << "\n";
    for (const Expense& e : g.expenses) {
        out << e.day << "\n";
        out << e.description << "\n";
        out << e.payer << " " << e.amount << " "<< (int)e.category << " "<< e.sharedBy.size();
        for (int p : e.sharedBy) out << " " << p;
        out << "\n";
    }
    return true;
}

bool loadFromFile(Group& g, const string& filename) {
    ifstream in(filename);
    if (!in) return false;
    Group temp;
    int numPeople;
    if (!(in >> numPeople) || numPeople < 1) return false;
    in.ignore(1000, '\n');
    for (int i = 0; i < numPeople; i++) {
        string name;
        if (!getline(in, name)) return false;
        temp.people.push_back(trimLine(name));
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
        int categoryValue;
        
        if (!(in >> e.payer >> e.amount >> categoryValue >> count)) return false;

        
        e.category = static_cast<Category>(categoryValue);

        if (categoryValue < 0 || categoryValue > 4) return false;
        if (e.payer < 0 || e.payer >= numPeople || e.amount <= 0) return false;
        if (count < 1 || count > numPeople) return false;
        for (int k = 0; k < count; k++) {
            int p;
            if (!(in >> p) || p < 0 || p >= numPeople) return false;
            e.sharedBy.push_back(p);
        }
        temp.expenses.push_back(e);
    }
    g = temp;
    return true;
}
