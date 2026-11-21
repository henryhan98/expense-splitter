#ifndef STORAGE_H
#define STORAGE_H
#include <string>
#include "expense.h"

bool saveToFile(const Group& g, const std::string& filename);
bool loadFromFile(Group& g, const std::string& filename);

#endif