#include "input.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

string trimLine(string s) {
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r')) s.pop_back();
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) start++;
    return s.substr(start);
}

string readText(const string& prompt) {
    cout << prompt;
    string line;
    if (!getline(cin, line)) {
        cout << "\nInput closed. Bye!\n";
        exit(0);
    }
    return trimLine(line);
}

int readInt(const string& prompt, int min, int max) {
    while (true) {
        string line = readText(prompt);
        stringstream ss(line);
        int value;
        string leftover;
        if (ss >> value && !(ss >> leftover) && value >= min && value <= max) {
            return value;
        }
        cout << "Please enter a number from " << min << " to " << max << ".\n";
    }
}

int readAmount(const string& prompt) {
    while (true) {
        string line = readText(prompt);
        stringstream ss(line);
        double value;
        string leftover;
        if (ss >> value && !(ss >> leftover) && value > 0 && value < 10000000) {
            return (int)round(value * 100);
        }
        cout << "Please enter a positive amount, e.g. 12.50\n";
    }
}

bool readYesNo(const string& prompt) {
    while (true) {
        string answer = readText(prompt + " (y/n): ");
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
        cout << "Please type y or n.\n";
    }
}