#include <iostream>
#include "expense.h"
#include "group.h"
#include "input.h"
#include "report.h"
#include "storage.h"
using namespace std;

int main() {
    Group g;
    cout << "=== Expense Splitter ===\n";
    bool loaded = false;
    if (readYesNo("Load a saved file?")) {
        string filename = readText("File name: ");
        loaded = loadFromFile(g, filename);
        if (loaded) cout << "Loaded " << filename << ".\n";
        else cout << "Could not load that file. Starting a new group.\n";
    }
    if (!loaded) setupPeople(g);
    int choice;
    do {
        cout << "\n----- Menu -----\n"
             << "1. Add expense\n"
             << "2. List expenses\n"
             << "3. Remove expense\n"
             << "4. Summary by day\n"
             << "5. Balances\n"
             << "6. Settle up (who pays whom)\n"
             << "7. Save to file\n"
             << "8. Load from file\n"
             << "9. Category totals\n"
             << "10. Category analytics\n"
             << "0. Exit\n";
        choice = readInt("Choose: ", 0, 10);
        switch (choice) {
            case 1: addExpense(g); break;
            case 2: listExpenses(g); break;
            case 3: removeExpense(g); break;
            case 4: showByDay(g); break;
            case 5: showBalances(g); break;
            case 6: settleUp(g); break;
            case 7: {
                string filename = readText("File name: ");
                if (saveToFile(g, filename)) cout << "Saved.\n";
                else cout << "Could not save to " << filename << ".\n";
                break;
            }
            case 8: {
                string filename = readText("File name: ");
                if (loadFromFile(g, filename)) cout << "Loaded.\n";
                else cout << "Could not load " << filename << ".\n";
                break;
            }
            
            case 9: {
                showCategoryTotals(g);
                break;
            }

            case 10:
                showCategoryAnalytics(g);
                break;

            
        }
    } while (choice != 0);
    cout << "Bye!\n";
    return 0;
}
