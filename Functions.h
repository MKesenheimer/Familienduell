#ifndef Functions_H
#define Functions_H

#include <iostream>
#include <vector>

void strToChar(const std::string tmp, char (&chars)[1024]);
bool is_number(const std::string& s);
std::vector<std::string> strToVec(std::string str);
std::string readNextString(std::string str, std::size_t *pos);

#endif