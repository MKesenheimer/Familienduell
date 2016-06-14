#include "Functions.h"

void strToChar(const std::string tmp, char (&chars)[1024]) {
    strncpy(chars, tmp.c_str(), sizeof(chars));
    chars[sizeof(chars) - 1] = 0;
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}