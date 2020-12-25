#ifndef UTIL_TIME_H
#define UTIL_TIME_H

#include <iostream>
#include <ctime>
#include <string>
using namespace std;

std::string & std_string_format(std::string & _str, const char * _Format, ...);
time_t StringToDatetime(string str);
string DatetimeToString(time_t time);


#endif// UTIL_TIME_H
