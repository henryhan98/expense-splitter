#ifndef INPUT_H
#define INPUT_H
#include <string>

std::string trimLine(std::string s);

std::string readText(const std::string& prompt);
int readInt(const std::string& prompt, int min, int max);
int readAmount(const std::string& prompt);
bool readYesNo(const std::string& prompt);

#endif